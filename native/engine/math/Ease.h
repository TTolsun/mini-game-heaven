#pragma once

#include <algorithm>
#include <cmath>

// Easing curves on t in [0, 1]. Use outBack/outElastic for "pop", outCubic for "settle".
namespace engine::ease {

inline float clamp01(float t) { return std::clamp(t, 0.0f, 1.0f); }

inline float linear(float t) { return clamp01(t); }

inline float outCubic(float t) {
    t = 1.0f - clamp01(t);
    return 1.0f - t * t * t;
}

inline float inCubic(float t) {
    t = clamp01(t);
    return t * t * t;
}

inline float outQuad(float t) {
    t = clamp01(t);
    return 1.0f - (1.0f - t) * (1.0f - t);
}

// Overshoots past 1 and settles back.
inline float outBack(float t, float overshoot = 1.70158f) {
    t = clamp01(t) - 1.0f;
    return 1.0f + t * t * ((overshoot + 1.0f) * t + overshoot);
}

inline float outElastic(float t) {
    t = clamp01(t);
    if (t == 0.0f || t == 1.0f) {
        return t;
    }
    constexpr float kPeriod = 0.3f;
    return std::pow(2.0f, -10.0f * t) * std::sin((t - kPeriod / 4.0f) * (2.0f * 3.14159265f) / kPeriod) + 1.0f;
}

// 0 -> 1 -> 0 hump, handy for one-shot flashes.
inline float pulse(float t) {
    t = clamp01(t);
    return std::sin(t * 3.14159265f);
}

}  // namespace engine::ease
