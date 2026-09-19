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
    engine::Sound tap;       // Martial strike
    engine::Sound fanfare;   // Insight or mastery
};

}  // namespace app
