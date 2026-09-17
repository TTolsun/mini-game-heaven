#pragma once

#include <random>
#include <vector>

#include "app/IMiniGame.h"
#include "app/ui/Popups.h"
#include "engine/graphics/Animation.h"
#include "engine/graphics/Font.h"
#include "engine/graphics/Particles.h"
#include "engine/graphics/Sprite.h"
#include "engine/math/Vec2.h"

namespace app {

class GameAssets;

// Stones and crates rain from the sky; hold the left or right half of the
// screen to run. Survive for points, and earn a bonus for every near miss.
class DodgeGame final : public IMiniGame {
public:
    explicit DodgeGame(const GameAssets& assets);

    void onEnter(engine::Engine& engine) override;
    void update(float dt) override;
    void render(engine::SpriteBatch& batch) override;
    void onTouch(const engine::TouchEvent& event) override;

    bool isFinished() const override { return finished_; }
    int score() const override { return score_; }

private:
    enum class Kind { Stone, Crate };

    struct Falling {
        Kind kind;
        engine::Sprite sprite;
        engine::Vec2 position;
        float speed = 0.0f;
        float spin = 0.0f;
        float angle = 0.0f;
        float landed = -1.0f;  // seconds since landing, < 0 while falling
        bool nearMissChecked = false;
    };

    void spawn();
    void land(Falling& object);
    void hit();
    void updateFalling(float dt);
    void updateDino(float dt);
    void drawHud(engine::SpriteBatch& batch);

    const GameAssets& assets_;
    engine::Engine* engine_ = nullptr;
    const engine::Font* font_ = nullptr;

    engine::Sprite background_;
    engine::Sprite groundTile_;
    engine::Sprite dirtTile_;
    engine::Animation idleAnim_;
    engine::Animation runAnim_;
    engine::Animation deadAnim_;
    engine::Particles particles_;
    ui::Popups popups_;

    engine::Vec2 dinoPos_;
    float groundY_ = 0.0f;
    int moveDir_ = 0;  // -1 left, 0 idle, +1 right
    bool facingLeft_ = false;
    int activePointer_ = -1;
    float dustTimer_ = 0.0f;
    engine::Vec2 squash_{1.0f, 1.0f};
    engine::Vec2 squashFrom_{1.0f, 1.0f};
    float squashTime_ = 1.0f;

    std::vector<Falling> falling_;
    std::mt19937 rng_;
    float spawnTimer_ = 0.0f;
    float elapsed_ = 0.0f;
    int dodged_ = 0;
    int bonus_ = 0;
    int score_ = 0;
    bool dead_ = false;
    bool finished_ = false;
    float deathTimer_ = 0.0f;
};

}  // namespace app
