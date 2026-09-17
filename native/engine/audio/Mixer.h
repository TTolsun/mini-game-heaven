#pragma once

#include <array>
#include <atomic>
#include <cstdint>

#include "engine/audio/Sound.h"

namespace engine {

// Polyphonic sample mixer. play() is called from the game thread, render()
// from the audio callback; voices are claimed with atomics so neither side
// blocks. Sounds must outlive any voice playing them.
class Mixer {
public:
    static constexpr int kSampleRate = 44100;
    static constexpr int kMaxVoices = 12;

    void play(const Sound& sound, float gain = 1.0f, float pitch = 1.0f);

    // Fills `frames` mono samples into `out` (already interleaved by the caller if stereo).
    void render(float* out, int frames);

    void setMasterGain(float gain) { masterGain_.store(gain); }

private:
    struct Voice {
        std::atomic<const Sound*> sound{nullptr};
        double position = 0.0;
        float gain = 1.0f;
        float pitch = 1.0f;
    };

    std::array<Voice, kMaxVoices> voices_;
    std::atomic<float> masterGain_{0.8f};
};

}  // namespace engine
