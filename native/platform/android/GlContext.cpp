#include "platform/android/GlContext.h"

#include <android/log.h>
#include <android/native_window.h>

namespace platform::android {

namespace {

constexpr const char* kTag = "GlContext";

}  // namespace

GlContext::~GlContext() {
    shutdown();
}

bool GlContext::init(ANativeWindow* window) {
    display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display_ == EGL_NO_DISPLAY || !eglInitialize(display_, nullptr, nullptr)) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "eglInitialize failed");
        return false;
    }

    const EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 0,
        EGL_NONE,
    };
    EGLConfig config = nullptr;
    EGLint numConfigs = 0;
    if (!eglChooseConfig(display_, configAttribs, &config, 1, &numConfigs) || numConfigs == 0) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "eglChooseConfig failed");
        return false;
    }

    surface_ = eglCreateWindowSurface(display_, config, window, nullptr);
    if (surface_ == EGL_NO_SURFACE) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "eglCreateWindowSurface failed");
        return false;
    }

    const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    context_ = eglCreateContext(display_, config, EGL_NO_CONTEXT, contextAttribs);
    if (context_ == EGL_NO_CONTEXT) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "eglCreateContext failed");
        return false;
    }

    if (!eglMakeCurrent(display_, surface_, surface_, context_)) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "eglMakeCurrent failed");
        return false;
    }

    eglQuerySurface(display_, surface_, EGL_WIDTH, &width_);
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height_);
    __android_log_print(ANDROID_LOG_INFO, kTag, "EGL ready %dx%d", width_, height_);
    return true;
}

void GlContext::shutdown() {
    if (display_ == EGL_NO_DISPLAY) {
        return;
    }
    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (context_ != EGL_NO_CONTEXT) {
        eglDestroyContext(display_, context_);
    }
    if (surface_ != EGL_NO_SURFACE) {
        eglDestroySurface(display_, surface_);
    }
    eglTerminate(display_);

    display_ = EGL_NO_DISPLAY;
    surface_ = EGL_NO_SURFACE;
    context_ = EGL_NO_CONTEXT;
    width_ = height_ = 0;
}

void GlContext::swapBuffers() {
    eglSwapBuffers(display_, surface_);
}

}  // namespace platform::android
