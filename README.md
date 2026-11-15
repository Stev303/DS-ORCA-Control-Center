# DS ORCA Control Center

A Windows 10/11 x64 control application for the DS ORCA MK2 USB audio device.

## License

The original DS ORCA Control Center project source is licensed under GNU GPL version 3. See [COPYING](COPYING).

The Steinberg ASIO SDK is a separate third-party component and is not bundled here. The project plans to use the SDK's GPLv3 alternative; preserve all SDK-specific notices and check file-level licensing before integrating SDK files. See [Third-party notices](docs/THIRD_PARTY_NOTICES.md).

## Current status — foundation 0.1.0

- Native Windows desktop application.
- Enumerates Windows playback and recording endpoints using Windows Core Audio.
- Displays endpoint names, IDs, direction and state.
- GitHub Actions builds the x64 Release executable automatically.

This is an engineering foundation, not yet a finished audio mixer or ASIO driver. The current executable does not provide audio processing, ASIO registration, or a virtual audio endpoint.

## Build without installing Visual Studio locally

1. Open the Actions tab in this repository.
2. Select Windows x64 build.
3. Select Run workflow, or push a commit to main.
4. When successful, download the artifact named DS-ORCA-Control-Center-Windows-x64.

The GitHub-hosted Windows runner provides the MSVC build environment.

## Planned modules

1. WASAPI capture and playback engine.
2. Audio routing and software mixer.
3. Windows loopback capture.
4. ASIO x64 driver backed by the Windows audio engine.
5. Optional virtual audio endpoint driver.
6. Presets, device dashboard and installer.

## Hardware compatibility

The application initially uses the Microsoft USB Audio class driver already installed by Windows. It must not replace the DS ORCA device driver.

Known DS ORCA USB identity: VID 320C, PID 1B0A. Match by USB identity and endpoint information rather than assuming every device named HD USB Audio Device is DS ORCA.

The supplied USB descriptors show a USB Audio Class 1 device with standard mute/volume controls and a vendor extension whose behavior is not yet verified. A software mixer UI does not automatically control hardware DSP.

A production ASIO driver must expose real input/output buffers backed by the DS ORCA audio endpoints. A virtual audio driver is a separate Windows driver project requiring WDK build, signing and installation tests.

## Validation gate

Do not label a build production-ready until real DS ORCA MK2 hardware has been tested for playback, capture, device reconnect, sample-rate changes and Studio One / FL Studio operation.
