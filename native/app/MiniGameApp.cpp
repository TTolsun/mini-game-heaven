#define LOG_TAG "MiniGameApp"
#include "app/MiniGameApp.h"

#include "app/IMiniGame.h"
#include "app/scenes/MenuScene.h"
#include "app/scenes/ResultScene.h"
#include "engine/Engine.h"
#include "engine/core/Log.h"

namespace app {

void MiniGameApp::onEnter(engine::Engine& engine) {
    engine_ = &engine;
    if (!assets_.load(engine)) {
        LOGE("asset load failed; menu will be incomplete");
    }
    showMenu();
}

void MiniGameApp::switchTo(std::unique_ptr<engine::Scene> next) {
    pending_ = std::move(next);
}

void MiniGameApp::showMenu() {
    activeGame_ = nullptr;
    switchTo(std::make_unique<MenuScene>(assets_, scores_, [this](GameId id) { startGame(id); }));
}

void MiniGameApp::startGame(GameId id) {
    std::unique_ptr<IMiniGame> game = createGame(id, assets_);
    if (!game) {
        LOGW("game %d not implemented", static_cast<int>(id));
        return;
    }
    activeGameId_ = id;
    activeGame_ = game.get();
    switchTo(std::move(game));
    LOGI("start %s", gameInfo(id).title);
}

void MiniGameApp::showResult(GameId id, int score) {
    activeGame_ = nullptr;
    const bool newBest = scores_.submit(id, score);
    ResultScene::Outcome outcome{id, score, scores_.best(id), newBest};
    switchTo(std::make_unique<ResultScene>(
        assets_, outcome, [this, id] { startGame(id); }, [this] { showMenu(); }));
    LOGI("%s finished: score %d%s", gameInfo(id).title, score, newBest ? " (new best)" : "");
}

void MiniGameApp::update(float dt) {
    if (pending_) {
        if (current_) {
            current_->onExit();
        }
        current_ = std::move(pending_);
        current_->onEnter(*engine_);
    }
    if (!current_) {
        return;
    }
    current_->update(dt);

    if (activeGame_ != nullptr && activeGame_->isFinished()) {
        showResult(activeGameId_, activeGame_->score());
    }
}

void MiniGameApp::render(engine::SpriteBatch& batch) {
    if (current_) {
        current_->render(batch);
    }
}

void MiniGameApp::onTouch(const engine::TouchEvent& event) {
    if (current_ && !pending_) {
        current_->onTouch(event);
    }
}

}  // namespace app
