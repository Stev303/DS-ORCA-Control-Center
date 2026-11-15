#include "MixerEngine.h"
#include <algorithm>
#include <cmath>
namespace dsorca {
MixerEngine::MixerEngine() = default;
float MixerEngine::Clamp(float value, float low, float high) { return std::max(low, std::min(value, high)); }
float MixerEngine::DbToLinear(float db) { return std::pow(10.0f, db / 20.0f); }
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
void MixerEngine::SetMasterGainDb(float gainDb) { masterGainDb_ = Clamp(gainDb, -60.0f, 12.0f); }
float MixerEngine::GetMasterGainDb() const { return masterGainDb_; }
void MixerEngine::Process(const float* const* inputs, std::size_t inputCount, float* output, std::size_t frameCount) {
    if (!output) return;
    inputCount = std::min(inputCount, kMaxMixerChannels);
    bool anySolo = false;
    for (std::size_t ch = 0; ch < inputCount; ++ch)
        if (inputs && inputs[ch] && channels_[ch].solo && !channels_[ch].muted) { anySolo = true; break; }
    channelMeters_.fill({});
    float masterL = 0.0f, masterR = 0.0f;
    const float masterGain = DbToLinear(masterGainDb_);
    for (std::size_t frame = 0; frame < frameCount; ++frame) {
        float left = 0.0f, right = 0.0f;
        for (std::size_t ch = 0; ch < inputCount; ++ch) {
            if (!inputs || !inputs[ch]) continue;
            const auto& state = channels_[ch];
            if (state.muted || (anySolo && !state.solo)) continue;
            const float gain = DbToLinear(state.gainDb);
            const float panL = state.pan > 0.0f ? 1.0f - state.pan : 1.0f;
            const float panR = state.pan < 0.0f ? 1.0f + state.pan : 1.0f;
            const float inL = inputs[ch][frame * 2] * gain * panL;
            const float inR = inputs[ch][frame * 2 + 1] * gain * panR;
            left += inL; right += inR;
            channelMeters_[ch].leftPeak = std::max(channelMeters_[ch].leftPeak, std::fabs(inL));
            channelMeters_[ch].rightPeak = std::max(channelMeters_[ch].rightPeak, std::fabs(inR));
        }
        left *= masterGain; right *= masterGain;
        output[frame * 2] = left; output[frame * 2 + 1] = right;
        masterL = std::max(masterL, std::fabs(left));
        masterR = std::max(masterR, std::fabs(right));
    }
    masterMeter_ = {masterL, masterR};
}
MeterState MixerEngine::GetChannelMeter(std::size_t channel) const { return channel < kMaxMixerChannels ? channelMeters_[channel] : MeterState{}; }
MeterState MixerEngine::GetMasterMeter() const { return masterMeter_; }
void MixerEngine::ResetMeters() { channelMeters_.fill({}); masterMeter_ = {}; }
}
