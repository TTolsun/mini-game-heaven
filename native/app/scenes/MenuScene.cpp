#include "app/scenes/MenuScene.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "app/GameAssets.h"
#include "app/HighScores.h"
#include "engine/Engine.h"
#include "engine/graphics/SpriteBatch.h"

namespace app {

namespace {

constexpr float kTileSize = 96.0f;
constexpr float kCardWidth = 560.0f;
constexpr float kCardHeight = 150.0f;
constexpr float kCardGap = 26.0f;
constexpr float kCardsTop = 400.0f;

}  // namespace

MenuScene::MenuScene(const GameAssets& assets, const HighScores& scores, OnSelect onSelect)
    : assets_(assets), scores_(scores), onSelect_(std::move(onSelect)) {}

void MenuScene::onEnter(engine::Engine& engine) {
    engine_ = &engine;
    background_ = assets_.forestBackground();
    groundTile_ = assets_.groundTile();
    dinoIdle_ = assets_.dinoIdle();
    groundY_ = engine.worldHeight() - 2.0f * kTileSize;

    cards_.clear();
    const float left = (engine.worldWidth() - kCardWidth) * 0.5f;
    float y = kCardsTop;
    for (const GameInfo& info : allGames()) {
        const engine::Color color = info.available ? info.accent : engine::Color::rgb8(158, 158, 158);
        cards_.emplace_back(engine::Rect{left, y, kCardWidth, kCardHeight}, "", color);
        y += kCardHeight + kCardGap;
    }
}

void MenuScene::update(float dt) {
    dinoIdle_.update(dt);
    bob_ += dt;
}

void MenuScene::render(engine::SpriteBatch& batch) {
    const engine::Vec2 world = engine_->worldSize();
    const engine::Font& font = assets_.font();

    const float bgScale = std::max(world.x / background_.width, world.y / background_.height);
    batch.draw(background_, world * 0.5f, bgScale);

    const int columns = static_cast<int>(std::ceil(world.x / kTileSize));
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < columns; ++col) {
            batch.draw(groundTile_,
                       {col * kTileSize + kTileSize * 0.5f, groundY_ + row * kTileSize + kTileSize * 0.5f},
                       {kTileSize, kTileSize});
        }
    }

    // Title with a gentle bounce.
    const float titleY = 120.0f + std::sin(bob_ * 2.0f) * 6.0f;
    font.draw(batch, "MINI GAME", {world.x * 0.5f + 5.0f, titleY + 5.0f}, 110.0f,
              engine::Color{0, 0, 0, 0.3f}, engine::TextAlign::Center);
    font.draw(batch, "MINI GAME", {world.x * 0.5f, titleY}, 110.0f, engine::Color::rgb8(255, 241, 118),
              engine::TextAlign::Center);
    font.draw(batch, "HEAVEN", {world.x * 0.5f + 5.0f, titleY + 115.0f}, 110.0f,
              engine::Color{0, 0, 0, 0.3f}, engine::TextAlign::Center);
    font.draw(batch, "HEAVEN", {world.x * 0.5f, titleY + 110.0f}, 110.0f, engine::Color::white(),
              engine::TextAlign::Center);

    const std::vector<GameInfo>& games = allGames();
    for (size_t i = 0; i < cards_.size(); ++i) {
        const GameInfo& info = games[i];
        const engine::Rect& r = cards_[i].rect();
        cards_[i].draw(batch, font, 0.0f);

        font.draw(batch, info.title, {r.x + 30.0f, r.y + 22.0f}, 60.0f, engine::Color::white());
        font.draw(batch, info.available ? info.hint : "COMING SOON", {r.x + 30.0f, r.y + 92.0f}, 32.0f,
                  engine::Color::white().withAlpha(0.85f));

        if (info.available) {
            char best[32];
            std::snprintf(best, sizeof(best), "BEST %d", scores_.best(info.id));
            font.draw(batch, best, {r.right() - 30.0f, r.y + 54.0f}, 40.0f,
                      engine::Color::white().withAlpha(0.9f), engine::TextAlign::Right);
        }
    }

    const engine::Sprite& frame = dinoIdle_.frame();
    const engine::Vec2 size = frame.size() * 0.6f;
    batch.draw(frame, {world.x * 0.5f, groundY_ - size.y * 0.5f + 6.0f}, size);
}

void MenuScene::onTouch(const engine::TouchEvent& event) {
    const std::vector<GameInfo>& games = allGames();
    for (size_t i = 0; i < cards_.size(); ++i) {
        if (cards_[i].handleTouch(event) && games[i].available && onSelect_) {
            onSelect_(games[i].id);
            return;
        }
    }
}

}  // namespace app
