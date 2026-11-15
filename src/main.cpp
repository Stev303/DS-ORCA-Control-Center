#include "AudioDevices.h"
#include <windows.h>
#include <objbase.h>
#include <string>
#include <sstream>
namespace {
constexpr int kRefreshButton = 1001;
HWND g_deviceList = nullptr;
void RefreshDevices() {
    if (!g_deviceList) return;
    std::wostringstream output;
    output << L"DS ORCA CONTROL CENTER | DEVICE DASHBOARD\r\n";
    output << L"==============================================\r\n";
    output << L"Windows audio endpoints detected:\r\n\r\n";
    const auto devices = EnumerateAudioEndpoints();
    if (devices.empty()) output << L"No audio endpoints could be enumerated. Check Windows Audio service.\r\n";
    for (const auto& device : devices) {
        output << L"[" << device.flow << L"] " << device.name << L"\r\n";
        output << L"    Status: " << device.state << L"\r\n";
        output << L"    ID: " << device.id << L"\r\n\r\n";
    }
    output << L"\r\nNEXT: audio stream engine, routing, mixer and ASIO integration.";
    SetWindowTextW(g_deviceList, output.str().c_str());
}
LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateWindowW(L"STATIC", L"DS ORCA | CONTROL CENTER", WS_CHILD | WS_VISIBLE, 24, 18, 620, 34, window, nullptr, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"Refresh audio devices", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 24, 62, 190, 34, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kRefreshButton)), nullptr, nullptr);
        g_deviceList = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Loading audio devices...", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 24, 112, 790, 420, window, nullptr, nullptr, nullptr);
        RefreshDevices();
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == kRefreshButton) { RefreshDevices(); return 0; }
        break;
    case WM_SIZE:
        if (g_deviceList) {
            const int width = LOWORD(lParam), height = HIWORD(lParam);
            MoveWindow(g_deviceList, 24, 112, width > 48 ? width - 48 : 300, height > 136 ? height - 136 : 200, TRUE);
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
    const wchar_t className[] = L"DSOrcaControlCenterWindow";
    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&windowClass)) { if (SUCCEEDED(comResult)) CoUninitialize(); return 1; }
    HWND window = CreateWindowExW(0, className, L"DS ORCA Control Center", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 870, 620, nullptr, nullptr, instance, nullptr);
    if (!window) { if (SUCCEEDED(comResult)) CoUninitialize(); return 1; }
    ShowWindow(window, showCommand);
    UpdateWindow(window);
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) { TranslateMessage(&message); DispatchMessageW(&message); }
    if (SUCCEEDED(comResult)) CoUninitialize();
    return static_cast<int>(message.wParam);
}
