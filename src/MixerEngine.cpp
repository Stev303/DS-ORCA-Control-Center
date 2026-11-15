#include "MixerEngine.h"

#include <algorithm>
#include <cmath>

namespace dsorca {

MixerEngine::MixerEngine() = default;

float MixerEngine::Clamp(float value, float low, float high) {
    return std::max(low, std::min(value, high));
}

float MixerEngine::DbToLinear(float db) {
    return std::pow(10.0f, db / 20.0f);
}

bool MixerEngine::SetChannelState(std::size_t channel, const ChannelState& state) {
    if (channel >= kMaxMixerChannels) return false;
    channels_[channel] = state;
    channels_[channel].gainDb = Clamp(state.gainDb, -60.0f, 12.0f);
    channels_[channel].pan = Clamp(state.pan, -1.0f, 1.0f);
    return true;
}

const ChannelState& MixerEngine::GetChannelState(std::size_t channel) const {
    static const ChannelState invalid{};
    return channel < kMaxMixerChannels ? channels_[channel] : invalid;
}

void MixerEngine::SetMasterGainDb(float gainDb) {
    masterGainDb_ = Clamp(gainDb, -60.0f, 12.0f);
}

float MixerEngine::GetMasterGainDb() const {
    return masterGainDb_;
}

void MixerEngine::Process(const float* const* inputs, std::size_t inputCount,
                          float* output, std::size_t frameCount) {
    if (output == nullptr) return;
    inputCount = std::min(inputCount, kMaxMixerChannels);

    bool anySolo = false;
    for (std::size_t channel = 0; channel < inputCount; ++channel) {
        if (inputs != nullptr && inputs[channel] != nullptr && channels_[channel].solo &&
            !channels_[channel].muted) {
            anySolo = true;
            break;
        }
    }

    for (std::size_t channel = 0; channel < kMaxMixerChannels; ++channel)
        channelMeters_[channel] = {};

    float masterLeftPeak = 0.0f;
    float masterRightPeak = 0.0f;
    const float masterGain = DbToLinear(masterGainDb_);

    for (std::size_t frame = 0; frame < frameCount; ++frame) {
        float left = 0.0f;
        float right = 0.0f;

        for (std::size_t channel = 0; channel < inputCount; ++channel) {
            if (inputs == nullptr || inputs[channel] == nullptr) continue;
            const ChannelState& state = channels_[channel];
            if (state.muted || (anySolo && !state.solo)) continue;

            const float gain = DbToLinear(state.gainDb);
            const float panLeft = state.pan > 0.0f ? 1.0f - state.pan : 1.0f;
            const float panRight = state.pan < 0.0f ? 1.0f + state.pan : 1.0f;
            const float inLeft = inputs[channel][frame * 2] * gain * panLeft;
            const float inRight = inputs[channel][frame * 2 + 1] * gain * panRight;

            left += inLeft;
            right += inRight;
            channelMeters_[channel].leftPeak =
                std::max(channelMeters_[channel].leftPeak, std::fabs(inLeft));
            channelMeters_[channel].rightPeak =
                std::max(channelMeters_[channel].rightPeak, std::fabs(inRight));
        }

        left *= masterGain;
        right *= masterGain;
        output[frame * 2] = left;
        output[frame * 2 + 1] = right;
        masterLeftPeak = std::max(masterLeftPeak, std::fabs(left));
        masterRightPeak = std::max(masterRightPeak, std::fabs(right));
    }

    masterMeter_ = { masterLeftPeak, masterRightPeak };
}

MeterState MixerEngine::GetChannelMeter(std::size_t channel) const {
    return channel < kMaxMixerChannels ? channelMeters_[channel] : MeterState{};
}

MeterState MixerEngine::GetMasterMeter() const {
    return masterMeter_;
}

void MixerEngine::ResetMeters() {
    channelMeters_.fill({});
    masterMeter_ = {};
}

} // namespace dsorca
