#define LOG_TAG "Engine"
#include "engine/Engine.h"

#include <GLES3/gl3.h>

#include <algorithm>
#include <cmath>

#include "engine/core/Log.h"

namespace engine {

Engine::Engine(AssetLoader& assets) : assets_(assets) {}

Engine::~Engine() {
    if (scene_) {
        scene_->onExit();
    }
}

bool Engine::initGraphics(int screenWidth, int screenHeight) {
    batch_ = std::make_unique<SpriteBatch>();
    if (!batch_->init()) {
        LOGE("sprite batch init failed");
        return false;
    }

    // Use a 4096 atlas when the GPU allows it; 2048 is the GLES 3.0 minimum.
    GLint maxTexture = 2048;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexture);
    const int atlasSize = maxTexture >= 4096 ? 4096 : 2048;
    atlas_ = std::make_unique<TextureAtlas>(atlasSize);

    resize(screenWidth, screenHeight);
    graphicsReady_ = true;

    if (scene_) {
        scene_->onEnter(*this);
    }
    LOGI("graphics ready: screen %dx%d, world %.0fx%.0f, atlas %d", screenWidth, screenHeight,
         kWorldWidth, worldHeight_, atlasSize);
    return true;
}

void Engine::resize(int screenWidth, int screenHeight) {
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;
    screenToWorld_ = kWorldWidth / static_cast<float>(screenWidth);
    worldHeight_ = screenHeight * screenToWorld_;
    safeTop_ = insetTopPx_ * screenToWorld_;
    safeBottom_ = insetBottomPx_ * screenToWorld_;
    glViewport(0, 0, screenWidth, screenHeight);
}

void Engine::setSafeInsets(int top, int bottom) {
    insetTopPx_ = top;
    insetBottomPx_ = bottom;
    safeTop_ = top * screenToWorld_;
    safeBottom_ = bottom * screenToWorld_;
    LOGI("safe insets: top %d px, bottom %d px", top, bottom);
}

void Engine::setScene(std::unique_ptr<Scene> scene) {
    if (scene_) {
        scene_->onExit();
    }
    scene_ = std::move(scene);
    if (scene_ && graphicsReady_) {
        scene_->onEnter(*this);
    }
}

void Engine::addTrauma(float amount) {
    trauma_ = std::min(1.0f, trauma_ + amount);
}

void Engine::frame() {
    if (!graphicsReady_) {
        return;
    }
    const float realDt = timer_.tick();

    // Hit-stop consumes real time but hands the scene dt = 0.
    float dt = realDt;
    if (hitStopRemaining_ > 0.0f) {
        hitStopRemaining_ -= realDt;
        dt = 0.0f;
    }

    if (scene_) {
        scene_->update(dt);
    }

    // Shake decays in real time and is sampled from smooth sines, not per-frame noise.
    Vec2 shake;
    if (trauma_ > 0.0f) {
        constexpr float kDecayPerSecond = 1.6f;
        constexpr Vec2 kMaxOffset{18.0f, 12.0f};
        trauma_ = std::max(0.0f, trauma_ - kDecayPerSecond * realDt);
        shakeTime_ += realDt * 30.0f;
        const float amount = trauma_ * trauma_;
        shake = {kMaxOffset.x * amount * std::sin(shakeTime_ * 1.7f),
                 kMaxOffset.y * amount * std::sin(shakeTime_ * 2.3f)};
    }

    glClearColor(0.08f, 0.09f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    batch_->begin(kWorldWidth, worldHeight_, shake);
    if (scene_) {
        scene_->render(*batch_);
    }
    batch_->end();
}

void Engine::onTouch(int32_t pointerId, TouchEvent::Phase phase, float screenX, float screenY) {
    if (!scene_) {
        return;
    }
    TouchEvent event;
    event.pointerId = pointerId;
    event.phase = phase;
    event.position = {screenX * screenToWorld_, screenY * screenToWorld_};
    scene_->onTouch(event);
}

bool Engine::onBack() {
    return scene_ ? scene_->onBack() : false;
}

}  // namespace engine
