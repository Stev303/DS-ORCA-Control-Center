# DS ORCA Control Center

A Windows 10/11 x64 control application for the DS ORCA MK2 USB audio device.

## License

The original DS ORCA Control Center project source is licensed under GNU GPL version 3. See [COPYING](COPYING).

The Steinberg ASIO SDK is a separate third-party component and is not bundled here. The project plans to use the SDK's GPLv3 alternative; preserve all SDK-specific notices and check file-level licensing before integrating SDK files. See [Third-party notices](docs/THIRD_PARTY_NOTICES.md).

## Current status — 0.2.0

Implemented and built:
- Native Windows desktop application.
- Windows Core Audio playback and recording endpoint enumeration.
- Active input and output endpoint selection.
- Input endpoint master volume and mute controls.
- Playback endpoint master volume and mute controls.
- Automated Windows x64 Release build.

The volume and mute controls operate on Windows Core Audio endpoint controls. They do not yet control undocumented DSP inside the DS ORCA hardware and do not constitute a multi-bus software mixer.

**Not implemented yet:** WASAPI audio streaming/mixing, ASIO driver, virtual audio endpoint, DAW integration and installer.

## Download the latest build

1. Open the [Windows x64 build workflow](https://github.com/Stev303/DS-ORCA-Control-Center/actions/workflows/windows-build.yml).
2. Open the latest successful run.
3. Download the artifact named `DS-ORCA-Control-Center-Windows-x64`.

## Hardware compatibility

The application uses the Microsoft USB Audio class driver already installed by Windows. It must not replace the DS ORCA device driver.

Known DS ORCA USB identity: VID 320C, PID 1B0A. Match by USB identity and endpoint information rather than assuming every device named HD USB Audio Device is DS ORCA.

The supplied USB descriptors show a USB Audio Class 1 device with standard mute/volume controls and a vendor extension whose behavior is not yet verified. A software mixer UI does not automatically control hardware DSP.

A production ASIO driver must expose real input/output buffers backed by the DS ORCA audio endpoints. A virtual audio driver is a separate Windows driver project requiring WDK build, signing and installation tests.

## Validation gate

Do not label a build production-ready until real DS ORCA MK2 hardware has been tested for playback, capture, device reconnect, sample-rate changes and Studio One / FL Studio operation.
