#define LOG_TAG "DodgeGame"
#include "app/games/DodgeGame.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "app/GameAssets.h"
#include "engine/Engine.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/math/Rect.h"

namespace app {

namespace {

constexpr float kDinoScale = 0.6f;
constexpr float kDinoSpeed = 620.0f;
constexpr float kTileSize = 96.0f;
constexpr float kGroundRows = 2.0f;
constexpr float kJellyScale = 0.55f;
constexpr float kDeathHold = 1.2f;  // seconds to show the dead pose before finishing

// Difficulty curve: spawn interval shrinks and fall speed grows with time.
float spawnIntervalAt(float t) { return std::max(0.28f, 0.9f - t * 0.025f); }
float fallSpeedAt(float t) { return 420.0f + std::min(t, 60.0f) * 14.0f; }

}  // namespace

DodgeGame::DodgeGame(const GameAssets& assets)
    : assets_(assets), rng_(std::random_device{}()) {}

void DodgeGame::onEnter(engine::Engine& engine) {
    engine_ = &engine;
    background_ = assets_.forestBackground();
    groundTile_ = assets_.groundTile();
    idleAnim_ = assets_.dinoIdle();
    runAnim_ = assets_.dinoRun();
    deadAnim_ = assets_.dinoDead();
    font_ = &assets_.font();

    groundY_ = engine.worldHeight() - kGroundRows * kTileSize;
    dinoPos_ = {engine.worldWidth() * 0.5f, groundY_};
    spawnTimer_ = 0.6f;
}

void DodgeGame::onTouch(const engine::TouchEvent& event) {
    using Phase = engine::TouchEvent::Phase;
    if (dead_) {
        return;
    }
    switch (event.phase) {
    case Phase::Down:
        activePointer_ = event.pointerId;
        moveDir_ = event.position.x < engine_->worldWidth() * 0.5f ? -1 : 1;
        break;
    case Phase::Move:
        if (event.pointerId == activePointer_) {
            moveDir_ = event.position.x < engine_->worldWidth() * 0.5f ? -1 : 1;
        }
        break;
    case Phase::Up:
    case Phase::Cancel:
        if (event.pointerId == activePointer_) {
            activePointer_ = -1;
            moveDir_ = 0;
        }
        break;
    }
}

void DodgeGame::spawnJelly() {
    std::uniform_int_distribution<int> pick(1, 6);
    std::uniform_real_distribution<float> x(70.0f, engine_->worldWidth() - 70.0f);
    std::uniform_real_distribution<float> spin(-2.5f, 2.5f);

    Jelly jelly;
    jelly.sprite = assets_.jelly(pick(rng_));
    jelly.position = {x(rng_), -jelly.sprite.height * kJellyScale};
    jelly.speed = fallSpeedAt(elapsed_);
    jelly.spin = spin(rng_);
    jellies_.push_back(jelly);
}

void DodgeGame::updateDino(float dt) {
    if (moveDir_ != 0) {
        dinoPos_.x += moveDir_ * kDinoSpeed * dt;
        facingLeft_ = moveDir_ < 0;
        runAnim_.update(dt);
    } else {
        idleAnim_.update(dt);
    }
    const float halfWidth = idleAnim_.frame().width * kDinoScale * 0.35f;
    dinoPos_.x = std::clamp(dinoPos_.x, halfWidth, engine_->worldWidth() - halfWidth);
}

void DodgeGame::updateJellies(float dt) {
    const engine::Sprite& dinoFrame = moveDir_ != 0 ? runAnim_.frame() : idleAnim_.frame();
    const engine::Vec2 dinoSize = dinoFrame.size() * kDinoScale;
    // Hit box is tighter than the sprite so near misses feel fair.
    const engine::Rect dinoBox = engine::Rect::fromCenter(
        {dinoPos_.x, groundY_ - dinoSize.y * 0.5f}, {dinoSize.x * 0.45f, dinoSize.y * 0.8f});

    for (Jelly& jelly : jellies_) {
        if (jelly.squash > 0.0f) {
            jelly.squash += dt;
            continue;
        }
        jelly.position.y += jelly.speed * dt;
        jelly.angle += jelly.spin * dt;

        const engine::Vec2 size = jelly.sprite.size() * kJellyScale;
        const engine::Rect box = engine::Rect::fromCenter(jelly.position, size * 0.7f);
        if (!dead_ && box.overlaps(dinoBox)) {
            dead_ = true;
            moveDir_ = 0;
            deadAnim_.restart();
        }
        if (jelly.position.y + size.y * 0.5f >= groundY_) {
            jelly.position.y = groundY_ - size.y * 0.5f;
            jelly.squash = 0.001f;
            if (!dead_) {
                ++dodged_;
            }
        }
    }

    // Remove jellies once their splat has played out.
    jellies_.erase(std::remove_if(jellies_.begin(), jellies_.end(),
                                  [](const Jelly& j) { return j.squash > 0.35f; }),
                   jellies_.end());
}

void DodgeGame::update(float dt) {
    if (finished_) {
        return;
    }

    if (dead_) {
        deadAnim_.update(dt);
        deathTimer_ += dt;
        updateJellies(dt);
        if (deathTimer_ >= kDeathHold) {
            finished_ = true;
        }
        return;
    }

    elapsed_ += dt;
    spawnTimer_ -= dt;
    if (spawnTimer_ <= 0.0f) {
        spawnJelly();
        spawnTimer_ = spawnIntervalAt(elapsed_);
    }

    updateDino(dt);
    updateJellies(dt);
    score_ = static_cast<int>(elapsed_ * 10.0f) + dodged_ * 5;
}

void DodgeGame::render(engine::SpriteBatch& batch) {
    const engine::Vec2 world = engine_->worldSize();

    const float bgScale = std::max(world.x / background_.width, world.y / background_.height);
    batch.draw(background_, world * 0.5f, bgScale);

    const int columns = static_cast<int>(std::ceil(world.x / kTileSize));
    for (int row = 0; row < static_cast<int>(kGroundRows); ++row) {
        for (int col = 0; col < columns; ++col) {
            batch.draw(groundTile_,
                       {col * kTileSize + kTileSize * 0.5f, groundY_ + row * kTileSize + kTileSize * 0.5f},
                       {kTileSize, kTileSize});
        }
    }

    for (const Jelly& jelly : jellies_) {
        engine::Vec2 size = jelly.sprite.size() * kJellyScale;
        float angle = jelly.angle;
        if (jelly.squash > 0.0f) {
            // Splat: flatten quickly, then fade out.
            const float t = std::min(jelly.squash / 0.35f, 1.0f);
            size = {size.x * (1.0f + 0.5f * t), size.y * (1.0f - 0.7f * t)};
            angle = 0.0f;
            batch.draw(jelly.sprite, {jelly.position.x, groundY_ - size.y * 0.5f}, size,
                       engine::Color::white().withAlpha(1.0f - t), angle);
        } else {
            batch.draw(jelly.sprite, jelly.position, size, engine::Color::white(), angle);
        }
    }

    const engine::Sprite& frame =
        dead_ ? deadAnim_.frame() : (moveDir_ != 0 ? runAnim_.frame() : idleAnim_.frame());
    const engine::Vec2 size = frame.size() * kDinoScale;
    batch.draw(frame, {dinoPos_.x, groundY_ - size.y * 0.5f + 6.0f}, size, engine::Color::white(),
               0.0f, facingLeft_);

    drawHud(batch);
}

void DodgeGame::drawHud(engine::SpriteBatch& batch) {
    const float w = engine_->worldWidth();
    char text[32];

    std::snprintf(text, sizeof(text), "%d", score_);
    font_->draw(batch, text, {w * 0.5f + 3.0f, 43.0f}, 72.0f, engine::Color{0, 0, 0, 0.35f},
                engine::TextAlign::Center);
    font_->draw(batch, text, {w * 0.5f, 40.0f}, 72.0f, engine::Color::white(),
                engine::TextAlign::Center);

    std::snprintf(text, sizeof(text), "%.1fs", elapsed_);
    font_->draw(batch, text, {24.0f, 30.0f}, 36.0f, engine::Color::white().withAlpha(0.85f));

    if (dead_) {
        font_->draw(batch, "OUCH!", {w * 0.5f + 4.0f, 244.0f}, 96.0f, engine::Color{0, 0, 0, 0.35f},
                    engine::TextAlign::Center);
        font_->draw(batch, "OUCH!", {w * 0.5f, 240.0f}, 96.0f, engine::Color::rgb8(255, 235, 59),
                    engine::TextAlign::Center);
    }
}

}  // namespace app
