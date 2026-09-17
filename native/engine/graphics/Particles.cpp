#include "engine/graphics/Particles.h"

#include <algorithm>
#include <cmath>

#include "engine/graphics/SpriteBatch.h"
#include "engine/math/Rect.h"

namespace engine {

void Particles::emit(Vec2 position, const Burst& burst) {
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);
    std::uniform_real_distribution<float> speed(burst.speedMin, burst.speedMax);
    std::uniform_real_distribution<float> angle(burst.angleMin, burst.angleMax);
    std::uniform_real_distribution<float> life(burst.lifeMin, burst.lifeMax);
    std::uniform_real_distribution<float> size(burst.sizeMin, burst.sizeMax);
    std::uniform_real_distribution<float> spin(-8.0f, 8.0f);

    for (int i = 0; i < burst.count; ++i) {
        const float a = angle(rng_);
        const float s = speed(rng_);
        const float mix = unit(rng_);
        Particle p;
        p.position = position;
        p.velocity = {std::cos(a) * s, std::sin(a) * s};
        p.maxLife = p.life = life(rng_);
        p.size = size(rng_);
        p.angle = angle(rng_);
        p.spin = spin(rng_);
        p.gravity = burst.gravity;
        p.drag = burst.drag;
        p.color = {burst.color.r + (burst.colorAlt.r - burst.color.r) * mix,
                   burst.color.g + (burst.colorAlt.g - burst.color.g) * mix,
                   burst.color.b + (burst.colorAlt.b - burst.color.b) * mix,
                   burst.color.a};
        p.sprite = burst.sprite;
        particles_.push_back(p);
    }
}

void Particles::update(float dt) {
    for (Particle& p : particles_) {
        p.life -= dt;
        p.velocity.y += p.gravity * dt;
        p.velocity *= std::max(0.0f, 1.0f - p.drag * dt);
        p.position += p.velocity * dt;
        p.angle += p.spin * dt;
    }
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(), [](const Particle& p) { return p.life <= 0.0f; }),
        particles_.end());
}

void Particles::render(SpriteBatch& batch) const {
    for (const Particle& p : particles_) {
        const float t = p.life / p.maxLife;   // 1 -> 0
        const float size = p.size * (0.4f + 0.6f * t);
        const Color color = p.color.withAlpha(p.color.a * std::min(1.0f, t * 2.0f));
        if (p.sprite.isValid()) {
            batch.draw(p.sprite, p.position, {size, size}, color, p.angle);
        } else {
            batch.drawQuad(p.position, {size, size}, color, p.angle);
        }
    }
}

}  // namespace engine
