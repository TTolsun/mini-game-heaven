#define LOG_TAG "AndroidAudio"
#include "platform/android/AndroidAudio.h"

#include <vector>

#include "engine/audio/Mixer.h"
#include "engine/core/Log.h"

namespace platform::android {

AndroidAudio::~AndroidAudio() {
    stop();
}

bool AndroidAudio::start() {
    if (stream_ != nullptr) {
        return true;
    }

    AAudioStreamBuilder* builder = nullptr;
    if (AAudio_createStreamBuilder(&builder) != AAUDIO_OK) {
        LOGE("createStreamBuilder failed");
        return false;
    }
    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
    AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
    AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_FLOAT);
    AAudioStreamBuilder_setSampleRate(builder, engine::Mixer::kSampleRate);
    AAudioStreamBuilder_setChannelCount(builder, channels_);
    AAudioStreamBuilder_setUsage(builder, AAUDIO_USAGE_GAME);
    AAudioStreamBuilder_setDataCallback(builder, onData, this);
    AAudioStreamBuilder_setErrorCallback(builder, onError, this);

    const aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &stream_);
    AAudioStreamBuilder_delete(builder);
    if (result != AAUDIO_OK) {
        LOGE("openStream failed: %s", AAudio_convertResultToText(result));
        stream_ = nullptr;
        return false;
    }

    channels_ = AAudioStream_getChannelCount(stream_);
    if (AAudioStream_getSampleRate(stream_) != engine::Mixer::kSampleRate) {
        // The device resamples for us; pitch is still correct because the
        // stream was requested at the mixer rate. Log it for the curious.
        LOGW("stream rate %d differs from mixer %d", AAudioStream_getSampleRate(stream_),
             engine::Mixer::kSampleRate);
    }
    // Two bursts of buffering keeps glitches away without adding much latency.
    AAudioStream_setBufferSizeInFrames(stream_, AAudioStream_getFramesPerBurst(stream_) * 2);

    if (AAudioStream_requestStart(stream_) != AAUDIO_OK) {
        LOGE("requestStart failed");
        stop();
        return false;
    }
    LOGI("stream started: %d Hz, %d ch, burst %d", AAudioStream_getSampleRate(stream_), channels_,
         AAudioStream_getFramesPerBurst(stream_));
    return true;
}

void AndroidAudio::stop() {
    if (stream_ == nullptr) {
        return;
    }
    AAudioStream_requestStop(stream_);
    AAudioStream_close(stream_);
    stream_ = nullptr;
}

aaudio_data_callback_result_t AndroidAudio::onData(AAudioStream* /*stream*/, void* userData,
                                                   void* audioData, int32_t numFrames) {
    auto* self = static_cast<AndroidAudio*>(userData);
    auto* out = static_cast<float*>(audioData);

    // Mix mono, then duplicate into every channel.
    thread_local std::vector<float> mono;
    mono.resize(static_cast<size_t>(numFrames));
    self->mixer_.render(mono.data(), numFrames);
    for (int32_t i = 0; i < numFrames; ++i) {
        for (int32_t c = 0; c < self->channels_; ++c) {
            out[i * self->channels_ + c] = mono[static_cast<size_t>(i)];
        }
    }
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void AndroidAudio::onError(AAudioStream* /*stream*/, void* /*userData*/, aaudio_result_t error) {
    // Headphones plugged, route change, etc. The game keeps running silently;
    // a restart could be added here if it matters.
    LOGW("stream error: %s", AAudio_convertResultToText(error));
}

}  // namespace platform::android
