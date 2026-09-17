#pragma once

#include <random>
#include <vector>

#include "engine/graphics/Sprite.h"
#include "engine/math/Color.h"
#include "engine/math/Vec2.h"

namespace engine {

class SpriteBatch;

// Lightweight CPU particle pool: bursts of squares (or a sprite) with gravity,
// drag, spin and fade. Enough for dust, confetti and hit sparks.
class Particles {
public:
    struct Burst {
        int count = 8;
        float speedMin = 200.0f;
        float speedMax = 500.0f;
        float angleMin = 0.0f;       // radians, 0 = right, pi/2 = down (screen space)
        float angleMax = 6.2831853f;
        float lifeMin = 0.3f;
        float lifeMax = 0.6f;
        float sizeMin = 8.0f;
        float sizeMax = 16.0f;
        float gravity = 1200.0f;
        float drag = 1.5f;           // per second
        Color color = Color::white();
        Color colorAlt = Color::white();  // each particle lerps between color and colorAlt
        Sprite sprite;               // invalid = solid square
    };

    explicit Particles(unsigned seed = 1234u) : rng_(seed) {}

    void emit(Vec2 position, const Burst& burst);
    void update(float dt);
    void render(SpriteBatch& batch) const;
    void clear() { particles_.clear(); }
    size_t count() const { return particles_.size(); }

private:
    struct Particle {
        Vec2 position;
        Vec2 velocity;
        float life;
        float maxLife;
        float size;
        float angle;
        float spin;
        float gravity;
        float drag;
        Color color;
        Sprite sprite;
    };

    std::vector<Particle> particles_;
    std::mt19937 rng_;
};

}  // namespace engine
