#include "engine/audio/Synth.h"

#include <algorithm>
#include <cmath>
#include <random>

#include "engine/audio/Mixer.h"

namespace engine::synth {

namespace {

constexpr float kPi = 3.14159265f;
constexpr float kRate = static_cast<float>(Mixer::kSampleRate);

// 2 ms fade at both ends so clips never click.
void declick(Sound& s) {
    const size_t ramp = std::min<size_t>(s.samples.size() / 2, static_cast<size_t>(kRate * 0.002f));
    for (size_t i = 0; i < ramp; ++i) {
        const float g = static_cast<float>(i) / ramp;
        s.samples[i] *= g;
        s.samples[s.samples.size() - 1 - i] *= g;
    }
}

}  // namespace

Sound tone(float seconds, float f0, float f1, float decay, float volume) {
    Sound s;
    const size_t n = static_cast<size_t>(seconds * kRate);
    s.samples.resize(n);
    float phase = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        const float t = static_cast<float>(i) / n;
        const float f = f0 + (f1 - f0) * t;
        phase += 2.0f * kPi * f / kRate;
        const float env = std::exp(-decay * t);
        // Sine with a touch of square for brightness.
        const float v = std::sin(phase) * 0.8f + (std::sin(phase) > 0.0f ? 0.2f : -0.2f);
        s.samples[i] = v * env * volume;
    }
    declick(s);
    return s;
}

Sound noise(float seconds, float decay, float lowpass, float volume) {
    Sound s;
    const size_t n = static_cast<size_t>(seconds * kRate);
    s.samples.resize(n);
    std::mt19937 rng(7);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    float last = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        const float t = static_cast<float>(i) / n;
        const float raw = dist(rng);
        last += (raw - last) * lowpass;  // one-pole lowpass
        s.samples[i] = last * std::exp(-decay * t) * volume;
    }
    declick(s);
    return s;
}

Sound pluck(float seconds, float frequency, float volume) {
    Sound s;
    const size_t n = static_cast<size_t>(seconds * kRate);
    s.samples.resize(n);
    for (size_t i = 0; i < n; ++i) {
        const float t = static_cast<float>(i) / n;
        const float phase = 2.0f * kPi * frequency * i / kRate;
        const float env = std::exp(-5.0f * t);
        // Fundamental plus a quiet octave gives a marimba-ish body.
        s.samples[i] = (std::sin(phase) + 0.3f * std::sin(phase * 2.0f)) * env * volume * 0.75f;
    }
    declick(s);
    return s;
}

Sound arpeggio(const float* frequencies, int count, float noteSeconds, float gap, float volume) {
    Sound s;
    const size_t total = static_cast<size_t>((gap * (count - 1) + noteSeconds) * kRate);
    s.samples.assign(total, 0.0f);
    for (int k = 0; k < count; ++k) {
        const Sound note = pluck(noteSeconds, frequencies[k], volume);
        const size_t start = static_cast<size_t>(gap * k * kRate);
        for (size_t i = 0; i < note.samples.size() && start + i < total; ++i) {
            s.samples[start + i] += note.samples[i];
        }
    }
    for (float& v : s.samples) {
        v = std::clamp(v, -1.0f, 1.0f);
    }
    return s;
}

Sound mix(const Sound& a, const Sound& b, float gainA, float gainB) {
    Sound s;
    s.samples.assign(std::max(a.samples.size(), b.samples.size()), 0.0f);
    for (size_t i = 0; i < a.samples.size(); ++i) {
        s.samples[i] += a.samples[i] * gainA;
    }
    for (size_t i = 0; i < b.samples.size(); ++i) {
        s.samples[i] += b.samples[i] * gainB;
    }
    for (float& v : s.samples) {
        v = std::clamp(v, -1.0f, 1.0f);
    }
    return s;
}

}  // namespace engine::synth
