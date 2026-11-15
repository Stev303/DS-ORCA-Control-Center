#include "AudioDevices.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <propvarutil.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace {
std::wstring ReadState(DWORD state) {
    if (state & DEVICE_STATE_ACTIVE) return L"Active";
    if (state & DEVICE_STATE_DISABLED) return L"Disabled";
    if (state & DEVICE_STATE_NOTPRESENT) return L"Not present";
    if (state & DEVICE_STATE_UNPLUGGED) return L"Unplugged";
    return L"Unknown";
}

std::wstring ReadFriendlyName(IMMDevice* device) {
    ComPtr<IPropertyStore> properties;
    if (FAILED(device->OpenPropertyStore(STGM_READ, &properties))) return L"(Unknown audio device)";
    PROPVARIANT value;
    PropVariantInit(&value);
    std::wstring result = L"(Unnamed audio device)";
    if (SUCCEEDED(properties->GetValue(PKEY_Device_FriendlyName, &value)) &&
        value.vt == VT_LPWSTR && value.pwszVal != nullptr) result = value.pwszVal;
    PropVariantClear(&value);
    return result;
}

bool OpenEndpointVolume(const std::wstring& endpointId, ComPtr<IAudioEndpointVolume>& volume) {
    if (endpointId.empty()) return false;
    ComPtr<IMMDeviceEnumerator> enumerator;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                IID_PPV_ARGS(&enumerator)))) return false;
    ComPtr<IMMDevice> device;
    if (FAILED(enumerator->GetDevice(endpointId.c_str(), &device))) return false;
    return SUCCEEDED(device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL,
                                      nullptr, reinterpret_cast<void**>(volume.GetAddressOf())));
}
}

std::vector<AudioEndpoint> EnumerateAudioEndpoints() {
    std::vector<AudioEndpoint> endpoints;
    ComPtr<IMMDeviceEnumerator> enumerator;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                IID_PPV_ARGS(&enumerator)))) return endpoints;
    const EDataFlow flows[] = { eRender, eCapture };
    const wchar_t* labels[] = { L"Playback", L"Recording" };
    for (int index = 0; index < 2; ++index) {
        ComPtr<IMMDeviceCollection> collection;
        if (FAILED(enumerator->EnumAudioEndpoints(flows[index], DEVICE_STATEMASK_ALL, &collection))) continue;
        UINT count = 0;
        if (FAILED(collection->GetCount(&count))) continue;
        for (UINT item = 0; item < count; ++item) {
            ComPtr<IMMDevice> device;
            if (FAILED(collection->Item(item, &device))) continue;
            LPWSTR rawId = nullptr;
            std::wstring id;
            if (SUCCEEDED(device->GetId(&rawId)) && rawId != nullptr) {
                id = rawId;
                CoTaskMemFree(rawId);
            }
            DWORD state = 0;
            device->GetState(&state);
            endpoints.push_back({ ReadFriendlyName(device.Get()), id, labels[index], ReadState(state) });
        }
    }
    return endpoints;
}

bool GetEndpointVolume(const std::wstring& endpointId, float& level) {
    ComPtr<IAudioEndpointVolume> volume;
    return OpenEndpointVolume(endpointId, volume) &&
           SUCCEEDED(volume->GetMasterVolumeLevelScalar(&level));
}

bool SetEndpointVolume(const std::wstring& endpointId, float level) {
    ComPtr<IAudioEndpointVolume> volume;
    if (!OpenEndpointVolume(endpointId, volume)) return false;
    if (level < 0.0f) level = 0.0f;
    if (level > 1.0f) level = 1.0f;
    return SUCCEEDED(volume->SetMasterVolumeLevelScalar(level, nullptr));
}

bool GetEndpointMute(const std::wstring& endpointId, bool& muted) {
    ComPtr<IAudioEndpointVolume> volume;
    BOOL value = FALSE;
    if (!OpenEndpointVolume(endpointId, volume) || FAILED(volume->GetMute(&value))) return false;
    muted = value != FALSE;
    return true;
}

bool SetEndpointMute(const std::wstring& endpointId, bool muted) {
    ComPtr<IAudioEndpointVolume> volume;
    return OpenEndpointVolume(endpointId, volume) &&
           SUCCEEDED(volume->SetMute(muted ? TRUE : FALSE, nullptr));
}
