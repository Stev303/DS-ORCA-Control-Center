#include "MixerEngine.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {
bool Near(float a, float b, float tolerance = 0.0005f) {
    return std::fabs(a - b) <= tolerance;
}
void Check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}
}

int main() {
    using namespace dsorca;
    MixerEngine mixer;
    const float strip0[] = { 0.25f, -0.5f, 0.5f, -0.25f };
    const float* inputs[] = { strip0 };
    float output[4]{};

    mixer.Process(inputs, 1, output, 2);
    Check(Near(output[0], 0.25f) && Near(output[1], -0.5f), "unity gain pass-through");
    Check(Near(mixer.GetMasterMeter().leftPeak, 0.5f), "master left peak meter");

    ChannelState state{};
    state.gainDb = -6.0206f;
    Check(mixer.SetChannelState(0, state), "set channel state");
    mixer.Process(inputs, 1, output, 2);
    Check(Near(output[0], 0.125f), "-6 dB channel gain");

    state = mixer.GetChannelState(0);
    state.muted = true;
    mixer.SetChannelState(0, state);
    mixer.Process(inputs, 1, output, 2);
    Check(Near(output[0], 0.0f) && Near(output[1], 0.0f), "mute");

    state.muted = false;
    state.pan = 1.0f;
    mixer.SetChannelState(0, state);
    mixer.Process(inputs, 1, output, 2);
    Check(Near(output[0], 0.0f) && Near(output[1], -0.125f), "hard-right balance");

    const float strip1[] = { 0.2f, 0.2f, 0.2f, 0.2f };
    const float* twoInputs[] = { strip0, strip1 };
    state = mixer.GetChannelState(0);
    state.pan = 0.0f;
    state.solo = true;
    mixer.SetChannelState(0, state);
    mixer.Process(twoInputs, 2, output, 2);
    Check(Near(output[0], 0.125f), "solo excludes other strips");

    Check(!mixer.SetChannelState(kMaxMixerChannels, {}), "reject out-of-range channel");
    mixer.SetMasterGainDb(6.0f);
    Check(Near(mixer.GetMasterGainDb(), 6.0f), "master gain setter");
    mixer.ResetMeters();
    Check(Near(mixer.GetMasterMeter().leftPeak, 0.0f), "reset meters");

    std::cout << "MixerEngine tests passed\n";
    return 0;
}
