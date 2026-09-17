#pragma once

#include <vector>

#include "engine/Scene.h"
#include "engine/graphics/Animation.h"
#include "engine/graphics/Sprite.h"
#include "engine/graphics/Texture.h"
#include "engine/math/Vec2.h"

namespace app {

// Temporary sandbox to exercise the renderer: forest background, ground tiles,
// the dino running to wherever you tap, and a jelly dropped at each tap.
class PlaygroundScene final : public engine::Scene {
public:
    void onEnter(engine::Engine& engine) override;
    void update(float dt) override;
    void render(engine::SpriteBatch& batch) override;
    void onTouch(const engine::TouchEvent& event) override;

private:
    struct Jelly {
        engine::Sprite sprite;
        engine::Vec2 position;
        float velocityY = 0.0f;
        float scale = 0.0f;  // pops in from 0 to 1
    };

    engine::Engine* engine_ = nullptr;

    engine::Texture background_;
    engine::Sprite backgroundSprite_;
    engine::Sprite groundTile_;
    std::vector<engine::Sprite> jellySprites_;

    engine::Animation idle_;
    engine::Animation run_;
    engine::Vec2 dinoPos_;
    float dinoTargetX_ = 0.0f;
    bool dinoFacingLeft_ = false;

    std::vector<Jelly> jellies_;
    float groundY_ = 0.0f;
};

}  // namespace app
