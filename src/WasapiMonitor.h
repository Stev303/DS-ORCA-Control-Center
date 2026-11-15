#pragma once
#include <atomic>
#include <string>
#include <thread>

class WasapiMonitor {
public:
    WasapiMonitor() = default;
    ~WasapiMonitor();
    bool Start(const std::wstring& captureId, const std::wstring& renderId);
    void Stop();
    int Status() const noexcept { return status_.load(); } // 0 stopped, 1 starting, 2 running, 3 error
private:
    void Run(std::wstring captureId, std::wstring renderId);
    std::atomic<bool> stop_{false};
    std::atomic<int> status_{0};
    std::thread worker_;
};
