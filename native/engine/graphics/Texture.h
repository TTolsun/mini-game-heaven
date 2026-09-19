#pragma once

#include <GLES3/gl3.h>

#include <cstdint>

namespace engine {

struct Image;

// GPU texture (RGBA8). Owns the GL object.
class Texture {
public:
    Texture() = default;
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    // Allocates an empty texture; contents are undefined until upload().
    bool create(int width, int height);
    bool create(const Image& image);

    // Copies a rectangle of RGBA8 pixels into the texture.
    void upload(int x, int y, int width, int height, const uint8_t* rgba);

    void bind(int unit = 0) const;
    void setNearest();

    GLuint id() const { return id_; }
    int width() const { return width_; }
    int height() const { return height_; }
    bool isValid() const { return id_ != 0; }

private:
    void release();

    GLuint id_ = 0;
    int width_ = 0;
    int height_ = 0;
};

}  // namespace engine
