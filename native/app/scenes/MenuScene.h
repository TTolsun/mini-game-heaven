#pragma once

#include <functional>
#include <vector>

#include "app/GameRegistry.h"
#include "app/ui/Button.h"
#include "engine/Scene.h"
#include "engine/graphics/Animation.h"
#include "engine/graphics/Sprite.h"

namespace app {

class GameAssets;
class HighScores;

// Title screen: one card per registered game, plus the dino idling on the ground.
class MenuScene final : public engine::Scene {
public:
    using OnSelect = std::function<void(GameId)>;

    MenuScene(const GameAssets& assets, const HighScores& scores, OnSelect onSelect);

    void onEnter(engine::Engine& engine) override;
    void update(float dt) override;
    void render(engine::SpriteBatch& batch) override;
    void onTouch(const engine::TouchEvent& event) override;

private:
    const GameAssets& assets_;
    const HighScores& scores_;
    OnSelect onSelect_;
    engine::Engine* engine_ = nullptr;

    engine::Sprite background_;
    engine::Sprite groundTile_;
    engine::Animation dinoIdle_;
    std::vector<ui::Button> cards_;
    float groundY_ = 0.0f;
    float bob_ = 0.0f;
};

}  // namespace app
