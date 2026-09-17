#pragma once

#include <string>

#include "engine/graphics/Font.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/input/TouchEvent.h"
#include "engine/math/Color.h"
#include "engine/math/Rect.h"

namespace app::ui {

// Flat rectangular button with a centred label. Pressed state darkens it.
// Returns true from handleTouch() on the release that completes a tap.
class Button {
public:
    Button() = default;
    Button(engine::Rect rect, std::string label, engine::Color color)
        : rect_(rect), label_(std::move(label)), color_(color) {}

    bool handleTouch(const engine::TouchEvent& event) {
        using Phase = engine::TouchEvent::Phase;
        const bool inside = rect_.contains(event.position);
        switch (event.phase) {
        case Phase::Down:
            pressed_ = inside;
            return false;
        case Phase::Move:
            if (pressed_ && !inside) {
                pressed_ = false;
            }
            return false;
        case Phase::Up: {
            const bool tapped = pressed_ && inside;
            pressed_ = false;
            return tapped;
        }
        case Phase::Cancel:
            pressed_ = false;
            return false;
        }
        return false;
    }

    void draw(engine::SpriteBatch& batch, const engine::Font& font, float textSize) const {
        const float k = pressed_ ? 0.75f : 1.0f;
        // Drop shadow, then face.
        batch.drawRect({rect_.x, rect_.y + 6.0f, rect_.w, rect_.h}, engine::Color{0, 0, 0, 0.25f});
        batch.drawRect(rect_, {color_.r * k, color_.g * k, color_.b * k, color_.a});
        font.draw(batch, label_, {rect_.center().x, rect_.center().y - textSize * 0.5f}, textSize,
                  engine::Color::white(), engine::TextAlign::Center);
    }

    const engine::Rect& rect() const { return rect_; }
    void setRect(engine::Rect rect) { rect_ = rect; }

private:
    engine::Rect rect_;
    std::string label_;
    engine::Color color_;
    bool pressed_ = false;
};

}  // namespace app::ui
