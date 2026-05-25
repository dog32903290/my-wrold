# P-LIVE16 OSC Receive Spine Closure

Date: 2026-05-25 15:00 Asia/Taipei.

## Question

Can the native core receive a controlled OSC float packet and expose it as a `LiveIOValueFrame` without entering realtime callback delivery?

## Contract

Input:

- UDP OSC float datagram
- receiver config host/port/address/value id

Output:

- decoded OSC address/value packet
- `LiveIOValueFrame` value for matching configured address

Rules:

- Receiver has explicit open/poll/close lifecycle.
- `pollOnce()` is controlled and timeout-bound.
- Non-matching OSC address produces an empty value frame.
- No audio/MIDI realtime callback delivery is introduced.
- No UI/server lifecycle is introduced.

## Proven Flow

```text
OSC float datagram
-> controlled UDP receiver poll
-> decoded address/value
-> LiveIOValueFrame for graph/control use
```

## Evidence

- `source/core/LiveIOOscReceiver.h`
- `source/core/LiveIOOscReceiver.cpp`
- `tests/LiveIOOscReceiverTests.cpp`
- `CMakeLists.txt`
- `docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md`

## Verification

RED:

- `cmake -S . -B build && cmake --build build --target my_world_live_io_osc_receiver_tests` failed first because `source/core/LiveIOOscReceiver.cpp` did not exist.

GREEN:

- `cmake -S . -B build && cmake --build build --target my_world_live_io_osc_receiver_tests && ./build/my_world_live_io_osc_receiver_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "live_io_osc_receiver|live_io_send_adapter"`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:

- `live io osc receiver ok`
- app target `my-world` builds.
- focused OSC receiver/send adapter tests passed 2/2.
- full `ctest` passed 74/74.
- `git diff --check` passed.

## Parked

- UI/server lifecycle.
- Realtime callback delivery.
- Broader MIDI output operators.
