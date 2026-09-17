#define LOG_TAG "PlaygroundScene"
#include "app/PlaygroundScene.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "engine/Engine.h"
#include "engine/asset/Image.h"
#include "engine/core/Log.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/graphics/TextureAtlas.h"

namespace app {

namespace {

constexpr float kDinoScale = 0.6f;
constexpr float kDinoSpeed = 420.0f;   // world units per second
constexpr float kGravity = 2400.0f;
constexpr float kTileSize = 96.0f;
constexpr float kGroundHeight = 2 * kTileSize;

}  // namespace

void PlaygroundScene::onEnter(engine::Engine& engine) {
    engine_ = &engine;
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
    for (int i = 1; i <= 6; ++i) {
        char name[64];
        char path[128];
        std::snprintf(name, sizeof(name), "jelly_%d", i);
        std::snprintf(path, sizeof(path), "sprites/jelly/jelly_%d.png", i);
        atlas.add(assets, name, path);
        jellySprites_.push_back(atlas.get(name));
    }
    atlas.add(assets, "ground", "tiles/2.png");
    groundTile_ = atlas.get("ground");

    // The background is large and drawn once, so it gets its own texture.
    background_.create(engine::Image::load(assets, "bg/forest.png"));
    backgroundSprite_.texture = &background_;
    backgroundSprite_.width = static_cast<float>(background_.width());
    backgroundSprite_.height = static_cast<float>(background_.height());

    idle_ = engine::Animation::fromAtlas(atlas, "dino_idle", 10.0f);
    run_ = engine::Animation::fromAtlas(atlas, "dino_run", 14.0f);

    groundY_ = engine.worldHeight() - kGroundHeight;
    dinoPos_ = {engine.worldWidth() * 0.5f, groundY_};
    dinoTargetX_ = dinoPos_.x;

    LOGI("loaded: idle %zu frames, run %zu frames", static_cast<size_t>(idle_.duration() * 10),
         static_cast<size_t>(run_.duration() * 14));
}

void PlaygroundScene::update(float dt) {
    const float dx = dinoTargetX_ - dinoPos_.x;
    const bool moving = std::fabs(dx) > 4.0f;
    if (moving) {
        const float step = std::copysign(std::min(kDinoSpeed * dt, std::fabs(dx)), dx);
        dinoPos_.x += step;
        dinoFacingLeft_ = dx < 0.0f;
        run_.update(dt);
    } else {
        idle_.update(dt);
    }

    for (Jelly& jelly : jellies_) {
        jelly.scale = std::min(1.0f, jelly.scale + dt * 6.0f);
        jelly.velocityY += kGravity * dt;
        jelly.position.y += jelly.velocityY * dt;

        const float restY = groundY_ - jelly.sprite.height * 0.5f * 0.7f;
        if (jelly.position.y > restY) {
            jelly.position.y = restY;
            jelly.velocityY = -jelly.velocityY * 0.45f;  // bounce with damping
            if (std::fabs(jelly.velocityY) < 60.0f) {
                jelly.velocityY = 0.0f;
            }
        }
    }
}

void PlaygroundScene::render(engine::SpriteBatch& batch) {
    const engine::Vec2 world = engine_->worldSize();

    // Cover the screen with the background, preserving aspect ratio.
    const float bgScale = std::max(world.x / backgroundSprite_.width, world.y / backgroundSprite_.height);
    batch.draw(backgroundSprite_, world * 0.5f, bgScale);

    // Two rows of ground tiles.
    const int columns = static_cast<int>(std::ceil(world.x / kTileSize));
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < columns; ++col) {
            const engine::Vec2 center{col * kTileSize + kTileSize * 0.5f,
                                      groundY_ + row * kTileSize + kTileSize * 0.5f};
            batch.draw(groundTile_, center, {kTileSize, kTileSize});
        }
    }

    for (const Jelly& jelly : jellies_) {
        batch.draw(jelly.sprite, jelly.position, 0.7f * jelly.scale);
    }

    const bool moving = std::fabs(dinoTargetX_ - dinoPos_.x) > 4.0f;
    const engine::Sprite& frame = moving ? run_.frame() : idle_.frame();
    const engine::Vec2 size = frame.size() * kDinoScale;
    const engine::Vec2 center{dinoPos_.x, groundY_ - size.y * 0.5f + 6.0f};
    batch.draw(frame, center, size, engine::Color::white(), 0.0f, dinoFacingLeft_);
}

void PlaygroundScene::onTouch(const engine::TouchEvent& event) {
    if (event.phase != engine::TouchEvent::Phase::Down) {
        return;
    }
    dinoTargetX_ = std::clamp(event.position.x, 60.0f, engine_->worldWidth() - 60.0f);

    if (!jellySprites_.empty()) {
        Jelly jelly;
        jelly.sprite = jellySprites_[jellies_.size() % jellySprites_.size()];
        jelly.position = {event.position.x, std::min(event.position.y, groundY_ - 80.0f)};
        jellies_.push_back(jelly);
        if (jellies_.size() > 12) {
            jellies_.erase(jellies_.begin());
        }
    }
}

}  // namespace app
