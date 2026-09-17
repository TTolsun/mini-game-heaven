#pragma once

#include "engine/math/Vec2.h"

namespace engine {

// Axis-aligned rectangle. Origin is the top-left corner; y grows downward,
// matching screen coordinates.
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    constexpr Rect() = default;
    constexpr Rect(float x_, float y_, float w_, float h_) : x(x_), y(y_), w(w_), h(h_) {}

    static constexpr Rect fromCenter(Vec2 center, Vec2 size) {
        return {center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y};
    }

    constexpr float left() const { return x; }
    constexpr float right() const { return x + w; }
    constexpr float top() const { return y; }
    constexpr float bottom() const { return y + h; }
    constexpr Vec2 center() const { return {x + w * 0.5f, y + h * 0.5f}; }
    constexpr Vec2 size() const { return {w, h}; }

    constexpr bool contains(Vec2 p) const {
        return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
    }

    constexpr bool overlaps(const Rect& o) const {
        return x < o.x + o.w && o.x < x + w && y < o.y + o.h && o.y < y + h;
    }
};

}  // namespace engine
