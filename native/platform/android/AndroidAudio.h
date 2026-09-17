#pragma once

#include <aaudio/AAudio.h>

namespace engine {
class Mixer;
}

namespace platform::android {

// Low-latency output stream that pulls samples from the engine Mixer.
class AndroidAudio {
public:
    explicit AndroidAudio(engine::Mixer& mixer) : mixer_(mixer) {}
    ~AndroidAudio();

    bool start();
    void stop();

private:
    static aaudio_data_callback_result_t onData(AAudioStream* stream, void* userData, void* audioData,
                                                int32_t numFrames);
    static void onError(AAudioStream* stream, void* userData, aaudio_result_t error);

    engine::Mixer& mixer_;
    AAudioStream* stream_ = nullptr;
    int32_t channels_ = 2;
};

}  // namespace platform::android
