#pragma once

#include <string>

#include "app/ui/Text.h"
#include "engine/graphics/Font.h"
#include "engine/graphics/Sprite.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/input/TouchEvent.h"
#include "engine/math/Color.h"
#include "engine/math/Ease.h"
#include "engine/math/Rect.h"

namespace app::ui {

// Sprite-skinned button (GUI pack face) with an optional centred label.
// Press: shrinks slightly and darkens. Release inside: springs back with
// overshoot and reports a tap. The hit rect is at least 48dp-equivalent
// (72 world units) tall regardless of the art.
class Button {
public:
    Button() = default;
    Button(engine::Rect rect, engine::Sprite face, std::string label = "")
        : rect_(rect), face_(face), label_(std::move(label)) {}

    // Returns true on the release that completes a tap.
    bool handleTouch(const engine::TouchEvent& event) {
        using Phase = engine::TouchEvent::Phase;
        const bool inside = hitRect().contains(event.position);
        switch (event.phase) {
        case Phase::Down:
            if (inside) {
                pressed_ = true;
                pointer_ = event.pointerId;
            }
            return false;
        case Phase::Move:
            if (pressed_ && event.pointerId == pointer_ && !inside) {
                pressed_ = false;
            }
            return false;
        case Phase::Up: {
            const bool tapped = pressed_ && event.pointerId == pointer_ && inside;
            if (pressed_ && event.pointerId == pointer_) {
                pressed_ = false;
                release_ = 0.0f;
            }
            return tapped;
        }
        case Phase::Cancel:
            pressed_ = false;
            return false;
        }
        return false;
    }

    void update(float dt) { release_ = std::min(1.0f, release_ + dt / 0.25f); }

    void draw(engine::SpriteBatch& batch, const engine::Font& font, float textSize = kBody,
              engine::Color labelColor = engine::Color::white()) const {
        // Pressed: 0.94; on release spring from 0.94 back to 1 with overshoot.
        const float scale = pressed_ ? 0.94f : 0.94f + 0.06f * engine::ease::outBack(release_, 2.5f);
        const float k = pressed_ ? 0.82f : 1.0f;
        batch.draw(face_, rect_.center(), rect_.size() * scale, engine::Color{k, k, k, 1.0f});
        if (!label_.empty()) {
            const engine::Vec2 c = rect_.center();
            // Labels sit a little above centre so they align with the button's glossy band.
            ui::shadowText(batch, font, label_, {c.x, c.y - textSize * 0.55f}, textSize * scale, labelColor,
                           engine::TextAlign::Center);
        }
    }

    const engine::Rect& rect() const { return rect_; }
    void setRect(engine::Rect rect) { rect_ = rect; }
    void setLabel(std::string label) { label_ = std::move(label); }
    bool pressed() const { return pressed_; }

private:
    static constexpr float kMinHit = 72.0f;

    engine::Rect hitRect() const {
        engine::Rect r = rect_;
        if (r.h < kMinHit) {
            r.y -= (kMinHit - r.h) * 0.5f;
            r.h = kMinHit;
        }
        if (r.w < kMinHit) {
            r.x -= (kMinHit - r.w) * 0.5f;
            r.w = kMinHit;
        }
        return r;
    }

    engine::Rect rect_;
    engine::Sprite face_;
    std::string label_;
    bool pressed_ = false;
    int pointer_ = -1;
    float release_ = 1.0f;
};

}  // namespace app::ui
