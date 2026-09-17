#pragma once

#include <memory>

#include "engine/Scene.h"
#include "engine/core/Timer.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/graphics/TextureAtlas.h"
#include "engine/input/TouchEvent.h"
#include "engine/math/Vec2.h"

namespace engine {

class AssetLoader;

// Platform-independent core: owns the renderer, the sprite atlas and the
// active scene, and maps screen pixels to a fixed-width world space.
//
// World space is kWorldWidth units wide; the height follows the screen's
// aspect ratio so nothing is stretched. Games read worldHeight() at runtime.
class Engine {
public:
    static constexpr float kWorldWidth = 720.0f;

    explicit Engine(AssetLoader& assets);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    // Must be called with a current GL context.
    bool initGraphics(int screenWidth, int screenHeight);
    void resize(int screenWidth, int screenHeight);
    bool graphicsReady() const { return graphicsReady_; }

    void setScene(std::unique_ptr<Scene> scene);

    // One update + render pass. Caller swaps buffers afterwards.
    void frame();

    // Call after a pause so the next frame's dt is not the paused duration.
    void resumeClock() { timer_.reset(); }

    // Screen shake: trauma accumulates (0..1) and decays; the visual offset is
    // trauma^2 so small hits barely move and big hits punch. Game positions
    // are never touched, only the projection.
    void addTrauma(float amount);

    // Hit-stop: freezes scene updates for `seconds` of real time.
    void hitStop(float seconds) { hitStopRemaining_ = seconds; }

    // Screen-space touch (pixels) -> world-space, forwarded to the scene.
    void onTouch(int32_t pointerId, TouchEvent::Phase phase, float screenX, float screenY);

    float worldWidth() const { return kWorldWidth; }
    float worldHeight() const { return worldHeight_; }
    Vec2 worldSize() const { return {kWorldWidth, worldHeight_}; }

    AssetLoader& assets() { return assets_; }
    TextureAtlas& atlas() { return *atlas_; }
    SpriteBatch& batch() { return *batch_; }
    float elapsed() const { return timer_.elapsed(); }

private:
    AssetLoader& assets_;
    std::unique_ptr<SpriteBatch> batch_;
    std::unique_ptr<TextureAtlas> atlas_;
    std::unique_ptr<Scene> scene_;
    Timer timer_;

    int screenWidth_ = 0;
    int screenHeight_ = 0;
    float worldHeight_ = 1280.0f;
    float screenToWorld_ = 1.0f;
    bool graphicsReady_ = false;

    float trauma_ = 0.0f;
    float shakeTime_ = 0.0f;
    float hitStopRemaining_ = 0.0f;
};

}  // namespace engine
