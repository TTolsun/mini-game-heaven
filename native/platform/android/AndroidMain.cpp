// Android entry point. Bridges GameActivity's native_app_glue to the engine.
// Everything Android-specific stays under platform/android/.

#define LOG_TAG "WutenAndroid"

#include <android/keycodes.h>
#include <game-activity/GameActivity.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

#include <algorithm>
#include <memory>

#include "app/srpg/BattleScene.h"
#include "engine/Engine.h"
#include "engine/core/Log.h"
#include "platform/android/AndroidAssetLoader.h"
#include "platform/android/AndroidAudio.h"
#include "platform/android/AndroidHaptics.h"
#include "platform/android/GlContext.h"

namespace {

using platform::android::AndroidAssetLoader;
using platform::android::AndroidAudio;
using platform::android::AndroidHaptics;
using platform::android::GlContext;

struct AppState {
    explicit AppState(android_app* app)
        : assets(app->activity->assetManager), engine(assets), haptics(app), audio(engine.mixer()) {
        engine.setHaptics(&haptics);
        engine.setDataPath(app->activity->internalDataPath);
    }

    AndroidAssetLoader assets;
    engine::Engine engine;
    AndroidHaptics haptics;
    AndroidAudio audio;
    GlContext gl;
    bool hasFocus = false;

    bool animating() const { return gl.hasSurface() && hasFocus; }
};

// HUD keeps clear of the camera cutout and the system bars.
void applyInsets(android_app* app, AppState& state) {
    ARect cutout{};
    ARect bars{};
    GameActivity_getWindowInsets(app->activity, GAMECOMMON_INSETS_TYPE_DISPLAY_CUTOUT, &cutout);
    GameActivity_getWindowInsets(app->activity, GAMECOMMON_INSETS_TYPE_SYSTEM_BARS, &bars);
    state.engine.setSafeInsets(std::max(cutout.top,bars.top),std::max(cutout.bottom,bars.bottom),
                              std::max(cutout.left,bars.left),std::max(cutout.right,bars.right));
}

void handleAppCmd(android_app* app, int32_t cmd) {
    auto* state = static_cast<AppState*>(app->userData);

    switch (cmd) {
    case APP_CMD_INIT_WINDOW:
        if (app->window != nullptr && state->gl.createSurface(app->window)) {
            if (!state->engine.graphicsReady()) {
                state->engine.initGraphics(state->gl.width(), state->gl.height());
                applyInsets(app, *state);
                state->engine.setScene(std::make_unique<app::srpg::BattleScene>());
            } else {
                state->engine.resize(state->gl.width(), state->gl.height());
            }
            state->engine.resumeClock();
        }
        break;
    case APP_CMD_TERM_WINDOW:
        state->gl.destroySurface();
        break;
    case APP_CMD_WINDOW_RESIZED:
    case APP_CMD_CONFIG_CHANGED:
        if (state->gl.hasSurface() && app->window != nullptr) {
            state->gl.createSurface(app->window);
            state->engine.resize(state->gl.width(), state->gl.height());
        }
        break;
    case APP_CMD_WINDOW_INSETS_CHANGED:
        applyInsets(app, *state);
        break;
    case APP_CMD_GAINED_FOCUS:
        state->hasFocus = true;
        state->engine.resumeClock();
        state->audio.start();
        break;
    case APP_CMD_LOST_FOCUS:
        state->hasFocus = false;
        state->audio.stop();
        break;
    default:
        break;
    }
}

engine::TouchEvent::Phase phaseFor(int actionMasked) {
    using Phase = engine::TouchEvent::Phase;
    switch (actionMasked) {
    case AMOTION_EVENT_ACTION_DOWN:
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
        return Phase::Down;
    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_POINTER_UP:
        return Phase::Up;
    case AMOTION_EVENT_ACTION_CANCEL:
        return Phase::Cancel;
    default:
        return Phase::Move;
    }
}

void processInput(android_app* app, AppState& state) {
    android_input_buffer* input = android_app_swap_input_buffers(app);
    if (input == nullptr) {
        return;
    }

    for (uint64_t i = 0; i < input->motionEventsCount; ++i) {
        const GameActivityMotionEvent& event = input->motionEvents[i];
        const int actionMasked = event.action & AMOTION_EVENT_ACTION_MASK;
        const engine::TouchEvent::Phase phase = phaseFor(actionMasked);

        if (phase == engine::TouchEvent::Phase::Move) {
            // MOVE carries every active pointer; report each one.
            for (uint32_t p = 0; p < event.pointerCount; ++p) {
                const GameActivityPointerAxes& ptr = event.pointers[p];
                state.engine.onTouch(ptr.id, phase, GameActivityPointerAxes_getX(&ptr),
                                     GameActivityPointerAxes_getY(&ptr));
            }
        } else {
            const uint32_t index = (event.action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                                   AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            if (index < event.pointerCount) {
                const GameActivityPointerAxes& ptr = event.pointers[index];
                state.engine.onTouch(ptr.id, phase, GameActivityPointerAxes_getX(&ptr),
                                     GameActivityPointerAxes_getY(&ptr));
            }
        }
    }
    android_app_clear_motion_events(input);

    for (uint64_t i = 0; i < input->keyEventsCount; ++i) {
        const GameActivityKeyEvent& key = input->keyEvents[i];
        if (key.keyCode == AKEYCODE_BACK && key.action == AKEY_EVENT_ACTION_UP) {
            if (!state.engine.onBack()) {
                GameActivity_finish(app->activity);
            }
        }
    }
    android_app_clear_key_events(input);
}

}  // namespace

extern "C" void android_main(android_app* app) {
    LOGI("android_main start");

    AppState state(app);
    app->userData = &state;
    app->onAppCmd = handleAppCmd;

    // Receive every motion source and every key (the default filter drops BACK).
    android_app_set_motion_event_filter(app, nullptr);
    android_app_set_key_event_filter(app, nullptr);

    while (true) {
        int events = 0;
        android_poll_source* source = nullptr;

        // Block while idle, spin while animating. The timeout is re-evaluated
        // every iteration so a command that starts animation falls through to draw.
        while (ALooper_pollOnce(state.animating() ? 0 : -1, nullptr, &events,
                                reinterpret_cast<void**>(&source)) >= 0) {
            if (source != nullptr) {
                source->process(app, source);
            }
            if (app->destroyRequested) {
                LOGI("android_main exit");
                return;
            }
        }

        processInput(app, state);

        if (state.animating()) {
            state.engine.frame();
            state.gl.swapBuffers();
        }
    }
}
