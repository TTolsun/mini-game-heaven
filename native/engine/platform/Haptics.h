#pragma once

namespace engine {

// Vibration feedback. The Android implementation goes through JNI; the
// default no-op keeps games free of null checks.
class Haptics {
public:
    enum class Strength { Light, Medium, Heavy };

    virtual ~Haptics() = default;
    virtual void vibrate(Strength strength) = 0;

    void light() { vibrate(Strength::Light); }
    void medium() { vibrate(Strength::Medium); }
    void heavy() { vibrate(Strength::Heavy); }
};

class NullHaptics final : public Haptics {
public:
    void vibrate(Strength) override {}
};

}  // namespace engine
