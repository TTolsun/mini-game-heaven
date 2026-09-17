#include "app/scenes/MenuScene.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "app/GameAssets.h"
#include "app/HighScores.h"
#include "app/ui/Text.h"
#include "engine/Engine.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/math/Ease.h"

namespace app {

namespace {

constexpr float kTile = 96.0f;
constexpr float kCardWidth = 600.0f;
constexpr float kCardAspect = 189.0f / 425.0f;  // GUI wide button
constexpr float kCardGap = 18.0f;

}  // namespace

MenuScene::MenuScene(const GameAssets& assets, const HighScores& scores, OnSelect onSelect)
    : assets_(assets), scores_(scores), onSelect_(std::move(onSelect)) {}

void MenuScene::onEnter(engine::Engine& engine) {
    engine_ = &engine;
    background_ = assets_.forestBackground();
    groundTile_ = assets_.groundTile();
    dirtTile_ = assets_.dirtTile();
    dinoIdle_ = assets_.dinoIdle();
    groundY_ = engine.worldHeight() - 2.0f * kTile;
    titleY_ = engine.safeTop() + 70.0f;

    // Cards fill the band between the title and the ground; shrink on short screens.
    const float top = titleY_ + 250.0f;
    const float bottom = groundY_ - 150.0f;
    const int count = static_cast<int>(allGames().size());
    float cardH = std::min(kCardWidth * kCardAspect, (bottom - top - kCardGap * (count - 1)) / count);
    const float cardW = cardH / kCardAspect;
    const float left = (engine.worldWidth() - cardW) * 0.5f;

    cards_.clear();
    float y = top;
    for (const GameInfo& info : allGames()) {
        cards_.emplace_back(engine::Rect{left, y, cardW, cardH}, assets_.wideButton(info.available ? info.buttonColor : 3));
        y += cardH + kCardGap;
    }
    enter_ = 0.0f;
}

engine::Sprite MenuScene::iconFor(GameId id) const {
    switch (id) {
    case GameId::Dodge:
        return assets_.stone();
    case GameId::Jump:
        return assets_.mushroomPink();
    case GameId::Tap:
        return assets_.crate();
    }
    return {};
}

void MenuScene::update(float dt) {
    dinoIdle_.update(dt);
    time_ += dt;
    enter_ = std::min(1.0f, enter_ + dt / 0.45f);
    for (ui::Button& card : cards_) {
        card.update(dt);
    }
}

void MenuScene::drawScenery(engine::SpriteBatch& batch) {
    const engine::Vec2 world = engine_->worldSize();
    // Trees behind, bushes in front, all standing on the grass line.
    const engine::Sprite tree = assets_.tree(0);
    batch.draw(tree, {100.0f, groundY_ - tree.height * 0.42f + 8.0f}, 0.85f);
    const engine::Sprite tree2 = assets_.tree(1);
    batch.draw(tree2, {world.x - 90.0f, groundY_ - tree2.height * 0.35f + 8.0f}, 0.7f);
    const engine::Sprite bush = assets_.bush(0);
    batch.draw(bush, {world.x - 200.0f, groundY_ - bush.height * 0.28f + 4.0f}, 0.55f);
    const engine::Sprite bush2 = assets_.bush(1);
    batch.draw(bush2, {230.0f, groundY_ - bush2.height * 0.3f + 4.0f}, 0.6f);
}

void MenuScene::render(engine::SpriteBatch& batch) {
    const engine::Vec2 world = engine_->worldSize();
    const engine::Font& font = assets_.font();

    const float bgScale = std::max(world.x / background_.width, world.y / background_.height);
    batch.draw(background_, world * 0.5f, bgScale);

    drawScenery(batch);

    const int columns = static_cast<int>(std::ceil(world.x / kTile));
    for (int col = 0; col < columns; ++col) {
        const float cx = col * kTile + kTile * 0.5f;
        batch.draw(groundTile_, {cx, groundY_ + kTile * 0.5f}, {kTile, kTile});
        for (float y = groundY_ + kTile; y < world.y; y += kTile) {
            batch.draw(dirtTile_, {cx, y + kTile * 0.5f}, {kTile, kTile});
        }
    }

    // Title with a gentle bob.
    const float bob = std::sin(time_ * 2.0f) * 5.0f;
    ui::shadowText(batch, font, "MINI GAME", {world.x * 0.5f, titleY_ + bob}, ui::kDisplay,
                   engine::Color::rgb8(255, 241, 118), engine::TextAlign::Center);
    ui::shadowText(batch, font, "HEAVEN", {world.x * 0.5f, titleY_ + 108.0f + bob}, ui::kDisplay,
                   engine::Color::white(), engine::TextAlign::Center);

    const std::vector<GameInfo>& games = allGames();
    for (size_t i = 0; i < cards_.size(); ++i) {
        const GameInfo& info = games[i];
        // Staggered slide-in from the right.
        const float t = std::clamp((enter_ - i * 0.12f) / 0.6f, 0.0f, 1.0f);
        const float slide = (1.0f - engine::ease::outCubic(t)) * world.x;
        engine::Rect r = cards_[i].rect();
        r.x += slide;
        ui::Button card = cards_[i];
        card.setRect(r);
        card.draw(batch, font);

        const float iconBox = r.h * 0.62f;
        const engine::Sprite icon = iconFor(info.id);
        const float iconScale = std::min(iconBox / icon.width, iconBox / icon.height);
        batch.draw(icon, {r.x + r.h * 0.5f, r.center().y}, iconScale);

        const float textX = r.x + r.h * 0.95f;
        ui::shadowText(batch, font, info.title, {textX, r.y + r.h * 0.17f}, ui::kTitle, engine::Color::white());
        ui::shadowText(batch, font, info.available ? info.hint : "COMING SOON", {textX, r.y + r.h * 0.55f},
                       ui::kLabel, engine::Color::white().withAlpha(0.9f));

        if (info.available) {
            // Best score badge on the title row: star, then the number, right-aligned.
            char best[32];
            std::snprintf(best, sizeof(best), "%d", scores_.best(info.id));
            const float starSize = r.h * 0.24f;
            const float numberW = font.measure(best, ui::kBody);
            const float right = r.right() - r.h * 0.18f;
            ui::shadowText(batch, font, best, {right, r.y + r.h * 0.22f}, ui::kBody, engine::Color::white(),
                           engine::TextAlign::Right);
            batch.draw(assets_.star(), {right - numberW - starSize * 0.6f, r.y + r.h * 0.32f},
                       {starSize, starSize});
        }
    }

    const engine::Sprite& frame = dinoIdle_.frame();
    const engine::Vec2 size = frame.size() * 0.6f;
    batch.draw(frame, {world.x * 0.5f, groundY_ - size.y * 0.5f + 6.0f}, size);
}

void MenuScene::onTouch(const engine::TouchEvent& event) {
    if (enter_ < 1.0f) {
        return;
    }
    const std::vector<GameInfo>& games = allGames();
    for (size_t i = 0; i < cards_.size(); ++i) {
        if (cards_[i].handleTouch(event) && games[i].available && onSelect_) {
            onSelect_(games[i].id);
            return;
        }
    }
}

}  // namespace app
