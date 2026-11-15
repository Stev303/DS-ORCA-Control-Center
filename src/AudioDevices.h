#pragma once
#include <string>
#include <vector>

struct AudioEndpoint {
    std::wstring name;
    std::wstring id;
    std::wstring flow;
    std::wstring state;
};

std::vector<AudioEndpoint> EnumerateAudioEndpoints();

// Controls the Windows Core Audio endpoint level. These functions do not claim
// to change undocumented DSP controls inside the DS ORCA hardware.
bool GetEndpointVolume(const std::wstring& endpointId, float& level);
bool SetEndpointVolume(const std::wstring& endpointId, float level);
bool GetEndpointMute(const std::wstring& endpointId, bool& muted);
bool SetEndpointMute(const std::wstring& endpointId, bool muted);
