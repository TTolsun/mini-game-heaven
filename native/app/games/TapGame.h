#pragma once

#include <random>
#include <vector>

#include "app/IMiniGame.h"
#include "engine/graphics/Font.h"
#include "engine/graphics/Particles.h"
#include "engine/graphics/Sprite.h"
#include "engine/math/Vec2.h"

namespace app {

class GameAssets;

// Ten seconds on the clock: tap the jelly as many times as you can.
// Every tap pops the jelly, floats a "+1" and sprays a few sparks.
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
    struct Popup {
        engine::Vec2 position;
        float age = 0.0f;
        int value = 1;
    };

    static constexpr float kDuration = 10.0f;

    void tap(engine::Vec2 where);

    const GameAssets& assets_;
    engine::Engine* engine_ = nullptr;
    const engine::Font* font_ = nullptr;
    engine::Sprite background_;
    engine::Particles particles_;
    std::mt19937 rng_;

    engine::Vec2 jellyPos_;
    int jellyIndex_ = 1;
    float popTime_ = 1.0f;     // 0..1 progress of the squash recovery
    float wobble_ = 0.0f;

    bool started_ = false;
    float timeLeft_ = kDuration;
    int taps_ = 0;
    float lastTapAge_ = 10.0f;
    int combo_ = 0;
    int bestCombo_ = 0;
    std::vector<Popup> popups_;

    float endTimer_ = 0.0f;
    bool finished_ = false;
};

}  // namespace app
