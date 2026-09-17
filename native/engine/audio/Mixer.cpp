#include "engine/audio/Mixer.h"

#include <algorithm>
#include <cmath>

namespace engine {

void Mixer::play(const Sound& sound, float gain, float pitch) {
    if (sound.empty()) {
        return;
    }
    // Take the first free voice; if all are busy, steal the first one.
    for (Voice& v : voices_) {
        const Sound* expected = nullptr;
        if (v.sound.compare_exchange_strong(expected, nullptr)) {
            v.position = 0.0;
            v.gain = gain;
            v.pitch = pitch;
            v.sound.store(&sound);
            return;
        }
    }
    Voice& v = voices_[0];
    v.sound.store(nullptr);
    v.position = 0.0;
    v.gain = gain;
    v.pitch = pitch;
    v.sound.store(&sound);
}

void Mixer::render(float* out, int frames) {
    std::fill(out, out + frames, 0.0f);
    const float master = masterGain_.load();

    for (Voice& v : voices_) {
        const Sound* sound = v.sound.load();
        if (sound == nullptr) {
            continue;
        }
        const std::vector<float>& s = sound->samples;
        const double step = v.pitch;
        double pos = v.position;
        const float gain = v.gain * master;

        for (int i = 0; i < frames; ++i) {
            const size_t index = static_cast<size_t>(pos);
            if (index + 1 >= s.size()) {
                v.sound.store(nullptr);
                break;
            }
            const float frac = static_cast<float>(pos - index);
            out[i] += (s[index] + (s[index + 1] - s[index]) * frac) * gain;
            pos += step;
        }
        v.position = pos;
    }

    for (int i = 0; i < frames; ++i) {
        out[i] = std::clamp(out[i], -1.0f, 1.0f);
    }
}

}  // namespace engine
