# P-LIVE15 External OSC Target Send Proof Closure

Date: 2026-05-25 14:53 Asia/Taipei.

## Question

Can the controlled app timer route an OSC float to an injected external target sender with host/port/address evidence, without adding an always-on server or realtime callback delivery?

## Contract

Input:

- `PerformancePreferences.liveIO.oscHost`
- `PerformancePreferences.liveIO.oscPort`
- `PerformancePreferences.liveIO.oscLoudnessAddress`
- injected `LiveIOOscFloatSender`
- analyzer snapshot

Output:

- `LiveIOOscFloatMessage` containing binding id, host, port, address, and float value
- controlled app timer proof that the injected sender is called once

Rules:

- OSC controlled send is enabled only when a sender is explicitly injected.
- The app UI path still does not open an external OSC sender by default.
- No always-on OSC receive server is introduced.
- No audio/MIDI realtime callback file IO, socket IO, allocation, or device scan is introduced.

## Proven Flow

```text
PerformancePreferences OSC target
-> LiveIOAppController controlled timer config
-> LiveIOControlTimer / Pump / Dispatcher
-> injected OSC sender receives host/port/address/value
```

## Evidence

- `source/core/LiveIOControlDispatcher.h`
- `source/core/LiveIOControlDispatcher.cpp`
- `source/core/LiveIOControlPump.h`
- `source/core/LiveIOControlPump.cpp`
- `source/core/LiveIOControlTimer.h`
- `source/core/LiveIOControlTimer.cpp`
- `source/app/LiveIOAppController.h`
- `source/app/LiveIOAppController.cpp`
- `tests/LiveIOAppControllerTests.cpp`

## Verification

RED:

- `cmake --build build --target my_world_live_io_app_controller_tests` failed first because `LiveIOAppTimerRequest` had no `oscSender`, and `LiveIOOscFloatMessage` had no host/port fields.

GREEN:

- `cmake --build build --target my_world_live_io_app_controller_tests && ./build/my_world_live_io_app_controller_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "live_io_app_controller|live_io_control_dispatcher|live_io_control_pump|live_io_control_timer"`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:

- `live io app controller ok`
- app target `my-world` builds.
- focused live IO control tests passed 4/4.
- full `ctest` passed 73/73.
- `git diff --check` passed.

## Parked

- Always-on OSC receive nodes/server.
- Realtime callback delivery.
- Broader MIDI output operators.
