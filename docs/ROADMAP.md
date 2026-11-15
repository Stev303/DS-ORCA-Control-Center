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

## Milestone 0.3 — WASAPI monitor (working baseline)
- [x] Shared-mode capture and playback streams
- [x] Negotiate endpoint mix formats
- [x] Event-driven capture and render
- [x] User-selectable requested buffer duration: 10 / 20 / 40 / 80 ms
- [x] Confirm all four buffer choices start on the user's DS ORCA hardware
- [ ] Automatic recovery after device removal
- [ ] Loopback capture
- [ ] Input/output peak meters and clipping indication
- [ ] Long-duration stability and drift testing

## Milestone 0.4 — Software mixer and routing
- [x] Stereo mixer DSP core for up to eight strips
- [x] Per-strip gain, pan/balance, mute and solo in the DSP core
- [x] Master gain in the DSP core
- [ ] Connect mixer DSP to live WASAPI streams
- [ ] Add real-time-safe parameter exchange
- [ ] Add input/output peak meters
- [ ] Add routing matrix and feedback protection
- [ ] Save/load presets

## Milestone 0.5 — Native ASIO x64 driver
- [x] Obtain and pin official Steinberg ASIO SDK 2.3.4 (2025-10-15); GPLv3 option confirmed in SDK LICENSE.txt
- [x] Audit SDK notices: common ASIO interface headers refer to the SDK GPLv3 option; sample driver files carry separate BSD-style notices and are not being copied as implementation
- [ ] Implement ASIO COM driver DLL, class factory and registration/unregistration
- [ ] Implement hardware-backed capture and playback
- [ ] Negotiate sample rates, channel layout and ASIO buffer sizes
- [ ] Report measured/defensible input and output latency
- [ ] Handle device removal, stream errors and clean shutdown
- [ ] Validate in Studio One and FL Studio using the physical DS ORCA

## Milestone 0.6 — Optional virtual audio endpoint
- [ ] Decide whether a virtual endpoint is actually needed
- [ ] Select architecture and supported Windows versions
- [ ] WDK build and test-signing process
- [ ] Install/uninstall and device recovery tests
- [ ] Audio stability and latency tests

## Milestone 1.0 — Installer and validation
- [ ] Package app and ASIO driver registration safely
- [ ] Provide clean uninstall and rollback
- [ ] Verify install on Windows 10 and Windows 11
- [ ] Verify playback, capture, reconnect, sample rates and DAW operation
- [ ] Publish checksums and corresponding source for distributed binaries

## Release gate

The current WASAPI monitor is a working baseline, not an ASIO driver. Do not label a build production-ready until real DS ORCA MK2 hardware has been tested for playback, capture, reconnect, sample-rate changes, sustained operation and Studio One / FL Studio operation.
