#include "AudioDevices.h"
#include <windows.h>
#include <mmdeviceapi.h>
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
}
std::vector<AudioEndpoint> EnumerateAudioEndpoints() {
    std::vector<AudioEndpoint> endpoints;
    ComPtr<IMMDeviceEnumerator> enumerator;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&enumerator)))) return endpoints;
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
