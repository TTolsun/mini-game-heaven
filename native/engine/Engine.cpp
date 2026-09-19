#define LOG_TAG "Engine"
#include "engine/Engine.h"

#include <GLES3/gl3.h>

#include <algorithm>
#include <cmath>

#include "engine/core/Log.h"
#include "engine/graphics/Viewport.h"

namespace engine {

Engine::Engine(AssetLoader& assets) : assets_(assets) {}

Engine::~Engine() {
    if (scene_) {
        scene_->onExit();
    }
    if(framebuffer_!=0) glDeleteFramebuffers(1,&framebuffer_);
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

    if(framebuffer_!=0) glDeleteFramebuffers(1,&framebuffer_);
    if(!renderTexture_.create(1280,720)) return false;
    renderTexture_.setNearest();
    glGenFramebuffers(1,&framebuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER,framebuffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,renderTexture_.id(),0);
    const bool complete=glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    if(!complete) { LOGE("render target incomplete"); return false; }

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
    viewport_=fitViewport(screenWidth,screenHeight,insetLeftPx_,insetTopPx_,insetRightPx_,insetBottomPx_);
}

void Engine::setSafeInsets(int top, int bottom, int left, int right) {
    insetTopPx_=std::max(0,top); insetBottomPx_=std::max(0,bottom);
    insetLeftPx_=std::max(0,left); insetRightPx_=std::max(0,right);
    resize(screenWidth_,screenHeight_);
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

    glBindFramebuffer(GL_FRAMEBUFFER,framebuffer_);
    glViewport(0,0,1280,720);
    glClearColor(0.08f, 0.09f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    batch_->begin(kWorldWidth, worldHeight_, shake);
    if (scene_) {
        scene_->render(*batch_);
    }
    batch_->end();
    glBindFramebuffer(GL_READ_FRAMEBUFFER,framebuffer_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
    glClearColor(0.025f,0.035f,0.04f,1);
    glClear(GL_COLOR_BUFFER_BIT);
    const int x=static_cast<int>(viewport_.x), y=screenHeight_-static_cast<int>(viewport_.y+viewport_.h);
    glBlitFramebuffer(0,0,1280,720,x,y,x+static_cast<int>(viewport_.w),y+static_cast<int>(viewport_.h),GL_COLOR_BUFFER_BIT,GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void Engine::onTouch(int32_t pointerId, TouchEvent::Phase phase, float screenX, float screenY) {
    if (!scene_) {
        return;
    }
    TouchEvent event;
    event.pointerId = pointerId;
    event.phase = viewport_.contains({screenX,screenY})?phase:TouchEvent::Phase::Cancel;
    event.position = viewportPoint(viewport_,{screenX,screenY});
    scene_->onTouch(event);
}

bool Engine::onBack() {
    return scene_ ? scene_->onBack() : false;
}

}  // namespace engine
