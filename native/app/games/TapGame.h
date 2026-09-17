#pragma once

#include <random>

#include "app/IMiniGame.h"
#include "app/ui/Popups.h"
#include "engine/graphics/Font.h"
#include "engine/graphics/Particles.h"
#include "engine/graphics/Sprite.h"
#include "engine/math/Vec2.h"

namespace app {

class GameAssets;

// Ten seconds on the clock: smash the crate as many times as you can.
// Every tap dents it; every tenth tap breaks it and a fresh one drops in.
class TapGame final : public IMiniGame {
public:
    explicit TapGame(const GameAssets& assets);

    void onEnter(engine::Engine& engine) override;
    void update(float dt) override;
    void render(engine::SpriteBatch& batch) override;
    void onTouch(const engine::TouchEvent& event) override;

    bool isFinished() const override { return finished_; }
    int score() const override { return taps_; }

private:
    static constexpr float kDuration = 10.0f;
    static constexpr int kHitsToBreak = 10;

    void tap(engine::Vec2 where);
    void breakCrate();
    void endRound();

    const GameAssets& assets_;
    engine::Engine* engine_ = nullptr;
    const engine::Font* font_ = nullptr;
    engine::Sprite background_;
    engine::Particles particles_;
    ui::Popups popups_;
    std::mt19937 rng_;

    engine::Vec2 cratePos_;    // resting centre
    float crateDrop_ = 0.0f;   // > 0 while a new crate is falling in (height above rest)
    float crateDropVel_ = 0.0f;
    float popTime_ = 1.0f;     // 0..1 progress of the squash recovery
    float tilt_ = 0.0f;        // radians, decays
    float time_ = 0.0f;
    int hitsOnCrate_ = 0;

    bool started_ = false;
    float timeLeft_ = kDuration;
    int lastTickSecond_ = 99;
    int taps_ = 0;
    float lastTapAge_ = 10.0f;
    int combo_ = 0;

    float endTimer_ = 0.0f;
    bool finished_ = false;
};

}  // namespace app
