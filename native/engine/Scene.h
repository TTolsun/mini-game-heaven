#pragma once

#include "engine/input/TouchEvent.h"

namespace engine {

class Engine;
class SpriteBatch;

// One screen of the app (menu, a mini game, result). The engine drives
// exactly one scene at a time.
class Scene {
public:
    virtual ~Scene() = default;

    virtual void onEnter(Engine& /*engine*/) {}
    virtual void onExit() {}
    virtual void update(float dt) = 0;
    virtual void render(SpriteBatch& batch) = 0;
    virtual void onTouch(const TouchEvent& /*event*/) {}
};

}  // namespace engine
