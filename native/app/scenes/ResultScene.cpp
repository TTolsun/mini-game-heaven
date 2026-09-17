#include "app/scenes/ResultScene.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "app/GameAssets.h"
#include "app/ui/Text.h"
#include "engine/Engine.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/math/Ease.h"

namespace app {

namespace {

constexpr float kPanelWidth = 600.0f;
constexpr float kButton = 128.0f;

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
    const engine::Sprite panelSprite = assets_.panel();
    const float panelH = kPanelWidth * panelSprite.height / panelSprite.width;
    panel_ = engine::Rect::fromCenter({world.x * 0.5f, world.y * 0.5f}, {kPanelWidth, panelH});

    const float buttonY = panel_.bottom() - kButton - 48.0f;
    retry_ = ui::Button({panel_.center().x - kButton - 24.0f, buttonY, kButton, kButton}, assets_.iconRetry());
    home_ = ui::Button({panel_.center().x + 24.0f, buttonY, kButton, kButton}, assets_.iconHome());

    engine.mixer().play(outcome_.newBest ? assets_.sfx().newBest : assets_.sfx().fanfare);
}

void ResultScene::update(float dt) {
    reveal_ = std::min(1.0f, reveal_ + dt / 0.35f);
    retry_.update(dt);
    home_.update(dt);
    confetti_.update(dt);

    if (reveal_ >= 1.0f && shownScore_ < outcome_.score) {
        const int step = std::max(1, outcome_.score / 30);
        shownScore_ = std::min(outcome_.score, shownScore_ + step);
        if (shownScore_ == outcome_.score && outcome_.newBest && !celebrated_) {
            celebrated_ = true;
            engine_->haptics().medium();
            engine_->addTrauma(0.35f);
            engine::Particles::Burst burst;
            burst.count = 60;
            burst.speedMin = 400.0f;
            burst.speedMax = 1100.0f;
            burst.angleMin = 3.4f;
            burst.angleMax = 6.0f;
            burst.lifeMin = 0.8f;
            burst.lifeMax = 1.4f;
            burst.sizeMin = 10.0f;
            burst.sizeMax = 22.0f;
            burst.gravity = 900.0f;
            burst.color = engine::Color::rgb8(255, 241, 118);
            burst.colorAlt = engine::Color::rgb8(41, 182, 246);
            confetti_.emit({panel_.center().x, panel_.bottom()}, burst);
        }
    }
}

void ResultScene::render(engine::SpriteBatch& batch) {
    const engine::Vec2 world = engine_->worldSize();
    const engine::Font& font = assets_.font();

    const float bgScale = std::max(world.x / background_.width, world.y / background_.height);
    batch.draw(background_, world * 0.5f, bgScale);
    batch.drawRect({0.0f, 0.0f, world.x, world.y}, engine::Color{0.1f, 0.15f, 0.2f, 0.55f * reveal_});

    // Panel pops in with overshoot.
    const float pop = engine::ease::outBack(reveal_, 1.4f);
    batch.draw(assets_.panel(), panel_.center(), panel_.size() * pop);

    // Ribbon straddles the top edge and carries the game name.
    const engine::Sprite ribbon = assets_.ribbon();
    const float ribbonW = kPanelWidth * 0.92f * pop;
    const engine::Vec2 ribbonPos{panel_.center().x, panel_.y + 10.0f};
    batch.draw(ribbon, ribbonPos, {ribbonW, ribbonW * ribbon.height / ribbon.width});

    if (reveal_ < 1.0f) {
        return;
    }

    const GameInfo& info = gameInfo(outcome_.game);
    const float cx = panel_.center().x;
    const engine::Color ink = engine::Color::rgb8(93, 64, 55);

    ui::shadowText(batch, font, info.title, {cx, ribbonPos.y - ui::kTitle * 0.5f - 6.0f}, ui::kTitle,
                   engine::Color::white(), engine::TextAlign::Center);

    font.draw(batch, "SCORE", {cx, panel_.y + 110.0f}, ui::kBody, ink.withAlpha(0.7f), engine::TextAlign::Center);
    char text[32];
    std::snprintf(text, sizeof(text), "%d", shownScore_);
    font.draw(batch, text, {cx, panel_.y + 150.0f}, 130.0f, ink, engine::TextAlign::Center);

    // Best with the star badge.
    const float starSize = 56.0f;
    std::snprintf(text, sizeof(text), "BEST %d", outcome_.best);
    const float bestW = font.measure(text, ui::kBody) + starSize + 12.0f;
    batch.draw(assets_.star(), {cx - bestW * 0.5f + starSize * 0.5f, panel_.y + 318.0f}, {starSize, starSize});
    font.draw(batch, text, {cx - bestW * 0.5f + starSize + 12.0f, panel_.y + 318.0f - ui::kBody * 0.55f},
              ui::kBody, ink.withAlpha(0.85f));

    if (outcome_.newBest && shownScore_ == outcome_.score) {
        const float wobble = 1.0f + std::sin(engine_->elapsed() * 8.0f) * 0.06f;
        ui::shadowText(batch, font, "NEW BEST!", {cx, panel_.y + 372.0f}, ui::kTitle * wobble,
                       engine::Color::rgb8(255, 112, 67), engine::TextAlign::Center);
    }

    retry_.draw(batch, font);
    home_.draw(batch, font);
    confetti_.render(batch);
}

void ResultScene::onTouch(const engine::TouchEvent& event) {
    if (reveal_ < 1.0f) {
        return;
    }
    if (retry_.handleTouch(event) && onRetry_) {
        engine_->haptics().light();
        engine_->mixer().play(assets_.sfx().click);
        onRetry_();
        return;
    }
    if (home_.handleTouch(event) && onHome_) {
        engine_->haptics().light();
        engine_->mixer().play(assets_.sfx().click);
        onHome_();
    }
}

bool ResultScene::onBack() {
    if (onHome_) {
        onHome_();
    }
    return true;
}

}  // namespace app
