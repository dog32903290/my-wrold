# P-LIVE8 Live UI Indicator Closure

Date: 2026-05-25 13:51 Asia/Taipei.

## Question

Can the app show a live IO indicator driven by the real app-timer `LiveIOControlTimerState`, without making UI code invent state, perform sends, or enter the realtime audio callback?

## Contract

Input:

- `LiveIOControlTimerState`
- `LiveIOControlTimerSendMode`

Output:

- `LiveIOStatusIndicatorState`
- compact display text
- detail message
- mode string
- status string
- tone token
- MIDI/OSC count pair for the selected send mode
- sample counter

Rules:

- Dry-run mode reports dry-run MIDI/OSC counts.
- Controlled-send mode reports controlled-send MIDI/OSC counts.
- The app indicator text is derived from `LiveIOStatusIndicatorState`, not manually reconstructed in UI code.
- UI color is a view of the indicator tone only.
- This slice does not add send-mode preferences, MIDI teach, external OSC targets, or realtime callback delivery.

## Proven Flow

```text
MainComponent::timerCallback()
-> updateAudioMeters()
-> tickLiveIOControl()
-> LiveIOControlTimerState
-> makeLiveIOStatusIndicatorState()
-> liveIOStatusLabel text/tone
```

## Evidence

- `source/core/LiveIOStatusIndicator.h`
- `source/core/LiveIOStatusIndicator.cpp`
- `tests/LiveIOStatusIndicatorTests.cpp`
- `source/app/MainComponent.h`
- `source/app/MainComponent.cpp`
- `CMakeLists.txt`

## Verification

RED:

- `cmake --build build --target my_world_live_io_status_indicator_tests` failed before CMake regeneration because the new target did not exist.
- `cmake -S . -B build && cmake --build build --target my_world_live_io_status_indicator_tests` failed because `LiveIOStatusIndicator.h` did not exist.

GREEN:

- `cmake --build build --target my_world_live_io_status_indicator_tests my-world && ./build/my_world_live_io_status_indicator_tests`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:

- `live io status indicator ok`
- app target `my-world` builds.
- `71/71 tests passed`
- `git diff --check passed`

## Parked

- Send mode preferences.
- MIDI teach.
- External OSC targets and always-on OSC receive nodes/server.
- Realtime callback delivery.
- Broader MIDI output operators.
