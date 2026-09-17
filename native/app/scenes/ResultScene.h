#pragma once

#include <functional>

#include "app/GameRegistry.h"
#include "app/ui/Button.h"
#include "engine/Scene.h"
#include "engine/graphics/Sprite.h"

namespace app {

class GameAssets;

// Shown after a mini game ends: score, best score, retry / home.
class ResultScene final : public engine::Scene {
public:
    struct Outcome {
        GameId game;
        int score;
        int best;
        bool newBest;
    };

    ResultScene(const GameAssets& assets, Outcome outcome, std::function<void()> onRetry,
                std::function<void()> onHome);

    void onEnter(engine::Engine& engine) override;
    void update(float dt) override;
    void render(engine::SpriteBatch& batch) override;
    void onTouch(const engine::TouchEvent& event) override;

private:
    const GameAssets& assets_;
    Outcome outcome_;
    std::function<void()> onRetry_;
    std::function<void()> onHome_;
    engine::Engine* engine_ = nullptr;

    engine::Sprite background_;
    ui::Button retry_;
    ui::Button home_;
    float reveal_ = 0.0f;  // 0..1 panel pop-in
    int shownScore_ = 0;   // counts up to the real score
};

}  // namespace app
