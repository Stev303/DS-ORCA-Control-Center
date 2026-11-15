# ASIO SDK 2.3.4 receipt and license audit

## Supplied archive

- Archive: `ASIO-SDK_2.3.4_2025-10-15.zip`
- Version: 2.3.4
- Release date in archive name: 2025-10-15
- Archive was inspected locally; it contains the `ASIOSDK/common` interface headers and the Windows sample driver.

## License findings

The top-level `ASIOSDK/LICENSE.txt` states that the SDK may be used under either the Steinberg ASIO License or GPL version 3. The project has selected the GPLv3 option.

The common interface files `common/asiosys.h`, `common/asio.h`, and `common/iasiodrv.h` explicitly refer to the SDK license file. These are the interface definitions required by a native ASIO driver.

The sample driver's `driver/asiosample/asiosmpl.h` and `asiosmpl.cpp` contain a separate BSD-style license notice. The sample implementation is a test driver (its own source describes simulated input and no real audio output), so it will not be copied as the DS ORCA implementation.

Some other SDK support files contain separate Microsoft copyright notices. They will not be copied unless their terms are individually reviewed and their use is necessary.

## Integration policy

1. Use the official ASIO interface headers and preserve the SDK license notices.
2. Implement the DS ORCA COM class, class factory, registration, and audio backend as project code rather than reusing the sample driver's test implementation.
3. Keep the SDK archive and unrelated sample projects out of the repository.
4. Do not register or expose an ASIO driver until it can actually open the DS ORCA capture and playback endpoints and pass audio.
5. The current WASAPI monitor remains the working fallback while native ASIO is implemented and tested.

## Current status

The SDK has been received and its licensing route and relevant interface files have been audited. This is not yet an ASIO driver build. COM implementation, exclusive-mode backend, callback/buffer handling, Windows x64 build, and DAW hardware validation remain outstanding.
