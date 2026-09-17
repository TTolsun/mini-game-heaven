#pragma once

#include <string>

#include "app/Sfx.h"
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

    // Character
    engine::Animation dinoIdle() const { return dinoIdle_; }
    engine::Animation dinoRun() const { return dinoRun_; }
    engine::Animation dinoJump() const { return dinoJump_; }
    engine::Animation dinoDead() const { return dinoDead_; }

    // Tiles
    engine::Sprite groundTile() const { return groundTile_; }  // grass top
    engine::Sprite dirtTile() const { return dirtTile_; }      // filler below grass
    engine::Sprite platformLeft() const { return platformLeft_; }
    engine::Sprite platformMid() const { return platformMid_; }
    engine::Sprite platformRight() const { return platformRight_; }
    engine::Sprite waterTop() const { return waterTop_; }
    engine::Sprite water() const { return water_; }
    engine::Sprite forestBackground() const { return forest_; }

    // Objects (same tileset as the ground, so they match the dino's style)
    engine::Sprite crate() const { return crate_; }
    engine::Sprite stone() const { return stone_; }
    engine::Sprite mushroomPink() const { return mushroomPink_; }
    engine::Sprite mushroomOrange() const { return mushroomOrange_; }
    engine::Sprite bush(int index) const { return index == 0 ? bush1_ : bush2_; }
    engine::Sprite tree(int index) const { return index == 0 ? tree1_ : tree2_; }
    engine::Sprite stump() const { return stump_; }

    // GUI
    engine::Sprite panel() const { return panel_; }
    engine::Sprite ribbon() const { return ribbon_; }
    engine::Sprite star() const { return star_; }
    engine::Sprite wideButton(int color) const;  // 0 blue, 1 green, 2 red, 3 gray
    engine::Sprite iconHome() const { return iconHome_; }
    engine::Sprite iconRetry() const { return iconRetry_; }
    engine::Sprite iconPlay() const { return iconPlay_; }
    engine::Sprite iconClose() const { return iconClose_; }
    engine::Sprite iconSoundOn() const { return iconSoundOn_; }
    engine::Sprite iconSoundOff() const { return iconSoundOff_; }

    const engine::Font& font() const { return font_; }
    const Sfx& sfx() const { return sfx_; }

private:
    engine::Texture forestTexture_;
    engine::Sprite forest_;

    engine::Sprite groundTile_, dirtTile_, platformLeft_, platformMid_, platformRight_, waterTop_, water_;
    engine::Sprite crate_, stone_, mushroomPink_, mushroomOrange_, bush1_, bush2_, tree1_, tree2_, stump_;
    engine::Sprite panel_, ribbon_, star_;
    engine::Sprite wideButtons_[4];
    engine::Sprite iconHome_, iconRetry_, iconPlay_, iconClose_, iconSoundOn_, iconSoundOff_;

    engine::Animation dinoIdle_, dinoRun_, dinoJump_, dinoDead_;
    engine::Font font_;
    Sfx sfx_;
};

}  // namespace app
