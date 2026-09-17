#define LOG_TAG "TapGame"
#include "app/games/TapGame.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "app/GameAssets.h"
#include "engine/Engine.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/math/Ease.h"

namespace app {

namespace {

constexpr float kJellyScale = 1.6f;
constexpr float kComboWindow = 0.45f;  // seconds between taps to keep a combo alive
constexpr float kEndHold = 1.4f;

}  // namespace

TapGame::TapGame(const GameAssets& assets)
    : assets_(assets), particles_(std::random_device{}()), rng_(std::random_device{}()) {}

void TapGame::onEnter(engine::Engine& engine) {
    engine_ = &engine;
    font_ = &assets_.font();
    background_ = assets_.forestBackground();
    jellyPos_ = {engine.worldWidth() * 0.5f, engine.worldHeight() * 0.52f};
}

void TapGame::onTouch(const engine::TouchEvent& event) {
    if (event.phase != engine::TouchEvent::Phase::Down || timeLeft_ <= 0.0f) {
        return;
    }
    started_ = true;
    tap(event.position);
}

void TapGame::tap(engine::Vec2 where) {
    ++taps_;
    combo_ = lastTapAge_ <= kComboWindow ? combo_ + 1 : 1;
    bestCombo_ = std::max(bestCombo_, combo_);
    lastTapAge_ = 0.0f;
    popTime_ = 0.0f;

    // Popup rises from wherever the finger landed, biased toward the jelly.
    Popup popup;
    popup.position = {where.x * 0.4f + jellyPos_.x * 0.6f, jellyPos_.y - 140.0f};
    popup.value = 1;
    popups_.push_back(popup);

    engine::Particles::Burst spark;
    spark.count = 6 + std::min(combo_, 12);
    spark.speedMin = 250.0f;
    spark.speedMax = 650.0f;
    spark.lifeMin = 0.25f;
    spark.lifeMax = 0.5f;
    spark.sizeMin = 8.0f;
    spark.sizeMax = 16.0f;
    spark.gravity = 1400.0f;
    spark.color = engine::Color::rgb8(255, 241, 118);
    spark.colorAlt = engine::Color::rgb8(255, 138, 101);
    particles_.emit(jellyPos_, spark);

    // Every tenth tap: swap the jelly, kick the screen a little.
    if (taps_ % 10 == 0) {
        std::uniform_int_distribution<int> pick(1, 6);
        int next = pick(rng_);
        if (next == jellyIndex_) {
            next = next % 6 + 1;
        }
        jellyIndex_ = next;
        engine_->addTrauma(0.3f);
    }
}

void TapGame::update(float dt) {
    if (finished_) {
        return;
    }
    particles_.update(dt);
    wobble_ += dt;
    lastTapAge_ += dt;
    popTime_ = std::min(1.0f, popTime_ + dt / 0.2f);
    if (lastTapAge_ > kComboWindow) {
        combo_ = 0;
    }

    for (Popup& p : popups_) {
        p.age += dt;
    }
    popups_.erase(std::remove_if(popups_.begin(), popups_.end(), [](const Popup& p) { return p.age > 0.7f; }),
                  popups_.end());

    if (!started_) {
        return;
    }
    if (timeLeft_ > 0.0f) {
        timeLeft_ -= dt;
        if (timeLeft_ <= 0.0f) {
            timeLeft_ = 0.0f;
            engine_->hitStop(0.1f);
            engine_->addTrauma(0.5f);
            engine::Particles::Burst confetti;
            confetti.count = 40;
            confetti.speedMin = 300.0f;
            confetti.speedMax = 900.0f;
            confetti.angleMin = 3.5f;
            confetti.angleMax = 5.9f;
            confetti.lifeMin = 0.6f;
            confetti.lifeMax = 1.1f;
            confetti.sizeMin = 10.0f;
            confetti.sizeMax = 22.0f;
            confetti.gravity = 1000.0f;
            confetti.color = engine::Color::rgb8(129, 199, 132);
            confetti.colorAlt = engine::Color::rgb8(100, 181, 246);
            particles_.emit({jellyPos_.x, jellyPos_.y + 60.0f}, confetti);
        }
    } else {
        endTimer_ += dt;
        if (endTimer_ >= kEndHold) {
            finished_ = true;
        }
    }
}

void TapGame::render(engine::SpriteBatch& batch) {
    const engine::Vec2 world = engine_->worldSize();
    const float bgScale = std::max(world.x / background_.width, world.y / background_.height);
    batch.draw(background_, world * 0.5f, bgScale);

    // Jelly: squash on tap (wide and flat), overshoot back; gentle idle wobble otherwise.
    const float e = engine::ease::outBack(popTime_, 2.2f);
    const float idle = std::sin(wobble_ * 3.0f) * 0.03f;
    const engine::Vec2 squash{1.35f + (1.0f - 1.35f) * e + idle, 0.65f + (1.0f - 0.65f) * e - idle};
    const engine::Sprite jelly = assets_.jelly(jellyIndex_);
    const engine::Vec2 size{jelly.width * kJellyScale * squash.x, jelly.height * kJellyScale * squash.y};
    // Anchor at the bottom so the squash looks like it presses into the floor.
    const float bottom = jellyPos_.y + jelly.height * kJellyScale * 0.5f;
    batch.drawQuad({jellyPos_.x, bottom + 10.0f}, {size.x * 0.9f, 30.0f}, engine::Color{0, 0, 0, 0.18f});
    batch.draw(jelly, {jellyPos_.x, bottom - size.y * 0.5f}, size);

    particles_.render(batch);

    for (const Popup& p : popups_) {
        const float t = p.age / 0.7f;
        const float rise = engine::ease::outCubic(t) * 160.0f;
        const float scale = 0.7f + 0.3f * engine::ease::outBack(std::min(1.0f, t * 3.0f));
        char text[16];
        std::snprintf(text, sizeof(text), "+%d", p.value);
        font_->draw(batch, text, {p.position.x, p.position.y - rise}, 64.0f * scale,
                    engine::Color::rgb8(255, 241, 118).withAlpha(1.0f - t * t), engine::TextAlign::Center);
    }

    // HUD.
    char text[32];
    std::snprintf(text, sizeof(text), "%.1f", timeLeft_);
    const engine::Color timeColor = timeLeft_ <= 3.0f && timeLeft_ > 0.0f ? engine::Color::rgb8(255, 112, 67)
                                                                           : engine::Color::white();
    font_->draw(batch, text, {world.x * 0.5f + 3.0f, 43.0f}, 84.0f, engine::Color{0, 0, 0, 0.35f},
                engine::TextAlign::Center);
    font_->draw(batch, text, {world.x * 0.5f, 40.0f}, 84.0f, timeColor, engine::TextAlign::Center);

    std::snprintf(text, sizeof(text), "%d", taps_);
    font_->draw(batch, text, {world.x * 0.5f, world.y * 0.22f}, 140.0f, engine::Color::white(),
                engine::TextAlign::Center);
    font_->draw(batch, "TAPS", {world.x * 0.5f, world.y * 0.22f + 140.0f}, 36.0f,
                engine::Color::white().withAlpha(0.8f), engine::TextAlign::Center);

    if (combo_ >= 3 && timeLeft_ > 0.0f) {
        std::snprintf(text, sizeof(text), "%d COMBO", combo_);
        const float pulse = 1.0f + 0.15f * engine::ease::pulse(std::min(1.0f, lastTapAge_ / 0.2f));
        font_->draw(batch, text, {world.x * 0.5f, world.y * 0.80f}, 64.0f * pulse,
                    engine::Color::rgb8(255, 138, 101), engine::TextAlign::Center);
    }

    if (!started_) {
        font_->draw(batch, "TAP! TAP! TAP!", {world.x * 0.5f, world.y * 0.80f}, 56.0f,
                    engine::Color::white().withAlpha(0.7f + 0.3f * std::sin(wobble_ * 6.0f)),
                    engine::TextAlign::Center);
    } else if (timeLeft_ <= 0.0f) {
        const float t = std::min(1.0f, endTimer_ / 0.3f);
        const float scale = engine::ease::outBack(t, 2.5f);
        font_->draw(batch, "TIME UP!", {world.x * 0.5f + 4.0f, world.y * 0.80f + 4.0f}, 96.0f * scale,
                    engine::Color{0, 0, 0, 0.35f}, engine::TextAlign::Center);
        font_->draw(batch, "TIME UP!", {world.x * 0.5f, world.y * 0.80f}, 96.0f * scale,
                    engine::Color::rgb8(255, 235, 59), engine::TextAlign::Center);
    }
}

}  // namespace app
