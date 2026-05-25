# P-LIVE9 Send Mode Preferences Closure

Date: 2026-05-25 13:58 Asia/Taipei.

## Question

Can live IO send mode become an explicit app preference, defaulting to dry-run and only switching the app timer to controlled send when the user opts in?

## Contract

Input:

- `LiveIOPreferences`
- `LiveIOSendModePreference`
- Preferences panel send mode selection

Output:

- sanitized `PerformancePreferences.liveIO.sendMode`
- stable send mode string
- `MainComponent` app-timer send mode mapping

Rules:

- Default live IO send mode is `dry_run`.
- Invalid preference values sanitize back to `dry_run`.
- `controlled_send` survives sanitization only when explicitly selected.
- Preferences layer does not depend on JUCE timer or live IO core dispatch types.
- The app timer maps the sanitized preference to `LiveIOControlTimerSendMode`.
- Controlled send keeps OSC disabled until a real OSC target preference exists.
- Controlled MIDI send uses the selected MIDI output already owned by app preferences.
- No work moves into the realtime audio callback.

## Proven Flow

```text
PreferencesPanel live IO send mode combo
-> getLiveIOPreferences()
-> MainComponent::applyLiveIOPreferences()
-> LiveIOControlTimerConfig.sendMode
-> dry_run by default
-> controlled_send only when selected
```

## Evidence

- `source/preferences/PerformancePreferences.h`
- `source/preferences/PerformancePreferences.cpp`
- `tests/PerformancePreferencesTests.cpp`
- `source/app/PreferencesPanel.h`
- `source/app/PreferencesPanel.cpp`
- `source/app/MainComponent.h`
- `source/app/MainComponent.cpp`

## Verification

RED:

- `cmake --build build --target my_world_performance_preferences_tests && ./build/my_world_performance_preferences_tests` failed because `PerformancePreferences.liveIO`, `LiveIOSendModePreference`, and `liveIOSendModePreferenceToString()` did not exist.

GREEN:

- `cmake --build build --target my_world_performance_preferences_tests my-world && ./build/my_world_performance_preferences_tests`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:

- `performance preferences ok`
- app target `my-world` builds.
- `71/71 tests passed`
- `git diff --check passed`

## Parked

- MIDI teach.
- OSC target preferences.
- External OSC targets and always-on OSC receive nodes/server.
- Realtime callback delivery.
- Broader MIDI output operators.
