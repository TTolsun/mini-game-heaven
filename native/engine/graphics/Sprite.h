#pragma once

#include "engine/math/Vec2.h"

namespace engine {

class Texture;

// A rectangular region of a texture, in normalized UV coordinates, plus the
// region's size in pixels so callers can draw it at native scale.
struct Sprite {
    const Texture* texture = nullptr;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
    float width = 0.0f;   // pixels
    float height = 0.0f;  // pixels

    bool isValid() const { return texture != nullptr; }
    Vec2 size() const { return {width, height}; }
};

}  // namespace engine
