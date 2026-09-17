#define LOG_TAG "Font"
#include "engine/graphics/Font.h"

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
constexpr int kBitmapHeight = 512;

}  // namespace

bool Font::load(AssetLoader& assets, TextureAtlas& atlas, const std::string& name,
                const std::string& ttfPath) {
    const std::vector<uint8_t> ttf = assets.readFile(ttfPath);
    if (ttf.empty()) {
        return false;
    }

    std::vector<uint8_t> alpha(static_cast<size_t>(kBitmapWidth) * kBitmapHeight);
    std::vector<stbtt_bakedchar> baked(kCharCount);
    const int rowsUsed = stbtt_BakeFontBitmap(ttf.data(), 0, kBakePixelHeight, alpha.data(),
                                              kBitmapWidth, kBitmapHeight, kFirstChar, kCharCount,
                                              baked.data());
    if (rowsUsed <= 0) {
        LOGE("bake failed for %s (bitmap too small)", ttfPath.c_str());
        return false;
    }

    // Only upload the rows actually used so the atlas is not wasted on blank space.
    bitmapWidth_ = kBitmapWidth;
    bitmapHeight_ = rowsUsed;

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

    glyphs_.resize(kCharCount);
    for (int i = 0; i < kCharCount; ++i) {
        const stbtt_bakedchar& b = baked[i];
        glyphs_[i] = {static_cast<float>(b.x0), static_cast<float>(b.y0),
                      static_cast<float>(b.x1), static_cast<float>(b.y1),
                      b.xoff, b.yoff, b.xadvance};
    }

    LOGI("baked %s: %dx%d used", ttfPath.c_str(), bitmapWidth_, bitmapHeight_);
    return true;
}

const Font::Glyph* Font::glyph(char c) const {
    const int index = static_cast<unsigned char>(c) - kFirstChar;
    if (index < 0 || index >= kCharCount) {
        return nullptr;
    }
    return &glyphs_[index];
}

float Font::measure(std::string_view text, float size) const {
    const float scale = size / kBakePixelHeight;
    float width = 0.0f;
    for (char c : text) {
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

    for (char c : text) {
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
