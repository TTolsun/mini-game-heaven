#include "engine/audio/Mixer.h"

#include <algorithm>
#include <cmath>

namespace engine {

void Mixer::play(const Sound& sound, float gain, float pitch) {
    if (sound.empty()) {
        return;
    }
    // Take the first free voice. A free voice is not being touched by the
    // audio thread (it only reads voices whose sound is non-null), so the
    // plain fields can be set before the pointer is published. If every voice
    // is busy the sound is dropped: stealing would race with render().
    for (Voice& v : voices_) {
        if (v.sound.load(std::memory_order_acquire) == nullptr) {
            v.position = 0.0;
            v.gain = gain;
            v.pitch = pitch;
            v.sound.store(&sound, std::memory_order_release);
            return;
        }
    }
}

void Mixer::render(float* out, int frames) {
    std::fill(out, out + frames, 0.0f);
    const float master = masterGain_.load();

    for (Voice& v : voices_) {
        const Sound* sound = v.sound.load(std::memory_order_acquire);
        if (sound == nullptr) {
            continue;
        }
        const std::vector<float>& s = sound->samples;
        const double step = v.pitch;
        double pos = v.position;
        const float gain = v.gain * master;

        bool finished = false;
        for (int i = 0; i < frames; ++i) {
            const size_t index = static_cast<size_t>(pos);
            if (index + 1 >= s.size()) {
                finished = true;
                break;
            }
            const float frac = static_cast<float>(pos - index);
            out[i] += (s[index] + (s[index + 1] - s[index]) * frac) * gain;
            pos += step;
        }
        if (finished) {
            // Releasing the pointer hands the voice back to play(); nothing may
            // be written to this voice after the store.
            v.sound.store(nullptr, std::memory_order_release);
        } else {
            v.position = pos;
        }
    }

    for (int i = 0; i < frames; ++i) {
        out[i] = std::clamp(out[i], -1.0f, 1.0f);
    }
}

}  // namespace engine
