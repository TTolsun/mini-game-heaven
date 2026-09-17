#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "app/ui/Text.h"
#include "engine/graphics/Font.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/math/Color.h"
#include "engine/math/Ease.h"
#include "engine/math/Vec2.h"

namespace app::ui {

// Floating score texts ("+5", "CLOSE!"): pop in with overshoot, rise, fade.
class Popups {
public:
    void add(std::string text, engine::Vec2 position, engine::Color color, float size = 56.0f) {
        items_.push_back({std::move(text), position, color, size, 0.0f});
    }

    void update(float dt) {
        for (Item& p : items_) {
            p.age += dt;
        }
        items_.erase(std::remove_if(items_.begin(), items_.end(), [](const Item& p) { return p.age > kLife; }),
                     items_.end());
    }

    void render(engine::SpriteBatch& batch, const engine::Font& font) const {
        for (const Item& p : items_) {
            const float t = p.age / kLife;
            const float rise = engine::ease::outCubic(t) * 150.0f;
            const float scale = 0.6f + 0.4f * engine::ease::outBack(std::min(1.0f, t * 4.0f), 2.5f);
            shadowText(batch, font, p.text, {p.position.x, p.position.y - rise}, p.size * scale,
                       p.color.withAlpha(1.0f - t * t), engine::TextAlign::Center);
        }
    }

    void clear() { items_.clear(); }

private:
    static constexpr float kLife = 0.8f;

    struct Item {
        std::string text;
        engine::Vec2 position;
        engine::Color color;
        float size;
        float age;
    };

    std::vector<Item> items_;
};

}  // namespace app::ui
