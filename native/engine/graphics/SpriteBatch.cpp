#define LOG_TAG "SpriteBatch"
#include "engine/graphics/SpriteBatch.h"

#include <cmath>

#include "engine/core/Log.h"

namespace engine {

namespace {

constexpr const char* kVertexShader = R"(#version 300 es
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec4 aColor;
uniform mat4 uProjection;
out vec2 vUv;
out vec4 vColor;
void main() {
    vUv = aUv;
    vColor = aColor;
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

constexpr const char* kFragmentShader = R"(#version 300 es
precision mediump float;
in vec2 vUv;
in vec4 vColor;
uniform sampler2D uTexture;
out vec4 fragColor;
void main() {
    // Assets are straight alpha; premultiply here so blending is GL_ONE / ONE_MINUS_SRC_ALPHA.
    vec4 t = texture(uTexture, vUv);
    fragColor = vec4(t.rgb * t.a, t.a) * vColor;
}
)";

// Column-major orthographic matrix mapping (0,0)-(w,h) to clip space with y down.
void makeOrtho(float w, float h, float out[16]) {
    for (int i = 0; i < 16; ++i) {
        out[i] = 0.0f;
    }
    out[0] = 2.0f / w;
    out[5] = -2.0f / h;
    out[10] = -1.0f;
    out[12] = -1.0f;
    out[13] = 1.0f;
    out[15] = 1.0f;
}

}  // namespace

SpriteBatch::~SpriteBatch() {
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
    }
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
    }
    if (ibo_ != 0) {
        glDeleteBuffers(1, &ibo_);
    }
}

bool SpriteBatch::init() {
    if (!shader_.build(kVertexShader, kFragmentShader)) {
        return false;
    }
    projectionLoc_ = shader_.uniform("uProjection");

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ibo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * kMaxQuads * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, x)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, u)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, r)));

    // Index pattern is identical for every quad, so fill it once.
    std::vector<GLushort> indices(static_cast<size_t>(kMaxQuads) * 6);
    for (int q = 0; q < kMaxQuads; ++q) {
        const GLushort base = static_cast<GLushort>(q * 4);
        GLushort* out = &indices[static_cast<size_t>(q) * 6];
        out[0] = base;
        out[1] = base + 1;
        out[2] = base + 2;
        out[3] = base;
        out[4] = base + 2;
        out[5] = base + 3;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), indices.data(),
                 GL_STATIC_DRAW);

    glBindVertexArray(0);

    const uint8_t whitePixel[4] = {255, 255, 255, 255};
    white_.create(1, 1);
    white_.upload(0, 0, 1, 1, whitePixel);
    whiteSprite_.texture = &white_;
    whiteSprite_.width = whiteSprite_.height = 1.0f;

    vertices_.reserve(static_cast<size_t>(kMaxQuads) * 4);
    LOGI("ready (%d quads per flush)", kMaxQuads);
    return glGetError() == GL_NO_ERROR;
}

void SpriteBatch::begin(float worldWidth, float worldHeight) {
    makeOrtho(worldWidth, worldHeight, projection_);
    drawCalls_ = 0;
    currentTexture_ = nullptr;
    vertices_.clear();
    active_ = true;

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);  // premultiplied output from shader
    glDisable(GL_DEPTH_TEST);
}

void SpriteBatch::end() {
    flush();
    active_ = false;
}

void SpriteBatch::draw(const Sprite& sprite, Vec2 center, float scale, Color tint, float rotation,
                       bool flipX) {
    draw(sprite, center, sprite.size() * scale, tint, rotation, flipX);
}

void SpriteBatch::draw(const Sprite& sprite, Vec2 center, Vec2 size, Color tint, float rotation,
                       bool flipX) {
    if (!sprite.isValid()) {
        return;
    }

    const float hw = size.x * 0.5f;
    const float hh = size.y * 0.5f;
    Vec2 corners[4] = {{-hw, -hh}, {hw, -hh}, {hw, hh}, {-hw, hh}};

    if (rotation != 0.0f) {
        const float c = std::cos(rotation);
        const float s = std::sin(rotation);
        for (Vec2& p : corners) {
            p = {p.x * c - p.y * s, p.x * s + p.y * c};
        }
    }
    for (Vec2& p : corners) {
        p += center;
    }

    const float u0 = flipX ? sprite.u1 : sprite.u0;
    const float u1 = flipX ? sprite.u0 : sprite.u1;
    pushQuad(sprite.texture, corners, u0, sprite.v0, u1, sprite.v1, tint);
}

void SpriteBatch::drawRect(const Rect& rect, Color color) {
    const Vec2 corners[4] = {
        {rect.left(), rect.top()},
        {rect.right(), rect.top()},
        {rect.right(), rect.bottom()},
        {rect.left(), rect.bottom()},
    };
    pushQuad(&white_, corners, 0.0f, 0.0f, 1.0f, 1.0f, color);
}

void SpriteBatch::pushQuad(const Texture* texture, const Vec2 corners[4], float u0, float v0,
                           float u1, float v1, Color color) {
    if (!active_) {
        LOGW("draw outside begin/end");
        return;
    }
    if (texture != currentTexture_ || vertices_.size() >= static_cast<size_t>(kMaxQuads) * 4) {
        flush();
        currentTexture_ = texture;
    }

    // Premultiply so blending with GL_ONE / GL_ONE_MINUS_SRC_ALPHA is correct
    // for both textured sprites (assets are straight alpha) and solid rects.
    const Color pm{color.r * color.a, color.g * color.a, color.b * color.a, color.a};

    const float us[4] = {u0, u1, u1, u0};
    const float vs[4] = {v0, v0, v1, v1};
    for (int i = 0; i < 4; ++i) {
        vertices_.push_back({corners[i].x, corners[i].y, us[i], vs[i], pm.r, pm.g, pm.b, pm.a});
    }
}

void SpriteBatch::flush() {
    if (vertices_.empty() || currentTexture_ == nullptr) {
        vertices_.clear();
        return;
    }

    shader_.use();
    glUniformMatrix4fv(projectionLoc_, 1, GL_FALSE, projection_);
    glUniform1i(shader_.uniform("uTexture"), 0);
    currentTexture_->bind(0);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    static_cast<GLsizeiptr>(sizeof(Vertex) * vertices_.size()), vertices_.data());

    const GLsizei indexCount = static_cast<GLsizei>(vertices_.size() / 4 * 6);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);

    ++drawCalls_;
    vertices_.clear();
}

}  // namespace engine
