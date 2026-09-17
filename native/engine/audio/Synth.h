#pragma once

#include <functional>

#include "engine/audio/Sound.h"

namespace engine::synth {

// Tiny procedural sound toolkit so the game ships no audio files.
// All functions produce Sounds at Mixer::kSampleRate.

// Frequency sweep from f0 to f1 over `seconds` with an exponential decay envelope.
Sound tone(float seconds, float f0, float f1, float decay, float volume = 0.8f);

// White noise with exponential decay; `lowpass` in 0..1 softens it (1 = raw).
Sound noise(float seconds, float decay, float lowpass = 1.0f, float volume = 0.6f);

// Short sine pulse (mallet-like) at `frequency`.
Sound pluck(float seconds, float frequency, float volume = 0.8f);

// Sequence of plucks, `gap` seconds apart (a fanfare).
Sound arpeggio(const float* frequencies, int count, float noteSeconds, float gap, float volume = 0.7f);

// Sums two sounds sample-wise (the longer length wins).
Sound mix(const Sound& a, const Sound& b, float gainA = 1.0f, float gainB = 1.0f);

}  // namespace engine::synth
