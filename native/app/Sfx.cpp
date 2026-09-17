#include "app/Sfx.h"

#include "engine/audio/Synth.h"

namespace app {

using namespace engine::synth;

void Sfx::build() {
    click = tone(0.06f, 900.0f, 600.0f, 6.0f, 0.5f);
    tap = mix(tone(0.08f, 700.0f, 300.0f, 8.0f, 0.7f), noise(0.04f, 12.0f, 0.5f, 0.3f));
    jump = tone(0.16f, 300.0f, 900.0f, 4.0f, 0.55f);
    doubleJump = tone(0.14f, 600.0f, 1400.0f, 4.0f, 0.5f);
    land = mix(noise(0.09f, 10.0f, 0.25f, 0.6f), tone(0.08f, 160.0f, 80.0f, 8.0f, 0.5f));
    hit = mix(noise(0.22f, 6.0f, 0.6f, 0.8f), tone(0.22f, 220.0f, 60.0f, 6.0f, 0.8f));
    splash = mix(noise(0.35f, 5.0f, 0.15f, 0.7f), tone(0.25f, 500.0f, 150.0f, 5.0f, 0.3f));
    ding = tone(0.22f, 1320.0f, 1320.0f, 7.0f, 0.45f);
    crack = mix(noise(0.18f, 8.0f, 0.8f, 0.8f), tone(0.12f, 400.0f, 120.0f, 9.0f, 0.5f));
    thud = mix(noise(0.14f, 9.0f, 0.2f, 0.7f), tone(0.14f, 120.0f, 50.0f, 7.0f, 0.7f));
    tick = tone(0.05f, 1200.0f, 1200.0f, 10.0f, 0.4f);

    const float major[] = {523.25f, 659.25f, 783.99f, 1046.5f};
    fanfare = arpeggio(major, 4, 0.35f, 0.09f, 0.6f);
    const float best[] = {659.25f, 783.99f, 987.77f, 1318.5f, 1568.0f};
    newBest = arpeggio(best, 5, 0.4f, 0.08f, 0.6f);
}

}  // namespace app
