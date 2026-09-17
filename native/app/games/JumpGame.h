#pragma once

#include <deque>
#include <random>
#include <vector>

#include "app/IMiniGame.h"
#include "engine/graphics/Animation.h"
#include "engine/graphics/Font.h"
#include "engine/graphics/Particles.h"
#include "engine/graphics/Sprite.h"
#include "engine/math/Vec2.h"

namespace app {

class GameAssets;

// Endless runner: the ground scrolls left, tap to jump over gaps and jellies.
// One tap in the air gives a second jump. Score is distance in metres.
class JumpGame final : public IMiniGame {
public:
    explicit JumpGame(const GameAssets& assets);

    void onEnter(engine::Engine& engine) override;
    void update(float dt) override;
    void render(engine::SpriteBatch& batch) override;
    void onTouch(const engine::TouchEvent& event) override;

    bool isFinished() const override { return finished_; }
    int score() const override { return score_; }

private:
    // One tile-wide slice of the level, generated ahead of the player.
    struct Column {
        float x;               // left edge in world units
        bool ground;           // false = water gap
        int obstacle;          // 0 none, 1 crate, 2 mushroom
        bool platform;         // floating platform above this column
        float platformY;
    };

    enum class DeathCause { None, Obstacle, Water };

    void generateAhead();
    void pushColumn();
    void jump();
    void land();
    void die(DeathCause cause);
    void updatePlayer(float dt);
    void updateWorld(float dt);
    void drawWorld(engine::SpriteBatch& batch);
    void drawHud(engine::SpriteBatch& batch);
    bool groundUnder(float x) const;
    float platformTopAt(float x, float yFrom, float yTo) const;

    const GameAssets& assets_;
    engine::Engine* engine_ = nullptr;
    const engine::Font* font_ = nullptr;

    engine::Sprite background_;
    std::vector<std::pair<float, int>> scenery_;  // x, sprite id (0-1 bush, 2-3 tree)
    engine::Animation runAnim_;
    engine::Animation jumpAnim_;
    engine::Animation deadAnim_;
    engine::Particles particles_;

    std::deque<Column> columns_;
    std::mt19937 rng_;
    float scroll_ = 0.0f;       // total distance scrolled
    float speed_ = 0.0f;
    int runLeft_ = 0;           // columns remaining in the current ground run
    int gapLeft_ = 0;           // columns remaining in the current gap
    int sinceObstacle_ = 0;

    float groundY_ = 0.0f;
    engine::Vec2 dino_;         // feet position
    float velocityY_ = 0.0f;
    bool grounded_ = false;
    int jumpsLeft_ = 0;
    float coyote_ = 0.0f;       // seconds since leaving ground
    float jumpBuffer_ = 0.0f;   // seconds since a tap that could not jump yet
    engine::Vec2 squash_{1.0f, 1.0f};
    float squashTime_ = 1.0f;   // 0..1 progress of the current squash recovery
    engine::Vec2 squashFrom_{1.0f, 1.0f};

    int score_ = 0;
    DeathCause death_ = DeathCause::None;
    float deathTimer_ = 0.0f;
    bool finished_ = false;
};

}  // namespace app
