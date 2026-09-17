#pragma once

#include <string>
#include <vector>

#include "engine/graphics/Sprite.h"

namespace engine {

class TextureAtlas;

// A sequence of atlas sprites played at a fixed frame rate.
class Animation {
public:
    Animation() = default;

    // Collects frames named "<prefix>_01", "<prefix>_02", ... (two-digit index)
    // from the atlas until one is missing.
    static Animation fromAtlas(const TextureAtlas& atlas, const std::string& prefix, float fps,
                               bool loop = true);

    void update(float dt);
    void restart() { time_ = 0.0f; }

    const Sprite& frame() const;
    bool finished() const { return !loop_ && time_ >= duration(); }
    float duration() const { return frames_.empty() ? 0.0f : frames_.size() / fps_; }
    bool empty() const { return frames_.empty(); }
    int frameCount() const { return static_cast<int>(frames_.size()); }

private:
    std::vector<Sprite> frames_;
    float fps_ = 12.0f;
    float time_ = 0.0f;
    bool loop_ = true;
};

}  // namespace engine
