#pragma once

#include <GLES3/gl3.h>

#include <vector>

#include "engine/graphics/Shader.h"
#include "engine/graphics/Sprite.h"
#include "engine/graphics/Texture.h"
#include "engine/math/Color.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"

namespace engine {

// Accumulates textured quads and submits them in as few draw calls as possible.
// A flush happens when the bound texture changes or the buffer fills up.
//
// Coordinates are "world" units set by begin(): (0,0) is the top-left corner,
// x grows right and y grows down, like a phone screen.
class SpriteBatch {
public:
    SpriteBatch() = default;
    ~SpriteBatch();

    SpriteBatch(const SpriteBatch&) = delete;
    SpriteBatch& operator=(const SpriteBatch&) = delete;

    bool init();

    // worldWidth/worldHeight define the orthographic projection for this frame.
    void begin(float worldWidth, float worldHeight);
    void end();

    // Draws `sprite` centred at `center`, scaled to `size` world units.
    // rotation is radians, clockwise on screen. flipX mirrors horizontally.
    void draw(const Sprite& sprite, Vec2 center, Vec2 size, Color tint = Color::white(),
              float rotation = 0.0f, bool flipX = false);

    // Convenience: draw at the sprite's own pixel size times `scale`.
    void draw(const Sprite& sprite, Vec2 center, float scale = 1.0f, Color tint = Color::white(),
              float rotation = 0.0f, bool flipX = false);

    // Solid rectangle (uses an internal 1x1 white texture).
    void drawRect(const Rect& rect, Color color);

    int drawCalls() const { return drawCalls_; }

private:
    struct Vertex {
        float x, y;
        float u, v;
        float r, g, b, a;
    };

    static constexpr int kMaxQuads = 2048;

    void pushQuad(const Texture* texture, const Vec2 corners[4], float u0, float v0, float u1,
                  float v1, Color color);
    void flush();

    Shader shader_;
    GLint projectionLoc_ = -1;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ibo_ = 0;
    Texture white_;
    Sprite whiteSprite_;

    std::vector<Vertex> vertices_;
    const Texture* currentTexture_ = nullptr;
    float projection_[16] = {};
    int drawCalls_ = 0;
    bool active_ = false;
};

}  // namespace engine
