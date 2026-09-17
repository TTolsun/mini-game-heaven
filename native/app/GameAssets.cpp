#define LOG_TAG "GameAssets"
#include "app/GameAssets.h"

#include <cstdio>

#include "engine/Engine.h"
#include "engine/asset/Image.h"
#include "engine/core/Log.h"
#include "engine/graphics/TextureAtlas.h"

namespace app {

bool GameAssets::load(engine::Engine& engine) {
    engine::TextureAtlas& atlas = engine.atlas();
    engine::AssetLoader& assets = engine.assets();

    // Character frames: dino_<anim>_NN in the atlas, files are sprites/dino/<anim>_NN.png.
    struct AnimFiles {
        const char* name;
        int frames;
    };
    for (const AnimFiles& anim : {AnimFiles{"idle", 10}, AnimFiles{"run", 8},
                                  AnimFiles{"jump", 12}, AnimFiles{"dead", 8}}) {
        for (int i = 1; i <= anim.frames; ++i) {
            char name[64];
            char path[128];
            std::snprintf(name, sizeof(name), "dino_%s_%02d", anim.name, i);
            std::snprintf(path, sizeof(path), "sprites/dino/%s_%02d.png", anim.name, i);
            atlas.add(assets, name, path);
        }
    }
    dinoIdle_ = engine::Animation::fromAtlas(atlas, "dino_idle", 10.0f);
    dinoRun_ = engine::Animation::fromAtlas(atlas, "dino_run", 14.0f);
    dinoJump_ = engine::Animation::fromAtlas(atlas, "dino_jump", 16.0f, false);
    dinoDead_ = engine::Animation::fromAtlas(atlas, "dino_dead", 12.0f, false);

    // Single sprites: atlas name, file, destination.
    struct File {
        const char* name;
        const char* path;
        engine::Sprite* out;
    };
    const File files[] = {
        // Tileset numbering: 2 grass top, 5 dirt, 13-15 floating platform L/M/R, 17-18 water.
        {"ground", "tiles/2.png", &groundTile_},
        {"dirt", "tiles/5.png", &dirtTile_},
        {"plat_l", "tiles/13.png", &platformLeft_},
        {"plat_m", "tiles/14.png", &platformMid_},
        {"plat_r", "tiles/15.png", &platformRight_},
        {"water_top", "tiles/17.png", &waterTop_},
        {"water", "tiles/18.png", &water_},
        {"crate", "objects/crate.png", &crate_},
        {"stone", "objects/stone.png", &stone_},
        {"mushroom_pink", "objects/mushroom_pink.png", &mushroomPink_},
        {"mushroom_orange", "objects/mushroom_orange.png", &mushroomOrange_},
        {"bush_1", "objects/bush_1.png", &bush1_},
        {"bush_2", "objects/bush_2.png", &bush2_},
        {"tree_1", "objects/tree_1.png", &tree1_},
        {"tree_2", "objects/tree_2.png", &tree2_},
        {"stump", "objects/stump.png", &stump_},
        {"panel", "gui/panel.png", &panel_},
        {"ribbon", "gui/ribbon.png", &ribbon_},
        {"star", "gui/star.png", &star_},
        {"btn_wide_blue", "gui/btn_wide_blue.png", &wideButtons_[0]},
        {"btn_wide_green", "gui/btn_wide_green.png", &wideButtons_[1]},
        {"btn_wide_red", "gui/btn_wide_red.png", &wideButtons_[2]},
        {"btn_wide_gray", "gui/btn_wide_gray.png", &wideButtons_[3]},
        {"btn_home", "gui/btn_home.png", &iconHome_},
        {"btn_retry", "gui/btn_retry.png", &iconRetry_},
        {"btn_play", "gui/btn_play.png", &iconPlay_},
        {"btn_close", "gui/btn_close.png", &iconClose_},
        {"btn_sound_on", "gui/btn_sound_on.png", &iconSoundOn_},
        {"btn_sound_off", "gui/btn_sound_off.png", &iconSoundOff_},
    };
    for (const File& f : files) {
        atlas.add(assets, f.name, f.path);
        *f.out = atlas.get(f.name);
    }

    // The background is large and drawn once per frame, so it gets its own texture.
    forestTexture_.create(engine::Image::load(assets, "bg/forest.png"));
    forest_.texture = &forestTexture_;
    forest_.width = static_cast<float>(forestTexture_.width());
    forest_.height = static_cast<float>(forestTexture_.height());

    if (!font_.load(assets, atlas, "font_ui", "fonts/Fredoka-SemiBold.ttf")) {
        LOGE("font load failed");
        return false;
    }

    sfx_.build();

    LOGI("loaded (idle %d, run %d, jump %d, dead %d frames)", dinoIdle_.frameCount(),
         dinoRun_.frameCount(), dinoJump_.frameCount(), dinoDead_.frameCount());
    return !dinoIdle_.empty() && forest_.isValid() && panel_.isValid();
}

engine::Sprite GameAssets::wideButton(int color) const {
    if (color < 0 || color > 3) {
        return wideButtons_[3];
    }
    return wideButtons_[color];
}

}  // namespace app
