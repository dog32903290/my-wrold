# P-LIVE23 UI Operator Picker Foundation

## Status

Closed on 2026-05-25.

## Acceptance

- Live IO preferences store one selected output operator.
- Supported choices are `midi.cc`, `midi.note_on`, `osc.float`, and `shader.uniform`.
- Invalid stored/operator values sanitize back to `midi.cc`.
- `LiveIOAppController` builds the primary loudness binding from the selected operator.
- Preferences UI exposes the operator choice without adding a full mapping editor.
- No new MIDI operator kind, graph node, dynamic OSC address scanner, or audio callback work is added.

## Target Line

```text
LiveIOPreferences.outputOperator
-> PreferencesPanel combo
-> LiveIOAppController binding selection
-> existing LiveIOControlTimer/dispatcher
-> live IO status evidence
```

## Evidence

- `source/preferences/PerformancePreferences.h`
- `source/preferences/PerformancePreferences.cpp`
- `source/app/PreferencesPanel.h`
- `source/app/PreferencesPanel.cpp`
- `source/app/LiveIOAppController.cpp`
- `tests/PerformancePreferencesTests.cpp`
- `tests/LiveIOAppControllerTests.cpp`

## Contract Notes

`midi.cc` is the safe default and invalid fallback because it matches the existing loudness CC vocabulary. The picker selects one primary loudness output operator at a time:

- `midi.cc` -> `makeLiveIOMidiCcBinding()`
- `midi.note_on` -> `makeLiveIOMidiNoteOnBinding()`
- `osc.float` -> `makeLiveIOOscFloatBinding()`
- `shader.uniform` -> `makeLiveIOShaderUniformBinding()`

This closes the operator selection contract without creating a mapping editor or graph node.

## Verification

- RED: `cmake --build build --target my_world_performance_preferences_tests` failed first because `LiveIOOutputOperatorPreference` and `LiveIOPreferences::outputOperator` did not exist.
- RED: `cmake --build build --target my_world_live_io_app_controller_tests && ./build/my_world_live_io_app_controller_tests` failed first because the controller still built fixed MIDI+OSC bindings.
- GREEN: `cmake --build build --target my_world_performance_preferences_tests my_world_live_io_app_controller_tests my-world && ./build/my_world_performance_preferences_tests && ./build/my_world_live_io_app_controller_tests`
- Focused CTest: `ctest --test-dir build --output-on-failure -R "performance_preferences|live_io_app_controller"`
- Full suite: `ctest --test-dir build --output-on-failure` passed 75/75.
- Diff check: `git diff --check` passed.

## Parked

- Full mapping editor.
- Multiple simultaneous user-selected bindings.
- Graph node for live IO input/output.
- Dynamic OSC address scanning.
- New MIDI operator kinds beyond existing `midi.cc` and `midi.note_on`.
