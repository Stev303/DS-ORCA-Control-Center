# DS ORCA native ASIO driver design

## Goal

Expose the physical DS ORCA USB audio device to ASIO hosts such as Studio One and FL Studio through a native 64-bit Windows ASIO driver. This is distinct from the existing WASAPI monitor application.

## Hardware facts currently available

- USB VID/PID: 320C:1B0A.
- Windows enumerates the device through Microsoft's USB Audio class driver (`usbaudio.sys`).
- The device is USB Audio Class 1.
- Capture and playback are separate Windows audio endpoints.
- The supplied descriptors advertise several PCM formats and sample rates, but the exact mutually supported capture/playback combinations still need runtime probing.
- Standard USB Audio controls include mute/volume. The vendor extension's DSP semantics are unknown.

Do not replace the Microsoft USB Audio driver or assume undocumented DSP controls exist.

## Proposed architecture

```
Studio One / FL Studio
        |
        | ASIO COM interface (IASIO)
        v
DS ORCA ASIO x64 DLL
        |
        | real-time double buffers + ASIO callbacks
        v
DS ORCA audio backend
        |
        | candidate: WASAPI exclusive mode first;
        | evaluate Kernel Streaming only if needed
        v
Windows USB Audio class driver (usbaudio.sys)
        |
        v
DS ORCA USB hardware
```

The desktop control application remains separate. It may provide the driver's control panel and shared configuration, but the ASIO audio callback must not depend on UI message handling.

## Implementation sequence

1. Obtain the current SDK from Steinberg's official ASIO SDK download page: https://www.steinberg.net/asiosdk
2. Pin the SDK release and source revision used by the build. Keep SDK files in a clearly separated third-party directory and preserve every file-level license notice.
3. Build the SDK's Windows sample driver unchanged as a reference. This validates the toolchain, COM registration model and host discovery before custom code is introduced.
4. Implement the driver COM class and registration/unregistration using the SDK's Windows driver patterns.
5. Implement a hardware backend that opens the actual DS ORCA endpoints. Start by testing WASAPI exclusive mode because it uses the existing Microsoft class driver without installing a replacement kernel driver.
6. Add ASIO channel descriptions, supported sample rates, buffer constraints, sample position, time info and defensible latency values.
7. Implement ASIO double-buffer conversion and callback scheduling. No heap allocation, blocking locks, UI calls or unbounded work in the real-time callback.
8. Add device-loss handling and clean stop/dispose paths.
9. Test first with a diagnostic ASIO host, then Studio One and FL Studio on the physical DS ORCA.
10. Only after hardware tests, package the DLL and safe registration/unregistration into an installer.

## Important design constraints

- An ASIO buffer size in samples is not the same thing as the existing WASAPI monitor's requested shared-mode buffer duration in milliseconds.
- Do not advertise 64/128/256-sample operation until the hardware backend can sustain it without underruns.
- Do not report zero latency or infer latency from requested buffer sizes. Measure or calculate from actual backend buffering and endpoint behavior.
- Capture and playback devices may expose different mix formats or clocks. If they cannot run synchronously, the backend needs clock-drift handling or a documented limitation.
- Do not silently resample or change hardware formats without reporting the actual negotiated format.
- Studio One and FL Studio must both enumerate and open the driver, start/stop it, record input and play output.
- Keep the existing WASAPI monitor as a fallback while the ASIO driver is under development.

## Licensing

This project has selected the GPLv3 open-source option. Steinberg's developer portal identifies an open-source GNU license for ASIO, and Steinberg's October 2025 announcement specifies GPLv3. The SDK's own top-level and per-directory/file notices remain authoritative for the files they cover. Do not assume every file in the SDK has identical terms.
