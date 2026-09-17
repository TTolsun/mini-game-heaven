// Android entry point. Bridges GameActivity's native_app_glue to the C++ game.
// Everything Android-specific stays under platform/android/.

#include <android/log.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <GLES3/gl3.h>

#include "platform/android/GlContext.h"

namespace {

constexpr const char* kTag = "MiniGameHeaven";

struct AppState {
    platform::android::GlContext gl;
    bool animating = false;
};

void handleAppCmd(android_app* app, int32_t cmd) {
    auto* state = static_cast<AppState*>(app->userData);

    switch (cmd) {
    case APP_CMD_INIT_WINDOW:
        if (app->window != nullptr && state->gl.init(app->window)) {
            state->animating = true;
        }
        break;
    case APP_CMD_TERM_WINDOW:
        state->animating = false;
        state->gl.shutdown();
        break;
    case APP_CMD_GAINED_FOCUS:
        state->animating = state->gl.isValid();
        break;
    case APP_CMD_LOST_FOCUS:
        state->animating = false;
        break;
    default:
        break;
    }
}

void drainInput(android_app* app) {
    android_input_buffer* input = android_app_swap_input_buffers(app);
    if (input == nullptr) {
        return;
    }
    // Input is not handled yet; just clear the buffers so they do not overflow.
    if (input->motionEventsCount > 0) {
        android_app_clear_motion_events(input);
    }
    if (input->keyEventsCount > 0) {
        android_app_clear_key_events(input);
    }
}

void drawFrame(AppState& state) {
    glViewport(0, 0, state.gl.width(), state.gl.height());
    glClearColor(0.08f, 0.09f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    state.gl.swapBuffers();
}

}  // namespace

extern "C" void android_main(android_app* app) {
    __android_log_print(ANDROID_LOG_INFO, kTag, "android_main start");

    AppState state;
    app->userData = &state;
    app->onAppCmd = handleAppCmd;

    // Receive every motion source, not only the touchscreen default filter.
    android_app_set_motion_event_filter(app, nullptr);

    while (true) {
        int events = 0;
        android_poll_source* source = nullptr;

        // Block while idle, spin while animating. The timeout is re-evaluated
        // every iteration so a command that starts animation falls through to draw.
        while (ALooper_pollOnce(state.animating ? 0 : -1, nullptr, &events,
                                reinterpret_cast<void**>(&source)) >= 0) {
            if (source != nullptr) {
                source->process(app, source);
            }
            if (app->destroyRequested) {
                __android_log_print(ANDROID_LOG_INFO, kTag, "android_main exit");
                return;
            }
        }

        drainInput(app);

        if (state.animating) {
            drawFrame(state);
        }
    }
}
