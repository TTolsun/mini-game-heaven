#define LOG_TAG "MiniGameApp"
#include "app/MiniGameApp.h"

#include "app/IMiniGame.h"
#include "app/scenes/MenuScene.h"
#include "app/scenes/ResultScene.h"
#include "engine/Engine.h"
#include "engine/core/Log.h"

namespace app {

namespace {

constexpr float kQuitSize = 84.0f;

}  // namespace

void MiniGameApp::onEnter(engine::Engine& engine) {
    engine_ = &engine;
    if (!assets_.load(engine)) {
        LOGE("asset load failed; menu will be incomplete");
    }
    scores_.load(engine.dataPath());

    // Quit button lives in the top-left corner, below the cutout / status bar.
    quit_ = ui::Button({20.0f, engine.safeTop() + 16.0f, kQuitSize, kQuitSize}, assets_.iconClose());
    showMenu();
}

void MiniGameApp::switchTo(std::unique_ptr<engine::Scene> next) {
    pending_ = std::move(next);
}

void MiniGameApp::showMenu() {
    activeGame_ = nullptr;
    inMenu_ = true;
    switchTo(std::make_unique<MenuScene>(assets_, scores_, [this](GameId id) { startGame(id); }));
}

void MiniGameApp::startGame(GameId id) {
    std::unique_ptr<IMiniGame> game = createGame(id, assets_);
    if (!game) {
        LOGW("game %d not implemented", static_cast<int>(id));
        return;
    }
    engine_->haptics().light();
    engine_->mixer().play(assets_.sfx().click);
    activeGameId_ = id;
    activeGame_ = game.get();
    inMenu_ = false;
    switchTo(std::move(game));
    LOGI("start %s", gameInfo(id).title);
}

void MiniGameApp::showResult(GameId id, int score) {
    activeGame_ = nullptr;
    inMenu_ = false;
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
    quit_.update(dt);

    if (activeGame_ != nullptr && activeGame_->isFinished()) {
        showResult(activeGameId_, activeGame_->score());
    }
}

void MiniGameApp::render(engine::SpriteBatch& batch) {
    if (current_) {
        current_->render(batch);
    }
    if (activeGame_ != nullptr) {
        quit_.draw(batch, assets_.font());
    }
}

void MiniGameApp::onTouch(const engine::TouchEvent& event) {
    if (!current_ || pending_) {
        return;
    }
    if (activeGame_ != nullptr && quit_.handleTouch(event)) {
        engine_->haptics().light();
        engine_->mixer().play(assets_.sfx().click);
        showMenu();
        return;
    }
    // A press that started on the quit button must not leak into the game.
    if (activeGame_ != nullptr && quit_.pressed()) {
        return;
    }
    current_->onTouch(event);
}

bool MiniGameApp::onBack() {
    if (inMenu_) {
        return false;  // let the activity finish
    }
    if (current_ && current_->onBack()) {
        return true;
    }
    showMenu();
    return true;
}

}  // namespace app
