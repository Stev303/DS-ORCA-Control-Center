# Engineering roadmap
## Milestone 0.1 — Windows application foundation
- [x] Native Windows x64 app shell
- [x] Enumerate playback and capture endpoints
- [x] Automated Windows build
- [ ] Verify artifact on a Windows 11 PC

## Milestone 0.2 — Device identification
- [ ] Match DS ORCA using USB VID 320C / PID 1B0A
- [ ] Show selected capture and playback endpoints
- [ ] Detect device arrival/removal

## Milestone 0.3 — Audio engine
- [ ] WASAPI shared-mode capture/playback
- [ ] Stable device format negotiation
- [ ] Buffer and latency diagnostics
- [ ] Loopback capture

## Milestone 0.4 — Mixer and routing
- [ ] Input/output meters
- [ ] Channel gain, mute and monitor
- [ ] Routing matrix
- [ ] Save/load presets

## Milestone 0.5 — DAW integration
- [ ] ASIO x64 implementation and Studio One validation
- [ ] FL Studio validation
- [ ] Sample-rate and buffer compatibility tests

## Milestone 0.6 — Virtual audio driver
- [ ] Select driver architecture and supported Windows versions
- [ ] WDK build and test-signing process
- [ ] Install/uninstall and device recovery tests
- [ ] Audio stability and latency tests

## Milestone 1.0 — Installer and release
- [ ] Signed installer package
- [ ] Clean install/uninstall tests on Windows 11
- [ ] Verify audio in/out, loopback and DAW workflows
- [ ] Publish release notes and checksums

## Release gate
Do not label a build production-ready until real DS ORCA MK2 hardware has been tested for
playback, capture, device reconnect, sample-rate changes and Studio One / FL Studio operation.
