#define LOG_TAG "TextureAtlas"
#include "engine/graphics/TextureAtlas.h"

#include <algorithm>

#include "engine/asset/Image.h"
#include "engine/core/Log.h"

namespace engine {

TextureAtlas::TextureAtlas(int size) : size_(size) {
    texture_.create(size, size);
    // Clear to transparent so padding between sprites never shows garbage.
    // Upload in strips: a full 4096x4096 RGBA clear would need a 64 MB buffer.
    constexpr int kStripRows = 64;
    std::vector<uint8_t> zeros(static_cast<size_t>(size) * kStripRows * 4, 0);
    for (int y = 0; y < size; y += kStripRows) {
        texture_.upload(0, y, size, std::min(kStripRows, size - y), zeros.data());
    }
}

bool TextureAtlas::add(AssetLoader& loader, const std::string& name, const std::string& path) {
    const Image image = Image::load(loader, path);
    return image.isValid() && add(name, image);
}

bool TextureAtlas::add(const std::string& name, const Image& image) {
    const int w = image.width;
    const int h = image.height;

    if (w + kPadding > size_ || h + kPadding > size_) {
        LOGE("'%s' (%dx%d) is larger than the atlas", name.c_str(), w, h);
        return false;
    }

    // Start a new shelf when this image does not fit on the current one.
    if (shelfX_ + w + kPadding > size_) {
        shelfX_ = 0;
        shelfY_ += shelfHeight_ + kPadding;
        shelfHeight_ = 0;
    }
    if (shelfY_ + h + kPadding > size_) {
        LOGE("atlas full while adding '%s'", name.c_str());
        return false;
    }

    texture_.upload(shelfX_, shelfY_, w, h, image.pixels.data());

    // Inset by half a texel so linear filtering at the sprite's edge never
    // blends in the padding (visible as seams between adjacent tiles).
    constexpr float kInset = 0.5f;
    Sprite sprite;
    sprite.texture = &texture_;
    sprite.u0 = (shelfX_ + kInset) / size_;
    sprite.v0 = (shelfY_ + kInset) / size_;
    sprite.u1 = (shelfX_ + w - kInset) / size_;
    sprite.v1 = (shelfY_ + h - kInset) / size_;
    sprite.width = static_cast<float>(w);
    sprite.height = static_cast<float>(h);
    sprites_[name] = sprite;

    shelfX_ += w + kPadding;
    if (h > shelfHeight_) {
        shelfHeight_ = h;
    }
    return true;
}

Sprite TextureAtlas::get(const std::string& name) const {
    const auto it = sprites_.find(name);
    if (it == sprites_.end()) {
        LOGW("unknown sprite '%s'", name.c_str());
        return {};
    }
    return it->second;
}

}  // namespace engine
