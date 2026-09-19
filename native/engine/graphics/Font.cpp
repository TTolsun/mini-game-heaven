#define LOG_TAG "Font"
#include "engine/graphics/Font.h"
#include <algorithm>

#include "engine/asset/AssetLoader.h"
#include "engine/asset/Image.h"
#include "engine/core/Log.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/graphics/TextureAtlas.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "third_party/stb/stb_truetype.h"

namespace engine {

namespace {

constexpr int kBitmapWidth = 1024;
constexpr int kBitmapHeight = 2048;

int nextCodepoint(std::string_view text, size_t& offset) {
    const unsigned char c = text[offset++];
    if (c < 128) return c;
    int remaining = c >= 240 ? 3 : c >= 224 ? 2 : c >= 192 ? 1 : 0;
    if (!remaining || offset + remaining > text.size()) return '?';
    int value = c & ((1 << (6 - remaining)) - 1);
    while (remaining--) {
        const unsigned char continuation = text[offset];
        if ((continuation & 0xc0) != 0x80) return '?';
        ++offset; value = (value << 6) | (continuation & 63);
    }
    return value;
}
}  // namespace

bool Font::load(AssetLoader& assets, TextureAtlas& atlas, const std::string& name,
                const std::string& ttfPath, std::string_view extraGlyphs) {
    const std::vector<uint8_t> ttf = assets.readFile(ttfPath);
    if (ttf.empty()) {
        return false;
    }

    codepoints_.clear();
    for (int c = 32; c < 127; ++c) codepoints_.push_back(c);
    for (size_t offset = 0; offset < extraGlyphs.size();) codepoints_.push_back(nextCodepoint(extraGlyphs, offset));
    std::sort(codepoints_.begin(), codepoints_.end());
    codepoints_.erase(std::unique(codepoints_.begin(), codepoints_.end()), codepoints_.end());
    std::vector<uint8_t> alpha(static_cast<size_t>(kBitmapWidth) * kBitmapHeight);
    std::vector<stbtt_packedchar> baked(codepoints_.size());
    stbtt_pack_context context{};
    if (!stbtt_PackBegin(&context, alpha.data(), kBitmapWidth, kBitmapHeight, 0, 1, nullptr)) return false;
    stbtt_pack_range range{};
    range.font_size = kBakePixelHeight;
    range.array_of_unicode_codepoints = codepoints_.data();
    range.num_chars = static_cast<int>(codepoints_.size());
    range.chardata_for_range = baked.data();
    const bool packed = stbtt_PackFontRanges(&context, ttf.data(), 0, &range, 1) != 0;
    stbtt_PackEnd(&context);
    if (!packed) { LOGE("font atlas too small: %s", ttfPath.c_str()); return false; }
    bitmapWidth_ = kBitmapWidth;
    bitmapHeight_ = 1;
    for (const auto& glyph : baked) bitmapHeight_ = std::max(bitmapHeight_, static_cast<int>(glyph.y1) + 1);
    Image image;
    image.width = bitmapWidth_;
    image.height = bitmapHeight_;
    image.pixels.resize(static_cast<size_t>(bitmapWidth_) * bitmapHeight_ * 4);
    for (size_t i = 0; i < static_cast<size_t>(bitmapWidth_) * bitmapHeight_; ++i) {
        image.pixels[i * 4 + 0] = 255;
        image.pixels[i * 4 + 1] = 255;
        image.pixels[i * 4 + 2] = 255;
        image.pixels[i * 4 + 3] = alpha[i];
    }
    if (!atlas.add(name, image)) {
        return false;
    }
    sprite_ = atlas.get(name);

    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, ttf.data(), 0)) {
        LOGE("stbtt_InitFont failed for %s", ttfPath.c_str());
        return false;
    }
    int ascent = 0;
    int descent = 0;
    int lineGap = 0;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
    ascent_ = ascent * stbtt_ScaleForPixelHeight(&info, kBakePixelHeight);

    glyphs_.resize(codepoints_.size());
    for (size_t i = 0; i < codepoints_.size(); ++i) {
        const stbtt_packedchar& b = baked[i];
        glyphs_[i] = {static_cast<float>(b.x0), static_cast<float>(b.y0),
                      static_cast<float>(b.x1), static_cast<float>(b.y1),
                      b.xoff, b.yoff, b.xadvance};
    }

    LOGI("baked %s: %dx%d used", ttfPath.c_str(), bitmapWidth_, bitmapHeight_);
    return true;
}

const Font::Glyph* Font::glyph(int codepoint) const {
    auto found = std::lower_bound(codepoints_.begin(), codepoints_.end(), codepoint);
    if (found == codepoints_.end() || *found != codepoint) {
        found = std::lower_bound(codepoints_.begin(), codepoints_.end(), static_cast<int>('?'));
    }
    return found == codepoints_.end() ? nullptr : &glyphs_[found - codepoints_.begin()];
}
float Font::measure(std::string_view text, float size) const {
    if (!isLoaded()) return 0.0f;
    const float scale = size / kBakePixelHeight;
    float width = 0.0f;
    for (size_t offset = 0; offset < text.size();) {
        const int c = nextCodepoint(text, offset);
        if (const Glyph* g = glyph(c)) {
            width += g->xadvance * scale;
        }
    }
    return width;
}

void Font::draw(SpriteBatch& batch, std::string_view text, Vec2 position, float size, Color color,
                TextAlign align) const {
    if (!isLoaded()) {
        return;
    }
    const float scale = size / kBakePixelHeight;

    float penX = position.x;
    if (align != TextAlign::Left) {
        const float width = measure(text, size);
        penX -= align == TextAlign::Center ? width * 0.5f : width;
    }
    const float baseline = position.y + ascent_ * scale;

    const float uSpan = sprite_.u1 - sprite_.u0;
    const float vSpan = sprite_.v1 - sprite_.v0;

    for (size_t offset = 0; offset < text.size();) {
        const int c = nextCodepoint(text, offset);
        const Glyph* g = glyph(c);
        if (g == nullptr) {
            continue;
        }
        const float w = (g->x1 - g->x0) * scale;
        const float h = (g->y1 - g->y0) * scale;
        if (w > 0.0f && h > 0.0f) {
            Sprite glyphSprite = sprite_;
            glyphSprite.u0 = sprite_.u0 + uSpan * (g->x0 / bitmapWidth_);
            glyphSprite.u1 = sprite_.u0 + uSpan * (g->x1 / bitmapWidth_);
            glyphSprite.v0 = sprite_.v0 + vSpan * (g->y0 / bitmapHeight_);
            glyphSprite.v1 = sprite_.v0 + vSpan * (g->y1 / bitmapHeight_);
            glyphSprite.width = w;
            glyphSprite.height = h;

            const Vec2 topLeft{penX + g->xoff * scale, baseline + g->yoff * scale};
            batch.draw(glyphSprite, topLeft + Vec2{w * 0.5f, h * 0.5f}, Vec2{w, h}, color);
        }
        penX += g->xadvance * scale;
    }
}

}  // namespace engine
