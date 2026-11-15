# DS ORCA Control Center
A Windows 10/11 x64 control application for the DS ORCA MK2 USB audio device.

## Current status — foundation 0.1.0
- Native Windows desktop application.
- Enumerates Windows playback and recording endpoints using Windows Core Audio.
- Displays endpoint names, IDs, direction and state.
- GitHub Actions builds the x64 Release executable automatically.

This is the first engineering milestone, not yet a finished audio mixer or ASIO driver.

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
4. Virtual audio endpoint driver.
5. ASIO x64 integration for Studio One and FL Studio.
6. Presets, device dashboard and installer.

## Hardware compatibility
The application initially uses the Microsoft USB Audio class driver already installed by Windows.
It must not replace the DS ORCA device driver.

Known DS ORCA USB identity: VID 320C, PID 1B0A. Match by USB identity and endpoint
information rather than assuming every device named HD USB Audio Device is DS ORCA.

A software mixer UI does not automatically control hardware DSP. Hardware controls must be
confirmed against the device USB control interface.

A production virtual audio driver requires a Windows Driver Kit build, installation and
signing plan. ASIO integration must be validated in the target DAWs. No driver is represented
as production-ready until it has passed audio and installation tests.
