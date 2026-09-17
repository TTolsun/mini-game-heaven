#define LOG_TAG "JumpGame"
#include "app/games/JumpGame.h"

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

constexpr float kTile = 96.0f;
constexpr float kDinoX = 190.0f;
constexpr float kDinoScale = 0.55f;
constexpr float kGravity = 3200.0f;
constexpr float kJumpVelocity = -1150.0f;
constexpr float kDoubleJumpVelocity = -1000.0f;
constexpr float kStartSpeed = 400.0f;
constexpr float kMaxSpeed = 760.0f;
constexpr float kCoyoteTime = 0.10f;
constexpr float kJumpBufferTime = 0.12f;
constexpr float kCrateScale = 0.42f;     // crate ~80 world units: needs a real jump
constexpr float kMushroomScale = 0.55f;  // mushroom ~67: a hop clears it
constexpr float kDeathHold = 1.3f;

}  // namespace

JumpGame::JumpGame(const GameAssets& assets)
    : assets_(assets), particles_(std::random_device{}()), rng_(std::random_device{}()) {}

void JumpGame::onEnter(engine::Engine& engine) {
    engine_ = &engine;
    font_ = &assets_.font();
    background_ = assets_.forestBackground();
    runAnim_ = assets_.dinoRun();
    jumpAnim_ = assets_.dinoJump();
    deadAnim_ = assets_.dinoDead();

    groundY_ = engine.worldHeight() - 2.0f * kTile;
    dino_ = {kDinoX, groundY_};
    grounded_ = true;
    jumpsLeft_ = 2;
    speed_ = kStartSpeed;

    // Start with a safe stretch of ground: no gap for 14 columns and the
    // first obstacle can only appear once the player has had ~2.5 s to read the hint.
    runLeft_ = 14;
    sinceObstacle_ = -10;
    columns_.clear();
    generateAhead();

    // Background scenery scrolls at 60% speed for depth.
    scenery_.clear();
    std::uniform_int_distribution<int> kind(0, 3);
    for (float x = 0.0f; x < engine.worldWidth() * 2.0f; x += 260.0f) {
        scenery_.emplace_back(x, kind(rng_));
    }
}

void JumpGame::pushColumn() {
    const float x = columns_.empty() ? -kTile : columns_.back().x + kTile;
    Column column{x, true, 0, false, 0.0f};

    std::uniform_int_distribution<int> runLen(3, 7);
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);

    if (gapLeft_ > 0) {
        column.ground = false;
        --gapLeft_;
    } else if (runLeft_ > 0) {
        --runLeft_;
        ++sinceObstacle_;
        // Obstacles only on solid ground, never right after a gap, never two in a row.
        if (sinceObstacle_ >= 3 && runLeft_ >= 1 && chance(rng_) < 0.38f) {
            column.obstacle = chance(rng_) < 0.5f ? 1 : 2;
            sinceObstacle_ = 0;
        }
    } else {
        // Start a gap; wider gaps as the game speeds up, sometimes bridged by a platform.
        const float difficulty = (speed_ - kStartSpeed) / (kMaxSpeed - kStartSpeed);
        std::uniform_int_distribution<int> gapLen(1, 2 + static_cast<int>(difficulty * 2.0f));
        gapLeft_ = gapLen(rng_) - 1;
        column.ground = false;
        runLeft_ = runLen(rng_);
        sinceObstacle_ = 0;
        if (gapLeft_ >= 2 && chance(rng_) < 0.6f) {
            // Platform spans the gap, hovering above head height so a single jump reaches it.
            column.platform = true;
            column.platformY = groundY_ - 260.0f;
        }
    }

    // Extend a platform across the whole gap it started on.
    if (!column.ground && !column.platform && !columns_.empty() && columns_.back().platform &&
        !columns_.back().ground) {
        column.platform = true;
        column.platformY = columns_.back().platformY;
    }
    columns_.push_back(column);
}

void JumpGame::generateAhead() {
    const float needUntil = engine_->worldWidth() + 3.0f * kTile;
    while (columns_.empty() || columns_.back().x < needUntil) {
        pushColumn();
    }
}

bool JumpGame::groundUnder(float x) const {
    for (const Column& c : columns_) {
        if (x >= c.x && x < c.x + kTile) {
            return c.ground;
        }
    }
    return false;
}

// Top surface of a platform the player crosses when moving from yFrom to yTo, or -1.
float JumpGame::platformTopAt(float x, float yFrom, float yTo) const {
    for (const Column& c : columns_) {
        if (!c.platform || x < c.x || x >= c.x + kTile) {
            continue;
        }
        const float top = c.platformY - kTile * 0.5f + 12.0f;
        if (yFrom <= top && yTo >= top) {
            return top;
        }
    }
    return -1.0f;
}

void JumpGame::onTouch(const engine::TouchEvent& event) {
    if (event.phase != engine::TouchEvent::Phase::Down || death_ != DeathCause::None) {
        return;
    }
    jumpBuffer_ = kJumpBufferTime;
}

void JumpGame::jump() {
    const bool canGroundJump = grounded_ || coyote_ < kCoyoteTime;
    if (canGroundJump) {
        velocityY_ = kJumpVelocity;
        jumpsLeft_ = 1;
        engine_->mixer().play(assets_.sfx().jump);
        engine_->haptics().light();
    } else if (jumpsLeft_ > 0) {
        velocityY_ = kDoubleJumpVelocity;
        jumpsLeft_ = 0;
        engine_->mixer().play(assets_.sfx().doubleJump);
        engine_->haptics().light();
        engine::Particles::Burst puff;
        puff.count = 8;
        puff.speedMin = 120.0f;
        puff.speedMax = 260.0f;
        puff.angleMin = 0.2f;
        puff.angleMax = 2.9f;  // downward fan
        puff.gravity = 600.0f;
        puff.color = engine::Color::white();
        puff.colorAlt = engine::Color::rgb8(200, 230, 255);
        particles_.emit(dino_, puff);
    } else {
        return;
    }
    grounded_ = false;
    coyote_ = kCoyoteTime;
    jumpBuffer_ = 0.0f;
    jumpAnim_.restart();

    // Stretch on take-off, then spring back.
    squashFrom_ = {0.8f, 1.25f};
    squashTime_ = 0.0f;
}

void JumpGame::land() {
    grounded_ = true;
    jumpsLeft_ = 2;
    velocityY_ = 0.0f;
    coyote_ = 0.0f;

    squashFrom_ = {1.3f, 0.7f};
    squashTime_ = 0.0f;
    engine_->mixer().play(assets_.sfx().land, 0.7f);

    engine::Particles::Burst dust;
    dust.count = 10;
    dust.speedMin = 150.0f;
    dust.speedMax = 380.0f;
    dust.angleMin = 3.4f;   // upward fan (pi..2pi is up on screen)
    dust.angleMax = 6.0f;
    dust.gravity = 900.0f;
    dust.sizeMin = 8.0f;
    dust.sizeMax = 18.0f;
    dust.color = engine::Color::rgb8(255, 248, 220);
    dust.colorAlt = engine::Color::rgb8(186, 140, 90);
    particles_.emit({dino_.x, dino_.y - 4.0f}, dust);
}

void JumpGame::die(DeathCause cause) {
    if (death_ != DeathCause::None) {
        return;
    }
    death_ = cause;
    deadAnim_.restart();
    velocityY_ = cause == DeathCause::Obstacle ? -500.0f : 0.0f;

    engine_->hitStop(0.09f);
    engine_->addTrauma(cause == DeathCause::Obstacle ? 0.6f : 0.45f);
    engine_->haptics().heavy();
    engine_->mixer().play(cause == DeathCause::Obstacle ? assets_.sfx().hit : assets_.sfx().splash);

    engine::Particles::Burst burst;
    burst.count = 24;
    burst.speedMin = 200.0f;
    burst.speedMax = 700.0f;
    burst.lifeMin = 0.4f;
    burst.lifeMax = 0.8f;
    burst.sizeMin = 10.0f;
    burst.sizeMax = 22.0f;
    if (cause == DeathCause::Water) {
        burst.angleMin = 3.6f;
        burst.angleMax = 5.8f;
        burst.color = engine::Color::rgb8(140, 210, 255);
        burst.colorAlt = engine::Color::white();
    } else {
        burst.color = engine::Color::rgb8(255, 235, 59);
        burst.colorAlt = engine::Color::rgb8(255, 112, 67);
    }
    particles_.emit({dino_.x, dino_.y - 60.0f}, burst);
}

void JumpGame::updatePlayer(float dt) {
    if (jumpBuffer_ > 0.0f) {
        jumpBuffer_ -= dt;
        jump();
    }

    velocityY_ += kGravity * dt;
    const float prevY = dino_.y;
    dino_.y += velocityY_ * dt;

    if (!grounded_) {
        coyote_ += dt;
    }

    const bool solidBelow = groundUnder(dino_.x);
    if (velocityY_ >= 0.0f) {
        // Landing on a floating platform takes priority over the ground below it.
        const float platformTop = platformTopAt(dino_.x, prevY, dino_.y);
        if (platformTop >= 0.0f) {
            dino_.y = platformTop;
            if (!grounded_) {
                land();
            }
        } else if (solidBelow && prevY <= groundY_ && dino_.y >= groundY_) {
            dino_.y = groundY_;
            if (!grounded_) {
                land();
            }
        } else if (grounded_) {
            // Walked off an edge (or the platform ended).
            grounded_ = false;
            coyote_ = 0.0f;
            jumpsLeft_ = std::min(jumpsLeft_, 1);
        }
    }

    if (dino_.y > groundY_ + 110.0f) {
        die(DeathCause::Water);
    }

    if (grounded_) {
        runAnim_.update(dt);
    } else {
        jumpAnim_.update(dt);
    }
    squashTime_ = std::min(1.0f, squashTime_ + dt / 0.22f);
    const float e = engine::ease::outBack(squashTime_);
    squash_ = {squashFrom_.x + (1.0f - squashFrom_.x) * e, squashFrom_.y + (1.0f - squashFrom_.y) * e};
}

void JumpGame::updateWorld(float dt) {
    speed_ = std::min(kMaxSpeed, speed_ + 9.0f * dt);
    const float step = speed_ * dt;
    scroll_ += step;
    for (Column& c : columns_) {
        c.x -= step;
    }
    while (!columns_.empty() && columns_.front().x < -2.0f * kTile) {
        columns_.pop_front();
    }
    generateAhead();

    // Scenery scrolls slower and wraps around.
    const float wrap = engine_->worldWidth() * 2.0f;
    for (auto& [x, kind] : scenery_) {
        x -= step * 0.6f;
        if (x < -200.0f) {
            x += wrap;
        }
    }

    // Obstacle collisions.
    const engine::Vec2 dinoSize = runAnim_.frame().size() * kDinoScale;
    const engine::Rect dinoBox = engine::Rect::fromCenter({dino_.x, dino_.y - dinoSize.y * 0.5f},
                                                          {dinoSize.x * 0.4f, dinoSize.y * 0.75f});
    for (const Column& c : columns_) {
        if (c.obstacle == 0) {
            continue;
        }
        const engine::Sprite sprite = c.obstacle == 1 ? assets_.crate() : assets_.mushroomPink();
        const engine::Vec2 size = sprite.size() * (c.obstacle == 1 ? kCrateScale : kMushroomScale);
        const engine::Rect box = engine::Rect::fromCenter({c.x + kTile * 0.5f, groundY_ - size.y * 0.5f},
                                                          size * 0.75f);
        if (box.overlaps(dinoBox)) {
            die(DeathCause::Obstacle);
            break;
        }
    }
    score_ = static_cast<int>(scroll_ / 10.0f);
}

void JumpGame::update(float dt) {
    if (finished_) {
        return;
    }
    particles_.update(dt);

    if (death_ != DeathCause::None) {
        deadAnim_.update(dt);
        deathTimer_ += dt;
        if (death_ == DeathCause::Obstacle) {
            velocityY_ += kGravity * dt;
            dino_.y = std::min(groundY_, dino_.y + velocityY_ * dt);
        } else {
            dino_.y += 120.0f * dt;  // sink
        }
        if (deathTimer_ >= kDeathHold) {
            finished_ = true;
        }
        return;
    }

    updateWorld(dt);
    updatePlayer(dt);
}

void JumpGame::drawWorld(engine::SpriteBatch& batch) {
    const engine::Vec2 world = engine_->worldSize();
    const engine::Sprite grass = assets_.groundTile();
    const engine::Sprite dirt = assets_.dirtTile();
    const engine::Sprite waterTop = assets_.waterTop();
    const engine::Sprite water = assets_.water();

    for (const auto& [x, kind] : scenery_) {
        const engine::Sprite s = kind < 2 ? assets_.bush(kind) : assets_.tree(kind - 2);
        const float scale = kind < 2 ? 0.5f : 0.7f;
        batch.draw(s, {x, groundY_ - s.height * scale * 0.5f + 10.0f}, scale,
                   engine::Color::white().withAlpha(0.85f));
    }

    for (const Column& c : columns_) {
        const float cx = c.x + kTile * 0.5f;
        if (c.ground) {
            batch.draw(grass, {cx, groundY_ + kTile * 0.5f}, {kTile, kTile});
            for (float y = groundY_ + kTile; y < world.y; y += kTile) {
                batch.draw(dirt, {cx, y + kTile * 0.5f}, {kTile, kTile});
            }
        } else {
            const float waterY = groundY_ + 36.0f;
            batch.draw(waterTop, {cx, waterY + kTile * 0.5f}, {kTile, kTile});
            for (float y = waterY + kTile; y < world.y; y += kTile) {
                batch.draw(water, {cx, y + kTile * 0.5f}, {kTile, kTile});
            }
        }
    }

    // Platforms: pick left/mid/right piece by looking at the neighbours.
    for (size_t i = 0; i < columns_.size(); ++i) {
        const Column& c = columns_[i];
        if (!c.platform) {
            continue;
        }
        const bool leftEnd = i == 0 || !columns_[i - 1].platform;
        const bool rightEnd = i + 1 >= columns_.size() || !columns_[i + 1].platform;
        const engine::Sprite piece = leftEnd ? assets_.platformLeft()
                                    : rightEnd ? assets_.platformRight()
                                               : assets_.platformMid();
        batch.draw(piece, {c.x + kTile * 0.5f, c.platformY}, {kTile, kTile});
    }

    for (const Column& c : columns_) {
        if (c.obstacle != 0) {
            const engine::Sprite sprite = c.obstacle == 1 ? assets_.crate() : assets_.mushroomPink();
            const engine::Vec2 size = sprite.size() * (c.obstacle == 1 ? kCrateScale : kMushroomScale);
            batch.draw(sprite, {c.x + kTile * 0.5f, groundY_ - size.y * 0.5f + 4.0f}, size);
        }
    }

    const engine::Sprite& frame = death_ != DeathCause::None ? deadAnim_.frame()
                                  : grounded_                ? runAnim_.frame()
                                                             : jumpAnim_.frame();
    const engine::Vec2 size{frame.width * kDinoScale * squash_.x, frame.height * kDinoScale * squash_.y};
    const engine::Color tint = death_ == DeathCause::Water ? engine::Color::white().withAlpha(0.6f)
                                                           : engine::Color::white();
    batch.draw(frame, {dino_.x, dino_.y - size.y * 0.5f + 6.0f}, size, tint);

    particles_.render(batch);
}

void JumpGame::drawHud(engine::SpriteBatch& batch) {
    const float w = engine_->worldWidth();
    const float top = engine_->safeTop() + 16.0f;
    char text[32];
    std::snprintf(text, sizeof(text), "%d m", score_);
    ui::shadowText(batch, *font_, text, {w * 0.5f, top}, ui::kHeadline, engine::Color::white(),
                   engine::TextAlign::Center);

    if (death_ != DeathCause::None) {
        const char* msg = death_ == DeathCause::Water ? "SPLASH!" : "OUCH!";
        ui::shadowText(batch, *font_, msg, {w * 0.5f, top + 200.0f}, 96.0f,
                       engine::Color::rgb8(255, 235, 59), engine::TextAlign::Center);
    } else if (scroll_ < 600.0f) {
        ui::shadowText(batch, *font_, "TAP TO JUMP", {w * 0.5f, top + 260.0f}, ui::kBody,
                       engine::Color::white().withAlpha(1.0f - scroll_ / 600.0f), engine::TextAlign::Center);
    }
}

void JumpGame::render(engine::SpriteBatch& batch) {
    const engine::Vec2 world = engine_->worldSize();
    const float bgScale = std::max(world.x / background_.width, world.y / background_.height);
    // Parallax: the background drifts at a fraction of the scroll speed.
    const float bgWidth = background_.width * bgScale;
    const float bgShift = std::fmod(scroll_ * 0.15f, bgWidth);
    batch.draw(background_, {world.x * 0.5f - bgShift, world.y * 0.5f}, bgScale);
    batch.draw(background_, {world.x * 0.5f - bgShift + bgWidth, world.y * 0.5f}, bgScale);

    drawWorld(batch);
    drawHud(batch);
}

}  // namespace app
