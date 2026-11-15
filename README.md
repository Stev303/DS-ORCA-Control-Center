# DS ORCA Control Center

A Windows 10/11 x64 control application and audio-engine project for the DS ORCA MK2 USB audio device.

## License

The project source is licensed under GNU GPL version 3. See [COPYING](COPYING).

The intended ASIO integration uses Steinberg's GPLv3 open-source alternative. Obtain the current SDK from the [official ASIO SDK page](https://www.steinberg.net/asiosdk). The SDK is not bundled in this repository. Preserve its file-level license notices. See [Third-party notices](docs/THIRD_PARTY_NOTICES.md).

## Current status

### Implemented and built
- Native Windows desktop application.
- Windows Core Audio playback and recording endpoint enumeration and selection.
- Windows endpoint master volume and mute controls.
- Stereo DSP mixer core for up to eight input strips, with gain, pan/balance, mute, solo and master gain.
- Event-driven WASAPI capture-to-playback monitor.
- Selectable requested shared-mode buffer duration: 10, 20, 40 and 80 ms.
- Automated Windows x64 Release build and mixer-core tests.

### Confirmed on the user's DS ORCA hardware
- The WASAPI input monitor produces audible microphone-to-headphone output.
- The user tested all four buffer selections and confirmed they all start and work.
- The user reported that latency improved after reducing the WASAPI buffer.

### Current limitations
- The WASAPI monitor is a basic input-to-output monitor, not a complete DAW audio engine.
- The buffer dropdown sets the requested shared-mode WASAPI buffer duration. It does not guarantee the same end-to-end hardware latency.
- The mixer DSP core is not yet connected to the live WASAPI stream; current endpoint sliders control Windows endpoint volume/mute.
- Input and output streams currently require matching sample rates.
- No ASIO driver is implemented or included yet. Studio One and FL Studio will not see this app as a dedicated DS ORCA ASIO device.
- No virtual audio endpoint driver or installer is implemented.
- The app does not yet control undocumented DS ORCA hardware DSP.

## Next engineering milestone: native ASIO driver

The next milestone is a real 64-bit ASIO COM driver DLL, built against the official Steinberg ASIO SDK under GPLv3. It must:
1. Implement the ASIO COM driver interface and safe registration/unregistration.
2. Open the actual DS ORCA capture and playback endpoints, identified by USB VID 320C / PID 1B0A plus endpoint identity.
3. Provide correct channel information, supported sample rates, buffer-size negotiation, sample position and latency reporting.
4. Move audio between ASIO double buffers and the hardware backend without allocations or blocking work in the real-time callback.
5. Handle device removal, format changes, start/stop and error recovery.
6. Be tested in Studio One and FL Studio on the physical DS ORCA hardware.

The first backend candidate is Windows audio in exclusive mode, subject to hardware/driver support testing. The current shared-mode WASAPI monitor is a working fallback and must remain available. An ASIO buffer setting must not be presented as true hardware latency until measured.

See [ASIO driver design](docs/ASIO_DRIVER_DESIGN.md) and [engineering roadmap](docs/ROADMAP.md).

## Build and tests

Windows 10/11 x64 with Visual Studio 2022 or later and CMake 3.24+:

```powershell
cmake -S . -B build -A x64 -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

## Download the latest build

1. Open the [Windows x64 build workflow](https://github.com/Stev303/DS-ORCA-Control-Center/actions/workflows/windows-build.yml).
2. Open the latest successful run.
3. Download the artifact named `DS-ORCA-Control-Center-Windows-x64`.

## Hardware compatibility

The application uses the Microsoft USB Audio class driver already installed by Windows. It must not replace the DS ORCA USB device driver.

Known DS ORCA USB identity: VID 320C, PID 1B0A. Match by USB identity and endpoint information rather than assuming every device named HD USB Audio Device is DS ORCA.

The supplied USB descriptors show a USB Audio Class 1 device with standard mute/volume controls and a vendor extension whose behavior is not yet verified. A software mixer UI does not automatically control hardware DSP.

A virtual audio endpoint is a separate Windows driver project requiring WDK build, signing and installation tests.

## Validation gate

Do not label a build production-ready until real DS ORCA MK2 hardware has been tested for playback, capture, reconnect, sample-rate changes, sustained operation and Studio One / FL Studio operation.
