#pragma once

#include "engine/Scene.h"

namespace app {

class GameAssets;

// Contract every mini game implements. The app drives it like any Scene and
// polls isFinished() each frame to move on to the result screen.
class IMiniGame : public engine::Scene {
public:
    ~IMiniGame() override = default;

    virtual bool isFinished() const = 0;
    virtual int score() const = 0;
};

}  // namespace app
