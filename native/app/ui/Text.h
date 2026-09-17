#pragma once

#include <string_view>

#include "engine/graphics/Font.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/math/Color.h"
#include "engine/math/Vec2.h"

namespace app::ui {

// Type scale used across all screens (world units). Keeping these few sizes
// consistent is what makes the screens feel like one app.
constexpr float kDisplay = 104.0f;  // title
constexpr float kHeadline = 72.0f;  // in-game score
constexpr float kTitle = 52.0f;     // card / panel titles
constexpr float kBody = 34.0f;      // hints, labels
constexpr float kLabel = 30.0f;     // small captions

// Text with a soft drop shadow so it stays readable on the bright forest art.
inline void shadowText(engine::SpriteBatch& batch, const engine::Font& font, std::string_view text,
                       engine::Vec2 position, float size, engine::Color color,
                       engine::TextAlign align = engine::TextAlign::Left, float shadowOffset = 0.05f) {
    const float o = size * shadowOffset;
    font.draw(batch, text, position + engine::Vec2{o, o}, size, engine::Color{0, 0, 0, 0.3f * color.a}, align);
    font.draw(batch, text, position, size, color, align);
}

}  // namespace app::ui
