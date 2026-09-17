#pragma once

#include <cstdint>
#include <vector>

#include "engine/asset/AssetLoader.h"

namespace engine {

// Decoded RGBA8 pixels in CPU memory.
struct Image {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels;  // width * height * 4, row-major, top row first

    bool isValid() const { return width > 0 && height > 0; }

    // Decodes a PNG/JPG via stb_image. Returns an invalid Image on failure.
    static Image load(AssetLoader& loader, const std::string& path);
};

}  // namespace engine
