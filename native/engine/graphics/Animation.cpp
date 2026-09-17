#include "engine/graphics/Animation.h"

#include <cmath>
#include <cstdio>

#include "engine/graphics/TextureAtlas.h"

namespace engine {

Animation Animation::fromAtlas(const TextureAtlas& atlas, const std::string& prefix, float fps,
                               bool loop) {
    Animation anim;
    anim.fps_ = fps;
    anim.loop_ = loop;

    for (int i = 1;; ++i) {
        char name[128];
        std::snprintf(name, sizeof(name), "%s_%02d", prefix.c_str(), i);
        if (!atlas.has(name)) {
            break;
        }
        anim.frames_.push_back(atlas.get(name));
    }
    return anim;
}

void Animation::update(float dt) {
    if (frames_.empty()) {
        return;
    }
    time_ += dt;
    const float total = duration();
    if (loop_ && time_ >= total) {
        time_ = std::fmod(time_, total);
    }
}

const Sprite& Animation::frame() const {
    static const Sprite kNone;
    if (frames_.empty()) {
        return kNone;
    }
    size_t index = static_cast<size_t>(time_ * fps_);
    if (index >= frames_.size()) {
        index = frames_.size() - 1;
    }
    return frames_[index];
}

}  // namespace engine
