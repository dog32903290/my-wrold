# P-LIVE12 Preference Disk Persistence Closure

Date: 2026-05-25 14:33 Asia/Taipei.

## Question

Can app performance preferences survive restart without putting file IO into audio or MIDI realtime callbacks?

## Contract

Input:

- `PerformancePreferences`
- app preference file path
- UI preference changes from `PreferencesPanel`

Output:

- persisted preference file
- sanitized preferences loaded on startup
- app MIDI/live IO state initialized from loaded preferences

Rules:

- Missing preference file falls back to default preferences.
- Loaded values pass through `sanitizePerformancePreferences()`.
- UI preference changes save the full `PerformancePreferences` snapshot.
- File IO happens during startup or UI preference changes only.
- Audio and MIDI callbacks still do no file IO.
- No arbitrary binding teach, OSC target preference, external receive node, or realtime callback delivery changes.

## Proven Flow

```text
PerformancePreferences
-> savePerformancePreferences()
-> performance-preferences.properties
-> loadPerformancePreferences()
-> PreferencesPanel::applyPerformancePreferences()
-> MainComponent apply MIDI/live IO preferences
```

## Evidence

- `source/preferences/PerformancePreferences.h`
- `source/preferences/PerformancePreferences.cpp`
- `tests/PerformancePreferencesTests.cpp`
- `source/app/AppPaths.h`
- `source/app/AppPaths.cpp`
- `source/app/PreferencesPanel.h`
- `source/app/PreferencesPanel.cpp`
- `source/app/MainComponent.h`
- `source/app/MainComponent.cpp`

## Verification

RED:

- `cmake --build build --target my_world_performance_preferences_tests` failed first because `savePerformancePreferences()` and `loadPerformancePreferences()` did not exist.

GREEN:

- `cmake --build build --target my_world_performance_preferences_tests && ./build/my_world_performance_preferences_tests`
- `cmake --build build --target my_world_performance_preferences_tests my-world`
- `./build/my_world_performance_preferences_tests`
- `ctest --test-dir build --output-on-failure -R "performance_preferences|live_io_app_controller"`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:

- `performance preferences ok`
- app target `my-world` builds.
- focused `performance_preferences|live_io_app_controller` tests passed 2/2.
- full `ctest` passed 73/73.
- `git diff --check` passed.

## Parked

- Arbitrary binding teach.
- OSC target preferences.
- External OSC targets and always-on OSC receive nodes/server.
- Realtime callback delivery.
- Broader MIDI output operators.
