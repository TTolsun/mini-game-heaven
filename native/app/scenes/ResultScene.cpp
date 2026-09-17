#include "app/scenes/ResultScene.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "app/GameAssets.h"
#include "engine/Engine.h"
#include "engine/graphics/SpriteBatch.h"

namespace app {

namespace {

constexpr float kPanelWidth = 580.0f;
constexpr float kPanelHeight = 560.0f;

}  // namespace

ResultScene::ResultScene(const GameAssets& assets, Outcome outcome, std::function<void()> onRetry,
                         std::function<void()> onHome)
    : assets_(assets),
      outcome_(outcome),
      onRetry_(std::move(onRetry)),
      onHome_(std::move(onHome)) {}

void ResultScene::onEnter(engine::Engine& engine) {
    engine_ = &engine;
    background_ = assets_.forestBackground();

    const engine::Vec2 world = engine.worldSize();
    const float panelX = (world.x - kPanelWidth) * 0.5f;
    const float panelY = (world.y - kPanelHeight) * 0.5f;
    const float buttonW = 240.0f;
    const float buttonH = 96.0f;
    const float buttonY = panelY + kPanelHeight - buttonH - 40.0f;

    retry_ = ui::Button({panelX + 40.0f, buttonY, buttonW, buttonH}, "RETRY",
                        engine::Color::rgb8(255, 138, 101));
    home_ = ui::Button({panelX + kPanelWidth - 40.0f - buttonW, buttonY, buttonW, buttonH}, "HOME",
                       engine::Color::rgb8(100, 181, 246));
}

void ResultScene::update(float dt) {
    reveal_ = std::min(1.0f, reveal_ + dt * 4.0f);
    if (shownScore_ < outcome_.score) {
        const int step = std::max(1, outcome_.score / 40);
        shownScore_ = std::min(outcome_.score, shownScore_ + step);
    }
}

void ResultScene::render(engine::SpriteBatch& batch) {
    const engine::Vec2 world = engine_->worldSize();
    const engine::Font& font = assets_.font();

    const float bgScale = std::max(world.x / background_.width, world.y / background_.height);
    batch.draw(background_, world * 0.5f, bgScale);
    batch.drawRect({0.0f, 0.0f, world.x, world.y}, engine::Color{0, 0, 0, 0.45f * reveal_});

    // Overshooting pop-in for the panel.
    const float pop = 1.0f + 0.15f * std::sin(reveal_ * 3.14159f);
    const engine::Vec2 panelSize{kPanelWidth * pop * reveal_, kPanelHeight * pop * reveal_};
    const engine::Rect panel = engine::Rect::fromCenter(world * 0.5f, panelSize);
    batch.drawRect({panel.x, panel.y + 10.0f, panel.w, panel.h}, engine::Color{0, 0, 0, 0.25f});
    batch.drawRect(panel, engine::Color::rgb8(255, 253, 231));

    if (reveal_ < 1.0f) {
        return;
    }

    const float cx = world.x * 0.5f;
    const engine::Color ink = engine::Color::rgb8(66, 66, 66);
    font.draw(batch, gameInfo(outcome_.game).title, {cx, panel.y + 36.0f}, 56.0f,
              gameInfo(outcome_.game).accent, engine::TextAlign::Center);

    font.draw(batch, "SCORE", {cx, panel.y + 120.0f}, 36.0f, ink.withAlpha(0.7f),
              engine::TextAlign::Center);
    char text[32];
    std::snprintf(text, sizeof(text), "%d", shownScore_);
    font.draw(batch, text, {cx, panel.y + 160.0f}, 120.0f, ink, engine::TextAlign::Center);

    std::snprintf(text, sizeof(text), "BEST %d", outcome_.best);
    font.draw(batch, text, {cx, panel.y + 300.0f}, 44.0f, ink.withAlpha(0.8f),
              engine::TextAlign::Center);

    if (outcome_.newBest && shownScore_ == outcome_.score) {
        const float wobble = std::sin(engine_->elapsed() * 8.0f) * 0.08f;
        font.draw(batch, "NEW BEST!", {cx, panel.y + 356.0f}, 52.0f + wobble * 52.0f,
                  engine::Color::rgb8(255, 112, 67), engine::TextAlign::Center);
    }

    retry_.draw(batch, font, 44.0f);
    home_.draw(batch, font, 44.0f);
}

void ResultScene::onTouch(const engine::TouchEvent& event) {
    if (reveal_ < 1.0f) {
        return;
    }
    if (retry_.handleTouch(event) && onRetry_) {
        onRetry_();
        return;
    }
    if (home_.handleTouch(event) && onHome_) {
        onHome_();
    }
}

}  // namespace app
