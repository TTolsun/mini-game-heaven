#include "app/Sfx.h"

#include "engine/audio/Synth.h"

namespace app {

using namespace engine::synth;

void Sfx::build() {
    click = tone(0.06f, 900.0f, 600.0f, 6.0f, 0.5f);
    tap = mix(tone(0.08f, 700.0f, 300.0f, 8.0f, 0.7f), noise(0.04f, 12.0f, 0.5f, 0.3f));

    const float major[] = {523.25f, 659.25f, 783.99f, 1046.5f};
    fanfare = arpeggio(major, 4, 0.35f, 0.09f, 0.6f);
}

}  // namespace app
