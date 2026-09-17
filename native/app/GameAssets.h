#pragma once

#include <string>

#include "engine/graphics/Animation.h"
#include "engine/graphics/Font.h"
#include "engine/graphics/Sprite.h"
#include "engine/graphics/Texture.h"

namespace engine {
class Engine;
}

namespace app {

// Everything the menu and the mini games share, loaded once at startup.
// Games take a const reference and copy the Sprites/Animations they need.
class GameAssets {
public:
    bool load(engine::Engine& engine);

    engine::Animation dinoIdle() const { return dinoIdle_; }
    engine::Animation dinoRun() const { return dinoRun_; }
    engine::Animation dinoJump() const { return dinoJump_; }
    engine::Animation dinoDead() const { return dinoDead_; }

    // index 1..6
    engine::Sprite jelly(int index) const;
    engine::Sprite groundTile() const { return groundTile_; }
    engine::Sprite forestBackground() const { return forest_; }

    const engine::Font& font() const { return font_; }

private:
    engine::Texture forestTexture_;
    engine::Sprite forest_;
    engine::Sprite groundTile_;
    engine::Sprite jellies_[6];
    engine::Animation dinoIdle_;
    engine::Animation dinoRun_;
    engine::Animation dinoJump_;
    engine::Animation dinoDead_;
    engine::Font font_;
};

}  // namespace app
