#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "engine/graphics/Sprite.h"
#include "engine/graphics/Texture.h"

namespace engine {

class AssetLoader;
struct Image;

// Packs many small images into one GPU texture at load time so the sprite
// batch can draw everything with a single texture bind. Uses shelf packing:
// images fill a row left to right; when a row is full, a new row starts below.
class TextureAtlas {
public:
    explicit TextureAtlas(int size = 2048);

    // Loads a PNG from assets and packs it under `name`.
    // Returns false if the file is missing or the atlas is full.
    bool add(AssetLoader& loader, const std::string& name, const std::string& path);
    bool add(const std::string& name, const Image& image);

    // Looks up a packed sprite; returns an invalid Sprite if unknown.
    Sprite get(const std::string& name) const;
    bool has(const std::string& name) const { return sprites_.count(name) != 0; }

    const Texture& texture() const { return texture_; }

private:
    static constexpr int kPadding = 2;  // gap so linear filtering never samples a neighbour

    Texture texture_;
    int size_;
    int shelfX_ = 0;
    int shelfY_ = 0;
    int shelfHeight_ = 0;
    std::unordered_map<std::string, Sprite> sprites_;
};

}  // namespace engine
