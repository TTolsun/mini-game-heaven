#pragma once

#include "engine/audio/Sound.h"

namespace engine {
class Mixer;
}

namespace app {

// Every sound effect in the game, synthesised at startup (no audio files).
class Sfx {
public:
    void build();

    engine::Sound click;     // UI button
    engine::Sound tap;       // Tap game hit
    engine::Sound jump;
    engine::Sound doubleJump;
    engine::Sound land;
    engine::Sound hit;       // player got hit
    engine::Sound splash;
    engine::Sound ding;      // near miss / bonus
    engine::Sound crack;     // crate breaking
    engine::Sound thud;      // stone landing
    engine::Sound tick;      // countdown last seconds
    engine::Sound fanfare;   // time up / result
    engine::Sound newBest;
};

}  // namespace app
