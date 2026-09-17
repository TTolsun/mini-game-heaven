#pragma once

#include <EGL/egl.h>

struct ANativeWindow;

namespace platform::android {

// Owns the EGL display and context for the app's lifetime, and a window
// surface that comes and goes with the Activity's window. Keeping the context
// across surface loss means GL textures and buffers survive backgrounding.
class GlContext {
public:
    GlContext() = default;
    ~GlContext();

    GlContext(const GlContext&) = delete;
    GlContext& operator=(const GlContext&) = delete;

    // Creates display/context on first call, then a surface for `window`.
    bool createSurface(ANativeWindow* window);
    void destroySurface();
    void shutdown();

    bool hasSurface() const { return surface_ != EGL_NO_SURFACE; }
    int width() const { return width_; }
    int height() const { return height_; }

    void swapBuffers();

private:
    bool initDisplay();

    EGLDisplay display_ = EGL_NO_DISPLAY;
    EGLConfig config_ = nullptr;
    EGLContext context_ = EGL_NO_CONTEXT;
    EGLSurface surface_ = EGL_NO_SURFACE;
    int width_ = 0;
    int height_ = 0;
};

}  // namespace platform::android
