#pragma once

#include <functional>
#include <memory>

#include "app/GameAssets.h"
#include "app/GameRegistry.h"
#include "app/HighScores.h"
#include "app/ui/Button.h"
#include "engine/Scene.h"

namespace app {

class IMiniGame;

// Root scene: loads shared assets, then flips between menu, game and result.
// Scene switches requested from callbacks are applied at the start of the
// next update so a scene never destroys itself mid-call. While a game runs,
// a quit button is overlaid in the safe area and system back returns home.
class MiniGameApp final : public engine::Scene {
public:
    void onEnter(engine::Engine& engine) override;
    void update(float dt) override;
    void render(engine::SpriteBatch& batch) override;
    void onTouch(const engine::TouchEvent& event) override;
    bool onBack() override;

private:
    void showMenu();
    void startGame(GameId id);
    void showResult(GameId id, int score);
    void switchTo(std::unique_ptr<engine::Scene> next);

    engine::Engine* engine_ = nullptr;
    GameAssets assets_;
    HighScores scores_;

    std::unique_ptr<engine::Scene> current_;
    std::unique_ptr<engine::Scene> pending_;
    IMiniGame* activeGame_ = nullptr;  // non-owning view of current_ while playing
    GameId activeGameId_ = GameId::Dodge;
    bool inMenu_ = true;
    ui::Button quit_;
};

}  // namespace app
