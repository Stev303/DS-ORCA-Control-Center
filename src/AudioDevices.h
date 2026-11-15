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
