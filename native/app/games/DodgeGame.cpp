#define LOG_TAG "DodgeGame"
#include "app/games/DodgeGame.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "app/GameAssets.h"
#include "app/ui/Text.h"
#include "engine/Engine.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/math/Ease.h"
#include "engine/math/Rect.h"

namespace app {

namespace {

constexpr float kDinoScale = 0.6f;
constexpr float kDinoSpeed = 640.0f;
constexpr float kTile = 96.0f;
constexpr float kObjectScale = 0.55f;
constexpr float kNearMiss = 110.0f;   // horizontal gap that counts as a close call
constexpr float kDeathHold = 1.2f;    // seconds to show the dead pose before finishing

// Difficulty curve: spawn interval shrinks and fall speed grows with time.
float spawnIntervalAt(float t) { return std::max(0.3f, 0.95f - t * 0.025f); }
float fallSpeedAt(float t) { return 430.0f + std::min(t, 60.0f) * 14.0f; }

}  // namespace

DodgeGame::DodgeGame(const GameAssets& assets)
    : assets_(assets), particles_(std::random_device{}()), rng_(std::random_device{}()) {}

void DodgeGame::onEnter(engine::Engine& engine) {
    engine_ = &engine;
    background_ = assets_.forestBackground();
    groundTile_ = assets_.groundTile();
    dirtTile_ = assets_.dirtTile();
    idleAnim_ = assets_.dinoIdle();
    runAnim_ = assets_.dinoRun();
    deadAnim_ = assets_.dinoDead();
    font_ = &assets_.font();

    groundY_ = engine.worldHeight() - 2.0f * kTile;
    dinoPos_ = {engine.worldWidth() * 0.5f, groundY_};
    spawnTimer_ = 0.8f;
}

void DodgeGame::onTouch(const engine::TouchEvent& event) {
    using Phase = engine::TouchEvent::Phase;
    if (dead_) {
        return;
    }
    const int dir = event.position.x < engine_->worldWidth() * 0.5f ? -1 : 1;
    switch (event.phase) {
    case Phase::Down:
        activePointer_ = event.pointerId;
        if (moveDir_ != dir) {
            // Quick lean when changing direction.
            squashFrom_ = {0.85f, 1.1f};
            squashTime_ = 0.0f;
        }
        moveDir_ = dir;
        break;
    case Phase::Move:
        if (event.pointerId == activePointer_) {
            moveDir_ = dir;
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

void DodgeGame::spawn() {
    std::uniform_real_distribution<float> x(70.0f, engine_->worldWidth() - 70.0f);
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);
    std::uniform_real_distribution<float> spin(-3.0f, 3.0f);

    Falling f;
    f.kind = chance(rng_) < 0.4f ? Kind::Crate : Kind::Stone;
    f.sprite = f.kind == Kind::Crate ? assets_.crate() : assets_.stone();
    f.position = {x(rng_), -f.sprite.height * kObjectScale};
    f.speed = fallSpeedAt(elapsed_) * (f.kind == Kind::Stone ? 1.15f : 0.95f);
    f.spin = f.kind == Kind::Crate ? spin(rng_) : 0.0f;
    falling_.push_back(f);
}

void DodgeGame::land(Falling& object) {
    object.landed = 0.0f;
    const engine::Vec2 size = object.sprite.size() * kObjectScale;
    object.position.y = groundY_ - size.y * 0.5f;

    engine::Particles::Burst burst;
    burst.angleMin = 3.5f;
    burst.angleMax = 5.9f;
    burst.gravity = 1200.0f;
    if (object.kind == Kind::Stone) {
        // Heavy: dust puff, a thump you can feel.
        burst.count = 10;
        burst.speedMin = 150.0f;
        burst.speedMax = 380.0f;
        burst.sizeMin = 8.0f;
        burst.sizeMax = 18.0f;
        burst.color = engine::Color::rgb8(255, 248, 220);
        burst.colorAlt = engine::Color::rgb8(186, 140, 90);
        engine_->mixer().play(assets_.sfx().thud, 0.8f);
        if (!dead_) {
            engine_->addTrauma(0.18f);
        }
    } else {
        // Crate shatters into planks.
        burst.count = 14;
        burst.speedMin = 250.0f;
        burst.speedMax = 620.0f;
        burst.sizeMin = 10.0f;
        burst.sizeMax = 22.0f;
        burst.lifeMin = 0.4f;
        burst.lifeMax = 0.7f;
        burst.color = engine::Color::rgb8(198, 132, 74);
        burst.colorAlt = engine::Color::rgb8(120, 72, 36);
        engine_->mixer().play(assets_.sfx().crack, 0.7f);
    }
    particles_.emit({object.position.x, groundY_}, burst);

    // Near miss: landed right next to the dino without touching -> bonus.
    if (!dead_ && !object.nearMissChecked) {
        const float gap = std::fabs(object.position.x - dinoPos_.x);
        if (gap < kNearMiss) {
            bonus_ += 5;
            popups_.add("CLOSE! +5", {dinoPos_.x, groundY_ - 200.0f}, engine::Color::rgb8(255, 241, 118));
            engine_->mixer().play(assets_.sfx().ding);
            engine_->haptics().light();
        }
    }
    object.nearMissChecked = true;
}

void DodgeGame::hit() {
    dead_ = true;
    moveDir_ = 0;
    deadAnim_.restart();
    engine_->hitStop(0.09f);
    engine_->addTrauma(0.6f);
    engine_->haptics().heavy();
    engine_->mixer().play(assets_.sfx().hit);

    engine::Particles::Burst burst;
    burst.count = 22;
    burst.speedMin = 200.0f;
    burst.speedMax = 700.0f;
    burst.lifeMin = 0.4f;
    burst.lifeMax = 0.8f;
    burst.sizeMin = 10.0f;
    burst.sizeMax = 22.0f;
    burst.color = engine::Color::rgb8(255, 235, 59);
    burst.colorAlt = engine::Color::rgb8(255, 112, 67);
    particles_.emit({dinoPos_.x, groundY_ - 70.0f}, burst);
}

void DodgeGame::updateDino(float dt) {
    if (moveDir_ != 0) {
        dinoPos_.x += moveDir_ * kDinoSpeed * dt;
        facingLeft_ = moveDir_ < 0;
        runAnim_.update(dt);

        // Little dust puffs at the feet while running.
        dustTimer_ -= dt;
        if (dustTimer_ <= 0.0f) {
            dustTimer_ = 0.11f;
            engine::Particles::Burst puff;
            puff.count = 2;
            puff.speedMin = 40.0f;
            puff.speedMax = 120.0f;
            puff.angleMin = facingLeft_ ? -0.6f : 3.14f;
            puff.angleMax = puff.angleMin + 0.9f;
            puff.gravity = -60.0f;
            puff.lifeMin = 0.25f;
            puff.lifeMax = 0.4f;
            puff.sizeMin = 6.0f;
            puff.sizeMax = 12.0f;
            puff.color = engine::Color::rgb8(255, 248, 220).withAlpha(0.7f);
            puff.colorAlt = engine::Color::rgb8(220, 200, 160).withAlpha(0.7f);
            particles_.emit({dinoPos_.x - moveDir_ * 20.0f, groundY_ - 6.0f}, puff);
        }
    } else {
        idleAnim_.update(dt);
    }
    const float halfWidth = idleAnim_.frame().width * kDinoScale * 0.35f;
    dinoPos_.x = std::clamp(dinoPos_.x, halfWidth, engine_->worldWidth() - halfWidth);

    squashTime_ = std::min(1.0f, squashTime_ + dt / 0.2f);
    const float e = engine::ease::outBack(squashTime_);
    squash_ = {squashFrom_.x + (1.0f - squashFrom_.x) * e, squashFrom_.y + (1.0f - squashFrom_.y) * e};
}

void DodgeGame::updateFalling(float dt) {
    const engine::Sprite& dinoFrame = moveDir_ != 0 ? runAnim_.frame() : idleAnim_.frame();
    const engine::Vec2 dinoSize = dinoFrame.size() * kDinoScale;
    // Hit box is tighter than the sprite so near misses feel fair.
    const engine::Rect dinoBox = engine::Rect::fromCenter(
        {dinoPos_.x, groundY_ - dinoSize.y * 0.5f}, {dinoSize.x * 0.42f, dinoSize.y * 0.8f});

    for (Falling& f : falling_) {
        if (f.landed >= 0.0f) {
            f.landed += dt;
            continue;
        }
        f.position.y += f.speed * dt;
        f.angle += f.spin * dt;

        const engine::Vec2 size = f.sprite.size() * kObjectScale;
        const engine::Rect box = engine::Rect::fromCenter(f.position, size * 0.72f);
        if (!dead_ && box.overlaps(dinoBox)) {
            hit();
        }
        if (f.position.y + size.y * 0.5f >= groundY_) {
            land(f);
            if (!dead_) {
                ++dodged_;
            }
        }
    }

    // Crates vanish once shattered; stones stay a moment then sink away.
    falling_.erase(std::remove_if(falling_.begin(), falling_.end(),
                                  [](const Falling& f) {
                                      return f.landed > (f.kind == Kind::Crate ? 0.05f : 0.6f);
                                  }),
                   falling_.end());
}

void DodgeGame::update(float dt) {
    if (finished_) {
        return;
    }
    particles_.update(dt);
    popups_.update(dt);

    if (dead_) {
        deadAnim_.update(dt);
        deathTimer_ += dt;
        updateFalling(dt);
        if (deathTimer_ >= kDeathHold) {
            finished_ = true;
        }
        return;
    }

    elapsed_ += dt;
    spawnTimer_ -= dt;
    if (spawnTimer_ <= 0.0f) {
        spawn();
        spawnTimer_ = spawnIntervalAt(elapsed_);
    }

    updateDino(dt);
    updateFalling(dt);
    score_ = static_cast<int>(elapsed_ * 10.0f) + dodged_ * 5 + bonus_;
}

void DodgeGame::render(engine::SpriteBatch& batch) {
    const engine::Vec2 world = engine_->worldSize();

    const float bgScale = std::max(world.x / background_.width, world.y / background_.height);
    batch.draw(background_, world * 0.5f, bgScale);

    const int columns = static_cast<int>(std::ceil(world.x / kTile));
    for (int col = 0; col < columns; ++col) {
        const float cx = col * kTile + kTile * 0.5f;
        batch.draw(groundTile_, {cx, groundY_ + kTile * 0.5f}, {kTile, kTile});
        for (float y = groundY_ + kTile; y < world.y; y += kTile) {
            batch.draw(dirtTile_, {cx, y + kTile * 0.5f}, {kTile, kTile});
        }
    }

    for (const Falling& f : falling_) {
        engine::Vec2 size = f.sprite.size() * kObjectScale;
        engine::Color tint = engine::Color::white();
        engine::Vec2 pos = f.position;
        if (f.landed >= 0.0f) {
            if (f.kind == Kind::Crate) {
                continue;  // shattered into particles
            }
            // Stone: quick squash on impact, then sink into the ground.
            const float t = std::min(1.0f, f.landed / 0.6f);
            const float sq = engine::ease::outBack(std::min(1.0f, f.landed / 0.18f), 2.0f);
            size = {size.x * (1.25f + (1.0f - 1.25f) * sq), size.y * (0.7f + (1.0f - 0.7f) * sq)};
            pos.y = groundY_ - size.y * 0.5f + t * t * size.y;
            tint = tint.withAlpha(1.0f - t * t);
        }
        // Shadow on the ground grows as the object approaches.
        const float shadowT = std::clamp(1.0f - (groundY_ - f.position.y) / world.y, 0.2f, 1.0f);
        batch.drawQuad({f.position.x, groundY_ + 6.0f}, {size.x * 0.9f * shadowT, 18.0f * shadowT},
                       engine::Color{0, 0, 0, 0.2f * shadowT});
        batch.draw(f.sprite, pos, size, tint, f.angle);
    }

    const engine::Sprite& frame =
        dead_ ? deadAnim_.frame() : (moveDir_ != 0 ? runAnim_.frame() : idleAnim_.frame());
    const engine::Vec2 size{frame.width * kDinoScale * squash_.x, frame.height * kDinoScale * squash_.y};
    batch.draw(frame, {dinoPos_.x, groundY_ - size.y * 0.5f + 6.0f}, size, engine::Color::white(),
               0.0f, facingLeft_);

    particles_.render(batch);
    popups_.render(batch, *font_);
    drawHud(batch);
}

void DodgeGame::drawHud(engine::SpriteBatch& batch) {
    const float w = engine_->worldWidth();
    const float top = engine_->safeTop() + 16.0f;
    char text[32];

    std::snprintf(text, sizeof(text), "%d", score_);
    ui::shadowText(batch, *font_, text, {w * 0.5f, top}, ui::kHeadline, engine::Color::white(),
                   engine::TextAlign::Center);

    std::snprintf(text, sizeof(text), "%.1fs", elapsed_);
    ui::shadowText(batch, *font_, text, {w - 24.0f, top + 12.0f}, ui::kBody,
                   engine::Color::white().withAlpha(0.85f), engine::TextAlign::Right);

    if (dead_) {
        ui::shadowText(batch, *font_, "OUCH!", {w * 0.5f, top + 200.0f}, 96.0f,
                       engine::Color::rgb8(255, 235, 59), engine::TextAlign::Center);
    } else if (elapsed_ < 2.5f) {
        ui::shadowText(batch, *font_, "HOLD LEFT / RIGHT", {w * 0.5f, top + 260.0f}, ui::kBody,
                       engine::Color::white().withAlpha(1.0f - elapsed_ / 2.5f), engine::TextAlign::Center);
    }
}

}  // namespace app
