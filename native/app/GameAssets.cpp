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

    for (int i = 1; i <= 6; ++i) {
        char name[64];
        char path[128];
        std::snprintf(name, sizeof(name), "jelly_%d", i);
        std::snprintf(path, sizeof(path), "sprites/jelly/jelly_%d.png", i);
        atlas.add(assets, name, path);
        jellies_[i - 1] = atlas.get(name);
    }

    // Tileset numbering: 2 grass top, 5 dirt, 13-15 floating platform L/M/R, 17-18 water.
    struct TileFile {
        const char* name;
        const char* file;
        engine::Sprite* out;
    };
    for (const TileFile& tile : {TileFile{"ground", "tiles/2.png", &groundTile_},
                                 TileFile{"dirt", "tiles/5.png", &dirtTile_},
                                 TileFile{"plat_l", "tiles/13.png", &platformLeft_},
                                 TileFile{"plat_m", "tiles/14.png", &platformMid_},
                                 TileFile{"plat_r", "tiles/15.png", &platformRight_},
                                 TileFile{"water_top", "tiles/17.png", &waterTop_},
                                 TileFile{"water", "tiles/18.png", &water_}}) {
        atlas.add(assets, tile.name, tile.file);
        *tile.out = atlas.get(tile.name);
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

    LOGI("loaded (idle %d, run %d, jump %d, dead %d frames)", dinoIdle_.frameCount(),
         dinoRun_.frameCount(), dinoJump_.frameCount(), dinoDead_.frameCount());
    return !dinoIdle_.empty() && forest_.isValid();
}

engine::Sprite GameAssets::jelly(int index) const {
    if (index < 1 || index > 6) {
        return {};
    }
    return jellies_[index - 1];
}

}  // namespace app
