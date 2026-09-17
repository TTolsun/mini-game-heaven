#pragma once

namespace engine {

struct Color {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;

    constexpr Color() = default;
    constexpr Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}

    static constexpr Color white() { return {1.0f, 1.0f, 1.0f}; }
    static constexpr Color black() { return {0.0f, 0.0f, 0.0f}; }

    static constexpr Color rgb8(int r8, int g8, int b8, float a_ = 1.0f) {
        return {r8 / 255.0f, g8 / 255.0f, b8 / 255.0f, a_};
    }

    constexpr Color withAlpha(float a_) const { return {r, g, b, a_}; }
};

}  // namespace engine
