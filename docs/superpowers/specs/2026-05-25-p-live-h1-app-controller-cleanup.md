# P-LIVE-H1 Live IO App Controller Cleanup Closure

Date: 2026-05-25 14:14 Asia/Taipei.

## Question

Can the app-layer live IO timer and MIDI teach state move out of `MainComponent` into a small adapter without changing live IO behavior, MIDI teach behavior, or realtime callback safety?

## Contract

Input:

- `AudioAnalyzerSnapshot`
- `PerformancePreferences`
- optional MIDI sender boundary
- MIDI teach target and incoming teach messages

Output:

- `LiveIOAppTimerResult`
- `LiveIOStatusIndicatorState`
- `LiveIOAppMidiTeachView`

Rules:

- `LiveIOAppController` owns app-timer live IO state.
- `LiveIOAppController` owns MIDI teach state.
- `MainComponent` keeps JUCE UI and device registration only.
- Controlled-send MIDI still uses an injected sender boundary.
- Controlled-send OSC remains disabled until OSC target preferences exist.
- MIDI input callbacks still do no UI work; callback only posts parsed CC data to the message thread.
- No proof runner schema or live IO proof behavior changes.

## Proven Flow

```text
MainComponent::tickLiveIOControl()
-> LiveIOAppController::tick()
-> LiveIOControlTimer
-> LiveIOStatusIndicatorState
-> MainComponent liveIOStatusLabel

MainComponent MIDI teach buttons / callback
-> LiveIOAppController teach API
-> PreferencesPanel apply learned channel/CC
```

## Evidence

- `source/app/LiveIOAppController.h`
- `source/app/LiveIOAppController.cpp`
- `tests/LiveIOAppControllerTests.cpp`
- `source/app/MainComponent.h`
- `source/app/MainComponent.cpp`
- `CMakeLists.txt`

## Verification

RED:

- `cmake -S . -B build && cmake --build build --target my_world_live_io_app_controller_tests` failed because `source/app/LiveIOAppController.cpp` did not exist.

GREEN:

- `cmake -S . -B build && cmake --build build --target my_world_live_io_app_controller_tests && ./build/my_world_live_io_app_controller_tests`
- `cmake --build build --target my_world_live_io_app_controller_tests my-world && ./build/my_world_live_io_app_controller_tests`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:

- `live io app controller ok`
- app target `my-world` builds.
- `73/73 tests passed`
- `git diff --check passed`

## Parked

- MIDI input selector/preference.
- Arbitrary binding teach.
- Preference persistence.
- OSC target preferences.
- External OSC targets and always-on OSC receive nodes/server.
- Realtime callback delivery.
