#pragma once

#include <chrono>

namespace engine {

// Frame timer. tick() returns the seconds elapsed since the previous tick,
// clamped so a long pause (app switch, debugger) does not explode physics.
class Timer {
public:
    using Clock = std::chrono::steady_clock;

    Timer() : last_(Clock::now()) {}

    float tick() {
        const auto now = Clock::now();
        float dt = std::chrono::duration<float>(now - last_).count();
        last_ = now;
        if (dt > kMaxDelta) {
            dt = kMaxDelta;
        }
        elapsed_ += dt;
        return dt;
    }

    // Call after a pause so the next tick does not include the paused time.
    void reset() { last_ = Clock::now(); }

    float elapsed() const { return elapsed_; }

private:
    static constexpr float kMaxDelta = 1.0f / 15.0f;

    Clock::time_point last_;
    float elapsed_ = 0.0f;
};

}  // namespace engine
