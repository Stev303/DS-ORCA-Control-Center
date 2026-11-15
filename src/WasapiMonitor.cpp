#include "WasapiMonitor.h"
#include <windows.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <ksmedia.h>
#include <wrl/client.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace {
bool IsFloat(const WAVEFORMATEX* f) {
    if (f->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) return true;
    return f->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
        reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(f)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
}
bool IsPcm(const WAVEFORMATEX* f) {
    if (f->wFormatTag == WAVE_FORMAT_PCM) return true;
    return f->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
        reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(f)->SubFormat == KSDATAFORMAT_SUBTYPE_PCM;
}
bool Supported(const WAVEFORMATEX* f) {
    return f && f->nChannels >= 1 && f->nChannels <= 8 &&
        ((IsFloat(f) && f->wBitsPerSample == 32) ||
         (IsPcm(f) && (f->wBitsPerSample == 16 || f->wBitsPerSample == 24 || f->wBitsPerSample == 32)));
}
float ReadSample(const BYTE* p, const WAVEFORMATEX* f) {
    if (IsFloat(f)) {
        float v; memcpy(&v, p, sizeof(v)); return std::clamp(v, -1.0f, 1.0f);
    }
    if (f->wBitsPerSample == 16) {
        int16_t v; memcpy(&v, p, sizeof(v)); return static_cast<float>(v) / 32768.0f;
    }
    if (f->wBitsPerSample == 24) {
        int32_t v = static_cast<int32_t>(p[0] | (p[1] << 8) | (p[2] << 16));
        if (v & 0x800000) v |= ~0xFFFFFF;
        return static_cast<float>(v) / 8388608.0f;
    }
    int32_t v; memcpy(&v, p, sizeof(v)); return static_cast<float>(v) / 2147483648.0f;
}
void WriteSample(BYTE* p, const WAVEFORMATEX* f, float x) {
    x = std::clamp(x, -1.0f, 1.0f);
    if (IsFloat(f)) { memcpy(p, &x, sizeof(x)); return; }
    if (f->wBitsPerSample == 16) {
        const int16_t v = static_cast<int16_t>(std::lrint(x * 32767.0f)); memcpy(p, &v, sizeof(v)); return;
    }
    if (f->wBitsPerSample == 24) {
        const int32_t v = static_cast<int32_t>(std::lrint(x * 8388607.0f));
        p[0] = static_cast<BYTE>(v); p[1] = static_cast<BYTE>(v >> 8); p[2] = static_cast<BYTE>(v >> 16); return;
    }
    const int32_t v = static_cast<int32_t>(std::llrint(static_cast<double>(x) * 2147483647.0));
    memcpy(p, &v, sizeof(v));
}
bool OpenClient(const std::wstring& id, EDataFlow flow, ComPtr<IAudioClient>& client,
                WAVEFORMATEX*& format, HANDLE& eventHandle, ComPtr<IMMDevice>& device) {
    ComPtr<IMMDeviceEnumerator> enumerator;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&enumerator)))) return false;
    if (FAILED(enumerator->GetDevice(id.c_str(), &device))) return false;
    if (FAILED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(client.GetAddressOf())))) return false;
    if (FAILED(client->GetMixFormat(&format)) || !Supported(format)) return false;
    eventHandle = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!eventHandle) return false;
    const HRESULT hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
        1000000, 0, format, nullptr);
    if (FAILED(hr) || FAILED(client->SetEventHandle(eventHandle))) return false;
    return true;
}
}

WasapiMonitor::~WasapiMonitor() { Stop(); }

bool WasapiMonitor::Start(const std::wstring& captureId, const std::wstring& renderId) {
    if (captureId.empty() || renderId.empty() || status_.load() == 1 || status_.load() == 2) return false;
    if (worker_.joinable()) worker_.join();
    stop_.store(false);
    status_.store(1);
    worker_ = std::thread(&WasapiMonitor::Run, this, captureId, renderId);
    return true;
}
void WasapiMonitor::Stop() {
    stop_.store(true);
    if (worker_.joinable()) worker_.join();
    status_.store(0);
}

void WasapiMonitor::Run(std::wstring captureId, std::wstring renderId) {
    const HRESULT com = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(com) && com != RPC_E_CHANGED_MODE) { status_.store(3); return; }

    ComPtr<IAudioClient> captureClient, renderClient;
    ComPtr<IMMDevice> captureDevice, renderDevice;
    WAVEFORMATEX *captureFormat = nullptr, *renderFormat = nullptr;
    HANDLE captureEvent = nullptr, renderEvent = nullptr;
    ComPtr<IAudioCaptureClient> capture;
    ComPtr<IAudioRenderClient> render;
    auto cleanup = [&]() {
        if (captureClient) captureClient->Stop();
        if (renderClient) renderClient->Stop();
        if (captureEvent) CloseHandle(captureEvent);
        if (renderEvent) CloseHandle(renderEvent);
        if (captureFormat) CoTaskMemFree(captureFormat);
        if (renderFormat) CoTaskMemFree(renderFormat);
        if (SUCCEEDED(com)) CoUninitialize();
    };

    if (!OpenClient(captureId, eCapture, captureClient, captureFormat, captureEvent, captureDevice) ||
        !OpenClient(renderId, eRender, renderClient, renderFormat, renderEvent, renderDevice) ||
        captureFormat->nSamplesPerSec != renderFormat->nSamplesPerSec ||
        FAILED(captureClient->GetService(IID_PPV_ARGS(&capture))) ||
        FAILED(renderClient->GetService(IID_PPV_ARGS(&render)))) {
        cleanup(); status_.store(3); return;
    }

    UINT32 renderBufferFrames = 0;
    if (FAILED(renderClient->GetBufferSize(&renderBufferFrames)) ||
        FAILED(captureClient->Start()) || FAILED(renderClient->Start())) {
        cleanup(); status_.store(3); return;
    }

    // Fixed-size stereo FIFO: no allocations occur in the audio processing loop.
    constexpr uint64_t capacity = 96000;
    std::vector<float> fifo(static_cast<size_t>(capacity) * 2, 0.0f);
    uint64_t readFrame = 0, writeFrame = 0;
    status_.store(2);

    HANDLE events[2] = { captureEvent, renderEvent };
    while (!stop_.load()) {
        const DWORD wait = WaitForMultipleObjects(2, events, FALSE, 100);
        if (wait == WAIT_TIMEOUT) continue;
        if (wait == WAIT_FAILED) { status_.store(3); break; }

        if (wait == WAIT_OBJECT_0) {
            UINT32 packetFrames = 0;
            if (FAILED(capture->GetNextPacketSize(&packetFrames))) { status_.store(3); break; }
            while (packetFrames > 0) {
                BYTE* data = nullptr; UINT32 frames = 0; DWORD flags = 0;
                if (FAILED(capture->GetBuffer(&data, &frames, &flags, nullptr, nullptr))) { status_.store(3); break; }
                const UINT32 channels = captureFormat->nChannels;
                const UINT32 bytesPerSample = captureFormat->wBitsPerSample / 8;
                for (UINT32 i = 0; i < frames; ++i) {
                    float left = 0.0f, right = 0.0f;
                    if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT) && data) {
                        const BYTE* frame = data + static_cast<size_t>(i) * captureFormat->nBlockAlign;
                        left = ReadSample(frame, captureFormat);
                        right = channels > 1 ? ReadSample(frame + bytesPerSample, captureFormat) : left;
                    }
                    if (writeFrame - readFrame < capacity) {
                        const size_t slot = static_cast<size_t>(writeFrame % capacity) * 2;
                        fifo[slot] = left; fifo[slot + 1] = right; ++writeFrame;
                    }
                }
                capture->ReleaseBuffer(frames);
                if (FAILED(capture->GetNextPacketSize(&packetFrames))) { status_.store(3); break; }
            }
        } else if (wait == WAIT_OBJECT_0 + 1) {
            UINT32 padding = 0;
            if (FAILED(renderClient->GetCurrentPadding(&padding))) { status_.store(3); break; }
            const UINT32 available = renderBufferFrames > padding ? renderBufferFrames - padding : 0;
            if (available) {
                BYTE* output = nullptr;
                if (FAILED(render->GetBuffer(available, &output))) { status_.store(3); break; }
                const UINT32 channels = renderFormat->nChannels;
                const UINT32 bytesPerSample = renderFormat->wBitsPerSample / 8;
                for (UINT32 i = 0; i < available; ++i) {
                    float left = 0.0f, right = 0.0f;
                    if (readFrame < writeFrame) {
                        const size_t slot = static_cast<size_t>(readFrame % capacity) * 2;
                        left = fifo[slot]; right = fifo[slot + 1]; ++readFrame;
                    }
                    BYTE* frame = output + static_cast<size_t>(i) * renderFormat->nBlockAlign;
                    for (UINT32 ch = 0; ch < channels; ++ch) {
                        const float sample = ch == 0 ? left : (ch == 1 ? right : 0.5f * (left + right));
                        WriteSample(frame + static_cast<size_t>(ch) * bytesPerSample, renderFormat, sample);
                    }
                }
                if (FAILED(render->ReleaseBuffer(available, 0))) { status_.store(3); break; }
            }
        }
    }
    cleanup();
    if (status_.load() != 3) status_.store(0);
}
