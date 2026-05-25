# P-LIVE14 OSC Target Preferences Closure

Date: 2026-05-25 14:48 Asia/Taipei.

## Question

Can OSC target host/port/address become app preferences without enabling external OSC send yet?

## Contract

Input:

- `PerformancePreferences.liveIO`
- user-provided OSC host, port, and loudness address

Output:

- sanitized OSC target preferences
- persisted preference file fields
- app Live IO binding uses the configured loudness OSC address

Rules:

- Empty OSC host falls back to `127.0.0.1`.
- OSC port clamps to `1..65535`.
- Invalid OSC address falls back to `/my-world/loudness`.
- Preference save/load roundtrips the OSC fields.
- Controlled app timer still keeps external OSC send disabled until P-LIVE15.
- No OSC server, external target proof, or realtime callback delivery changes.

## Proven Flow

```text
PerformancePreferences.liveIO OSC target
-> sanitize host/port/address
-> savePerformancePreferences()
-> loadPerformancePreferences()
-> app OSC binding address can use the sanitized preference
```

## Evidence

- `source/preferences/PerformancePreferences.h`
- `source/preferences/PerformancePreferences.cpp`
- `source/app/LiveIOAppController.cpp`
- `tests/PerformancePreferencesTests.cpp`
- `docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md`

## Verification

RED:

- `cmake --build build --target my_world_performance_preferences_tests` failed first because `LiveIOPreferences` had no OSC target fields.

GREEN:

- `cmake --build build --target my_world_performance_preferences_tests && ./build/my_world_performance_preferences_tests`
- `cmake --build build --target my-world`
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

- UI fields for editing OSC target preferences.
- External OSC target send proof.
- Always-on OSC receive nodes/server.
- Realtime callback delivery.
- Broader MIDI output operators.
