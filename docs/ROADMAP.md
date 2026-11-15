# Engineering roadmap

## Milestone 0.1 — Windows application foundation
- [x] Native Windows x64 app shell
- [x] Enumerate playback and capture endpoints
- [x] Automated Windows build
- [x] Windows x64 Release artifact generated

## Milestone 0.2 — Endpoint control dashboard
- [x] List active playback and recording endpoints
- [x] Select input and output endpoints
- [x] Change Windows endpoint input/output master volume
- [x] Mute and unmute Windows input/output endpoints
- [ ] Match DS ORCA by USB VID 320C / PID 1B0A rather than friendly name alone
- [ ] Detect device arrival/removal and refresh automatically

## Milestone 0.3 — WASAPI audio engine
- [ ] Shared-mode capture and playback streams
- [ ] Negotiate endpoint mix formats safely
- [ ] Event-driven buffering and recovery after device removal
- [ ] Loopback capture
- [ ] Input/output peak meters and clipping indication

## Milestone 0.4 — Software mixer and routing
- [ ] Input and playback channel strips
- [ ] Per-channel gain, mute and monitor
- [ ] Master output level
- [ ] Routing matrix
- [ ] Save/load presets
- [ ] Verify no unintended feedback paths

## Milestone 0.5 — ASIO x64 driver
- [ ] Integrate Steinberg ASIO SDK under GPLv3-compatible file terms
- [ ] Implement ASIO COM driver and registration/unregistration
- [ ] Bind ASIO input/output channels to actual DS ORCA audio endpoints
- [ ] Implement sample-rate and buffer negotiation
- [ ] Validate in Studio One and FL Studio
- [ ] Measure underruns, latency and stability

## Milestone 0.6 — Optional virtual audio endpoint
- [ ] Select architecture and supported Windows versions
- [ ] WDK build and test-signing process
- [ ] Install/uninstall and device recovery tests
- [ ] Audio stability and latency tests

## Milestone 1.0 — Installer and validation
- [ ] Package app and ASIO driver registration safely
- [ ] Provide clean uninstall and rollback
- [ ] Verify install on Windows 10 and Windows 11
- [ ] Verify playback, capture, reconnect, sample rates and DAW operation
- [ ] Publish checksums and complete corresponding source when conveying binaries

## Release gate

Do not label a build production-ready until real DS ORCA MK2 hardware has been tested for playback, capture, device reconnect, sample-rate changes and Studio One / FL Studio operation. Version 0.2.0 is an endpoint control dashboard, not an ASIO driver or full mixer.
