#define LOG_TAG "TapGame"
#include "app/games/TapGame.h"

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

constexpr float kCrateSize = 300.0f;
constexpr float kComboWindow = 0.45f;  // seconds between taps to keep a combo alive
constexpr float kEndHold = 1.4f;
constexpr float kDropGravity = 6000.0f;

}  // namespace

TapGame::TapGame(const GameAssets& assets)
    : assets_(assets), particles_(std::random_device{}()), rng_(std::random_device{}()) {}

void TapGame::onEnter(engine::Engine& engine) {
    engine_ = &engine;
    font_ = &assets_.font();
    background_ = assets_.forestBackground();
    cratePos_ = {engine.worldWidth() * 0.5f, engine.worldHeight() * 0.55f};
}

void TapGame::onTouch(const engine::TouchEvent& event) {
    if (event.phase != engine::TouchEvent::Phase::Down || timeLeft_ <= 0.0f) {
        return;
    }
    // No crate to hit while the replacement is still falling in.
    if (crateDrop_ > 0.0f) {
        return;
    }
    started_ = true;
    tap(event.position);
}

void TapGame::tap(engine::Vec2 where) {
    ++taps_;
    ++hitsOnCrate_;
    combo_ = lastTapAge_ <= kComboWindow ? combo_ + 1 : 1;
    lastTapAge_ = 0.0f;
    popTime_ = 0.0f;
    // Tilt away from the finger.
    tilt_ = (where.x < cratePos_.x ? 1.0f : -1.0f) * 0.12f;

    engine_->haptics().light();
    engine_->mixer().play(assets_.sfx().tap, 0.9f, 0.95f + 0.1f * (combo_ % 3));

    popups_.add("+1", {where.x * 0.3f + cratePos_.x * 0.7f, cratePos_.y - kCrateSize * 0.6f},
                engine::Color::rgb8(255, 241, 118), 60.0f);

    // Wood chips fly off the crate.
    engine::Particles::Burst chips;
    chips.count = 4 + std::min(combo_, 8);
    chips.speedMin = 250.0f;
    chips.speedMax = 650.0f;
    chips.lifeMin = 0.25f;
    chips.lifeMax = 0.5f;
    chips.sizeMin = 8.0f;
    chips.sizeMax = 16.0f;
    chips.gravity = 1400.0f;
    chips.color = engine::Color::rgb8(198, 132, 74);
    chips.colorAlt = engine::Color::rgb8(120, 72, 36);
    particles_.emit(cratePos_, chips);

    if (hitsOnCrate_ >= kHitsToBreak) {
        breakCrate();
    }
}

void TapGame::breakCrate() {
    hitsOnCrate_ = 0;
    engine_->hitStop(0.06f);
    engine_->addTrauma(0.45f);
    engine_->haptics().medium();
    engine_->mixer().play(assets_.sfx().crack);
    popups_.add("SMASH!", {cratePos_.x, cratePos_.y - kCrateSize * 0.9f}, engine::Color::rgb8(255, 112, 67), 80.0f);

    engine::Particles::Burst planks;
    planks.count = 28;
    planks.speedMin = 400.0f;
    planks.speedMax = 1000.0f;
    planks.lifeMin = 0.5f;
    planks.lifeMax = 0.9f;
    planks.sizeMin = 14.0f;
    planks.sizeMax = 30.0f;
    planks.gravity = 1600.0f;
    planks.color = engine::Color::rgb8(210, 150, 90);
    planks.colorAlt = engine::Color::rgb8(110, 64, 30);
    particles_.emit(cratePos_, planks);

    // Next crate drops in from above.
    crateDrop_ = engine_->worldHeight() * 0.6f;
    crateDropVel_ = 0.0f;
}

void TapGame::endRound() {
    timeLeft_ = 0.0f;
    engine_->hitStop(0.1f);
    engine_->addTrauma(0.5f);
    engine_->haptics().heavy();
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
    particles_.emit({cratePos_.x, cratePos_.y + 60.0f}, confetti);
}

void TapGame::update(float dt) {
    if (finished_) {
        return;
    }
    particles_.update(dt);
    popups_.update(dt);
    time_ += dt;
    lastTapAge_ += dt;
    popTime_ = std::min(1.0f, popTime_ + dt / 0.2f);
    tilt_ *= std::max(0.0f, 1.0f - dt * 9.0f);
    if (lastTapAge_ > kComboWindow) {
        combo_ = 0;
    }

    if (crateDrop_ > 0.0f) {
        crateDropVel_ += kDropGravity * dt;
        crateDrop_ -= crateDropVel_ * dt;
        if (crateDrop_ <= 0.0f) {
            crateDrop_ = 0.0f;
            popTime_ = 0.0f;  // landing squash
            engine_->mixer().play(assets_.sfx().thud, 0.8f);
            engine_->addTrauma(0.2f);
        }
    }

    if (!started_) {
        return;
    }
    if (timeLeft_ > 0.0f) {
        timeLeft_ -= dt;
        const int second = static_cast<int>(std::ceil(timeLeft_));
        if (second <= 3 && second != lastTickSecond_ && timeLeft_ > 0.0f) {
            lastTickSecond_ = second;
            engine_->mixer().play(assets_.sfx().tick);
        }
        if (timeLeft_ <= 0.0f) {
            endRound();
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
    const float top = engine_->safeTop() + 16.0f;
    const float bgScale = std::max(world.x / background_.width, world.y / background_.height);
    batch.draw(background_, world * 0.5f, bgScale);

    // Crate: squash on tap (wide and flat), overshoot back; slight idle breathing.
    const float e = engine::ease::outBack(popTime_, 2.2f);
    const float idle = std::sin(time_ * 3.0f) * 0.015f;
    const engine::Vec2 squash{1.25f + (1.0f - 1.25f) * e + idle, 0.75f + (1.0f - 0.75f) * e - idle};
    const engine::Sprite crate = assets_.crate();
    const engine::Vec2 size{kCrateSize * squash.x, kCrateSize * squash.y};
    const float bottom = cratePos_.y + kCrateSize * 0.5f;
    // Shadow shrinks while the replacement crate is still in the air.
    const float shadowScale = std::clamp(1.0f - crateDrop_ / world.y, 0.3f, 1.0f);
    batch.drawQuad({cratePos_.x, bottom + 12.0f}, {size.x * 0.95f * shadowScale, 34.0f * shadowScale},
                   engine::Color{0, 0, 0, 0.18f});
    batch.draw(crate, {cratePos_.x, bottom - size.y * 0.5f - crateDrop_}, size, engine::Color::white(), tilt_);

    // Cracks: the crate darkens as it takes hits.
    const float damage = static_cast<float>(hitsOnCrate_) / kHitsToBreak;
    if (damage > 0.0f && crateDrop_ <= 0.0f) {
        batch.draw(crate, {cratePos_.x, bottom - size.y * 0.5f}, size, engine::Color{0, 0, 0, 0.35f * damage},
                   tilt_);
    }

    particles_.render(batch);
    popups_.render(batch, *font_);

    // HUD.
    char text[32];
    std::snprintf(text, sizeof(text), "%.1f", timeLeft_);
    const engine::Color timeColor = timeLeft_ <= 3.0f && timeLeft_ > 0.0f ? engine::Color::rgb8(255, 112, 67)
                                                                           : engine::Color::white();
    ui::shadowText(batch, *font_, text, {world.x * 0.5f, top}, 84.0f, timeColor, engine::TextAlign::Center);

    std::snprintf(text, sizeof(text), "%d", taps_);
    ui::shadowText(batch, *font_, text, {world.x * 0.5f, world.y * 0.20f}, 140.0f, engine::Color::white(),
                   engine::TextAlign::Center);
    ui::shadowText(batch, *font_, "HITS", {world.x * 0.5f, world.y * 0.20f + 140.0f}, ui::kBody,
                   engine::Color::white().withAlpha(0.8f), engine::TextAlign::Center);

    if (combo_ >= 3 && timeLeft_ > 0.0f) {
        std::snprintf(text, sizeof(text), "%d COMBO", combo_);
        const float pulse = 1.0f + 0.15f * engine::ease::pulse(std::min(1.0f, lastTapAge_ / 0.2f));
        ui::shadowText(batch, *font_, text, {world.x * 0.5f, world.y * 0.80f}, 64.0f * pulse,
                       engine::Color::rgb8(255, 138, 101), engine::TextAlign::Center);
    }

    if (!started_) {
        ui::shadowText(batch, *font_, "SMASH THE CRATE!", {world.x * 0.5f, world.y * 0.80f}, ui::kTitle,
                       engine::Color::white().withAlpha(0.7f + 0.3f * std::sin(time_ * 6.0f)),
                       engine::TextAlign::Center);
    } else if (timeLeft_ <= 0.0f) {
        const float t = std::min(1.0f, endTimer_ / 0.3f);
        const float scale = engine::ease::outBack(t, 2.5f);
        ui::shadowText(batch, *font_, "TIME UP!", {world.x * 0.5f, world.y * 0.80f}, 96.0f * scale,
                       engine::Color::rgb8(255, 235, 59), engine::TextAlign::Center);
    }
}

}  // namespace app
