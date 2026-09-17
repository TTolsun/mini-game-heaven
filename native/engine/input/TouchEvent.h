#pragma once

#include <cstdint>

#include "engine/math/Vec2.h"

namespace engine {

// Platform-neutral touch event in world coordinates.
struct TouchEvent {
    enum class Phase { Down, Move, Up, Cancel };

    int32_t pointerId = 0;
    Phase phase = Phase::Down;
    Vec2 position;
};

}  // namespace engine
