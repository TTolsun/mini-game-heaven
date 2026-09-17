#pragma once

#include <EGL/egl.h>

struct ANativeWindow;

namespace platform::android {

// Owns the EGL display/surface/context bound to one ANativeWindow.
class GlContext {
public:
    GlContext() = default;
    ~GlContext();

    GlContext(const GlContext&) = delete;
    GlContext& operator=(const GlContext&) = delete;

    bool init(ANativeWindow* window);
    void shutdown();

    bool isValid() const { return context_ != EGL_NO_CONTEXT; }
    int width() const { return width_; }
    int height() const { return height_; }

    void swapBuffers();

private:
    EGLDisplay display_ = EGL_NO_DISPLAY;
    EGLSurface surface_ = EGL_NO_SURFACE;
    EGLContext context_ = EGL_NO_CONTEXT;
    int width_ = 0;
    int height_ = 0;
};

}  // namespace platform::android
