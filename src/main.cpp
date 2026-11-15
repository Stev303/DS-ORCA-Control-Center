#include "AudioDevices.h"
#include <windows.h>
#include <commctrl.h>
#include <objbase.h>
#include <string>
#include <sstream>
#include <vector>

namespace {
constexpr int kRefreshButton = 1001;
constexpr int kInputCombo = 1002;
constexpr int kOutputCombo = 1003;
constexpr int kInputSlider = 1004;
constexpr int kOutputSlider = 1005;
constexpr int kInputMute = 1006;
constexpr int kOutputMute = 1007;

HWND g_deviceList = nullptr;
HWND g_inputCombo = nullptr;
HWND g_outputCombo = nullptr;
HWND g_inputSlider = nullptr;
HWND g_outputSlider = nullptr;
HWND g_inputMute = nullptr;
HWND g_outputMute = nullptr;
HWND g_status = nullptr;
std::vector<AudioEndpoint> g_inputs;
std::vector<AudioEndpoint> g_outputs;

const AudioEndpoint* SelectedEndpoint(HWND combo, const std::vector<AudioEndpoint>& endpoints) {
    const LRESULT index = SendMessageW(combo, CB_GETCURSEL, 0, 0);
    if (index == CB_ERR || static_cast<size_t>(index) >= endpoints.size()) return nullptr;
    return &endpoints[static_cast<size_t>(index)];
}

void SetStatus(const std::wstring& message) {
    if (g_status) SetWindowTextW(g_status, message.c_str());
}

void PopulateEndpointCombo(HWND combo, const std::vector<AudioEndpoint>& endpoints) {
    SendMessageW(combo, CB_RESETCONTENT, 0, 0);
    for (const auto& endpoint : endpoints)
        SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(endpoint.name.c_str()));
    if (!endpoints.empty()) SendMessageW(combo, CB_SETCURSEL, 0, 0);
}

void UpdateEndpointControls(bool input) {
    HWND combo = input ? g_inputCombo : g_outputCombo;
    HWND slider = input ? g_inputSlider : g_outputSlider;
    HWND muteButton = input ? g_inputMute : g_outputMute;
    const auto& endpoints = input ? g_inputs : g_outputs;
    const AudioEndpoint* endpoint = SelectedEndpoint(combo, endpoints);
    if (!endpoint) {
        EnableWindow(slider, FALSE);
        EnableWindow(muteButton, FALSE);
        return;
    }

    float level = 0.0f;
    bool muted = false;
    if (GetEndpointVolume(endpoint->id, level)) {
        EnableWindow(slider, TRUE);
        SendMessageW(slider, TBM_SETPOS, TRUE, static_cast<LPARAM>(level * 100.0f + 0.5f));
    } else {
        EnableWindow(slider, FALSE);
    }
    if (GetEndpointMute(endpoint->id, muted)) {
        EnableWindow(muteButton, TRUE);
        SetWindowTextW(muteButton, muted ? L"Unmute" : L"Mute");
    } else {
        EnableWindow(muteButton, FALSE);
    }
}

void RefreshDevices() {
    if (!g_deviceList) return;
    g_inputs.clear();
    g_outputs.clear();
    std::wostringstream output;
    output << L"DS ORCA CONTROL CENTER | WINDOWS AUDIO ENDPOINTS\r\n";
    output << L"================================================\r\n";
    const auto devices = EnumerateAudioEndpoints();
    for (const auto& device : devices) {
        output << L"[" << device.flow << L"] " << device.name << L"\r\n";
        output << L"    Status: " << device.state << L"\r\n";
        output << L"    ID: " << device.id << L"\r\n\r\n";
        if (device.flow == L"Recording" && device.state == L"Active") g_inputs.push_back(device);
        if (device.flow == L"Playback" && device.state == L"Active") g_outputs.push_back(device);
    }
    if (devices.empty()) output << L"No audio endpoints found. Check Windows Audio service.\r\n";
    SetWindowTextW(g_deviceList, output.str().c_str());
    PopulateEndpointCombo(g_inputCombo, g_inputs);
    PopulateEndpointCombo(g_outputCombo, g_outputs);
    UpdateEndpointControls(true);
    UpdateEndpointControls(false);
    SetStatus(L"Endpoint controls use Windows Core Audio volume/mute. No undocumented hardware DSP is assumed.");
}

void ApplySlider(bool input) {
    HWND combo = input ? g_inputCombo : g_outputCombo;
    HWND slider = input ? g_inputSlider : g_outputSlider;
    const auto& endpoints = input ? g_inputs : g_outputs;
    const AudioEndpoint* endpoint = SelectedEndpoint(combo, endpoints);
    if (!endpoint) return;
    const int position = static_cast<int>(SendMessageW(slider, TBM_GETPOS, 0, 0));
    if (SetEndpointVolume(endpoint->id, static_cast<float>(position) / 100.0f))
        SetStatus((input ? L"Input level: " : L"Output level: ") + std::to_wstring(position) + L"%");
    else
        SetStatus(L"Windows did not allow changing this endpoint volume.");
}

void ToggleMute(bool input) {
    HWND combo = input ? g_inputCombo : g_outputCombo;
    const auto& endpoints = input ? g_inputs : g_outputs;
    const AudioEndpoint* endpoint = SelectedEndpoint(combo, endpoints);
    if (!endpoint) return;
    bool muted = false;
    if (!GetEndpointMute(endpoint->id, muted) || !SetEndpointMute(endpoint->id, !muted)) {
        SetStatus(L"Unable to change mute state for this endpoint.");
        return;
    }
    UpdateEndpointControls(input);
    SetStatus(input ? L"Recording endpoint mute updated." : L"Playback endpoint mute updated.");
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateWindowW(L"STATIC", L"DS ORCA | CONTROL CENTER", WS_CHILD | WS_VISIBLE, 24, 16, 620, 30, window, nullptr, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"Refresh devices", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 24, 54, 150, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kRefreshButton)), nullptr, nullptr);

        CreateWindowW(L"STATIC", L"RECORDING INPUT", WS_CHILD | WS_VISIBLE, 24, 100, 180, 22, window, nullptr, nullptr, nullptr);
        g_inputCombo = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 24, 124, 380, 220, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kInputCombo)), nullptr, nullptr);
        g_inputSlider = CreateWindowW(TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS, 24, 158, 300, 34, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kInputSlider)), nullptr, nullptr);
        SendMessageW(g_inputSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
        g_inputMute = CreateWindowW(L"BUTTON", L"Mute", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 340, 158, 90, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kInputMute)), nullptr, nullptr);

        CreateWindowW(L"STATIC", L"PLAYBACK OUTPUT", WS_CHILD | WS_VISIBLE, 470, 100, 180, 22, window, nullptr, nullptr, nullptr);
        g_outputCombo = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 470, 124, 380, 220, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kOutputCombo)), nullptr, nullptr);
        g_outputSlider = CreateWindowW(TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS, 470, 158, 300, 34, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kOutputSlider)), nullptr, nullptr);
        SendMessageW(g_outputSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
        g_outputMute = CreateWindowW(L"BUTTON", L"Mute", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 786, 158, 90, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kOutputMute)), nullptr, nullptr);

        g_status = CreateWindowW(L"STATIC", L"Loading endpoints...", WS_CHILD | WS_VISIBLE, 24, 212, 840, 42, window, nullptr, nullptr, nullptr);
        g_deviceList = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 24, 264, 840, 340, window, nullptr, nullptr, nullptr);
        RefreshDevices();
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case kRefreshButton: RefreshDevices(); return 0;
        case kInputMute: ToggleMute(true); return 0;
        case kOutputMute: ToggleMute(false); return 0;
        case kInputCombo:
            if (HIWORD(wParam) == CBN_SELCHANGE) UpdateEndpointControls(true);
            return 0;
        case kOutputCombo:
            if (HIWORD(wParam) == CBN_SELCHANGE) UpdateEndpointControls(false);
            return 0;
        }
        break;

    case WM_HSCROLL:
        if (reinterpret_cast<HWND>(lParam) == g_inputSlider) ApplySlider(true);
        if (reinterpret_cast<HWND>(lParam) == g_outputSlider) ApplySlider(false);
        return 0;

    case WM_SIZE:
        if (g_deviceList) {
            const int width = LOWORD(lParam), height = HIWORD(lParam);
            MoveWindow(g_deviceList, 24, 264, width > 48 ? width - 48 : 300, height > 288 ? height - 288 : 180, TRUE);
            MoveWindow(g_outputCombo, width > 900 ? width - 450 : 470, 124, 380, 220, TRUE);
            MoveWindow(g_outputSlider, width > 900 ? width - 450 : 470, 158, 300, 34, TRUE);
            MoveWindow(g_outputMute, width > 900 ? width - 134 : 786, 158, 90, 30, TRUE);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    const HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(comResult) && comResult != RPC_E_CHANGED_MODE) {
        MessageBoxW(nullptr, L"Unable to initialize Windows audio services.", L"DS ORCA Control Center", MB_ICONERROR);
        return 1;
    }

    INITCOMMONCONTROLSEX controls{ sizeof(INITCOMMONCONTROLSEX), ICC_BAR_CLASSES };
    InitCommonControlsEx(&controls);

    const wchar_t className[] = L"DSOrcaControlCenterWindow";
    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&windowClass)) {
        if (SUCCEEDED(comResult)) CoUninitialize();
        return 1;
    }

    HWND window = CreateWindowExW(0, className, L"DS ORCA Control Center", WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT, CW_USEDEFAULT, 930, 700,
                                  nullptr, nullptr, instance, nullptr);
    if (!window) {
        if (SUCCEEDED(comResult)) CoUninitialize();
        return 1;
    }

    ShowWindow(window, showCommand);
    UpdateWindow(window);
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    if (SUCCEEDED(comResult)) CoUninitialize();
    return static_cast<int>(message.wParam);
}
