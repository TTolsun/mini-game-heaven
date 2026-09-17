#pragma once

#include <vector>

namespace engine {

// Mono PCM clip at Mixer::kSampleRate.
struct Sound {
    std::vector<float> samples;

    bool empty() const { return samples.empty(); }
};

}  // namespace engine
