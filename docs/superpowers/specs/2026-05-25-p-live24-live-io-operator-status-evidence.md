# P-LIVE24 Live IO Operator Status Evidence

## Status

Closed on 2026-05-25.

## Acceptance

- Live IO app status exposes the selected output operator as a stable string.
- Supported strings stay aligned with `LiveIOPreferences.outputOperator`: `midi.cc`, `midi.note_on`, `osc.float`, and `shader.uniform`.
- Status text appends `op <kind>` after the MIDI/OSC lane counts.
- Status JSON includes `outputOperator`.
- App-timer proof artifacts include the output operator used by the MIDI and OSC proof lanes.
- No mapping editor, graph node, dynamic OSC scanner, new MIDI operator kind, or audio callback work is added.

## Target Line

```text
LiveIOPreferences.outputOperator
-> LiveIOAppController sanitized preference
-> LiveIOStatusIndicatorState.outputOperator
-> status text and status JSON
-> app timer proof JSON
```

## Evidence

- `source/core/LiveIOStatusIndicator.h`
- `source/core/LiveIOStatusIndicator.cpp`
- `source/app/LiveIOAppController.cpp`
- `source/app/LiveIOProofRunner.cpp`
- `tests/LiveIOStatusIndicatorTests.cpp`
- `tests/LiveIOAppControllerTests.cpp`
- `tests/LiveIOProofRunnerTests.cpp`

## Contract Notes

`LiveIOStatusIndicator` does not parse preferences. It only carries a string already selected by the app/controller boundary. This keeps the status component small and lets `LiveIOAppController` remain responsible for preference sanitization.

The app-timer proof artifacts record the operator as evidence only:

- `live_io_app_timer_midi_report.json` records `midi.cc`.
- `live_io_app_timer_osc_loopback_report.json` records `osc.float`.

This closes observability for the picker foundation without adding multiple simultaneous mappings.

## Verification

- RED: `cmake --build build --target my_world_live_io_status_indicator_tests my_world_live_io_app_controller_tests my_world_live_io_proof_runner_tests` failed first because `withLiveIOOutputOperator` did not exist.
- GREEN: `cmake --build build --target my_world_live_io_status_indicator_tests my_world_live_io_app_controller_tests my_world_live_io_proof_runner_tests && ./build/my_world_live_io_status_indicator_tests && ./build/my_world_live_io_app_controller_tests && ./build/my_world_live_io_proof_runner_tests`
- App build: `cmake --build build --target my-world`
- Full suite: `ctest --test-dir build --output-on-failure` passed 75/75.
- Diff check: `git diff --check` passed.

## Parked

- Full mapping editor.
- Multiple simultaneous user-selected bindings.
- Graph node for live IO input/output.
- Dynamic OSC address scanning.
- New MIDI operator kinds beyond existing `midi.cc` and `midi.note_on`.
- Direct realtime MIDI/OSC send from the audio callback.
