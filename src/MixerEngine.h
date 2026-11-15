#pragma once

#include <array>
#include <cstddef>

namespace dsorca {

constexpr std::size_t kMaxMixerChannels = 8;

struct ChannelState {
    float gainDb = 0.0f;
    float pan = 0.0f;       // -1.0 = left, 0.0 = center, +1.0 = right
    bool muted = false;
    bool solo = false;
};

struct MeterState {
    float leftPeak = 0.0f;
    float rightPeak = 0.0f;
};

class MixerEngine {
public:
    MixerEngine();

    bool SetChannelState(std::size_t channel, const ChannelState& state);
    const ChannelState& GetChannelState(std::size_t channel) const;

    void SetMasterGainDb(float gainDb);
    float GetMasterGainDb() const;

    // Each input points to interleaved stereo float samples: L0,R0,L1,R1...
    // The output buffer must hold frameCount * 2 samples. No memory is allocated.
    // The caller owns device I/O, format conversion and thread scheduling.
    void Process(const float* const* inputs, std::size_t inputCount,
                 float* output, std::size_t frameCount);

    MeterState GetChannelMeter(std::size_t channel) const;
    MeterState GetMasterMeter() const;
    void ResetMeters();

private:
    static float DbToLinear(float db);
    static float Clamp(float value, float low, float high);

    std::array<ChannelState, kMaxMixerChannels> channels_{};
    std::array<MeterState, kMaxMixerChannels> channelMeters_{};
    MeterState masterMeter_{};
    float masterGainDb_ = 0.0f;
};

} // namespace dsorca
