#pragma once

#include <memory>
#include <string>

#include "engine/Scene.h"
#include "engine/audio/Mixer.h"
#include "engine/core/Timer.h"
#include "engine/graphics/SpriteBatch.h"
#include "engine/graphics/TextureAtlas.h"
#include "engine/input/TouchEvent.h"
#include "engine/math/Vec2.h"
#include "engine/platform/Haptics.h"

namespace engine {

class AssetLoader;

// Platform-independent core: owns the renderer, the sprite atlas, the audio
// mixer and the active scene, and maps screen pixels to a fixed-width world.
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

    // Platform services. Optional; defaults are no-ops.
    void setHaptics(Haptics* haptics) { haptics_ = haptics != nullptr ? haptics : &nullHaptics_; }
    void setDataPath(std::string path) { dataPath_ = std::move(path); }

    // Must be called with a current GL context.
    bool initGraphics(int screenWidth, int screenHeight);
    void resize(int screenWidth, int screenHeight);
    bool graphicsReady() const { return graphicsReady_; }

    // Display cutout / system bar insets in screen pixels; HUD avoids them.
    void setSafeInsets(int top, int bottom);

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

    // Returns true if the scene consumed the back press.
    bool onBack();

    float worldWidth() const { return kWorldWidth; }
    float worldHeight() const { return worldHeight_; }
    Vec2 worldSize() const { return {kWorldWidth, worldHeight_}; }
    float safeTop() const { return safeTop_; }        // world units
    float safeBottom() const { return safeBottom_; }  // world units

    AssetLoader& assets() { return assets_; }
    TextureAtlas& atlas() { return *atlas_; }
    SpriteBatch& batch() { return *batch_; }
    Mixer& mixer() { return mixer_; }
    Haptics& haptics() { return *haptics_; }
    const std::string& dataPath() const { return dataPath_; }
    float elapsed() const { return timer_.elapsed(); }

private:
    AssetLoader& assets_;
    std::unique_ptr<SpriteBatch> batch_;
    std::unique_ptr<TextureAtlas> atlas_;
    std::unique_ptr<Scene> scene_;
    Timer timer_;
    Mixer mixer_;
    NullHaptics nullHaptics_;
    Haptics* haptics_ = &nullHaptics_;
    std::string dataPath_;

    int screenWidth_ = 0;
    int screenHeight_ = 0;
    float worldHeight_ = 1280.0f;
    float screenToWorld_ = 1.0f;
    float safeTop_ = 0.0f;
    float safeBottom_ = 0.0f;
    int insetTopPx_ = 0;
    int insetBottomPx_ = 0;
    bool graphicsReady_ = false;

    float trauma_ = 0.0f;
    float shakeTime_ = 0.0f;
    float hitStopRemaining_ = 0.0f;
};

}  // namespace engine
