#pragma once

#include <jni.h>

#include "engine/platform/Haptics.h"

struct android_app;

namespace platform::android {

// Drives android.os.Vibrator through JNI. There is no NDK vibration API, so
// this is the one place the project touches Java at runtime.
class AndroidHaptics final : public engine::Haptics {
public:
    explicit AndroidHaptics(android_app* app);
    ~AndroidHaptics() override;

    void vibrate(Strength strength) override;

private:
    bool resolve(JNIEnv* env);

    android_app* app_;
    jobject vibrator_ = nullptr;   // global ref
    jclass effectClass_ = nullptr; // global ref to VibrationEffect
    jmethodID createOneShot_ = nullptr;
    jmethodID vibrateEffect_ = nullptr;
    bool resolved_ = false;
    bool available_ = false;
};

}  // namespace platform::android
