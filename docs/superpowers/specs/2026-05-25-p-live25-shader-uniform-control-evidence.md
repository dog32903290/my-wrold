# P-LIVE25 Shader Uniform Control Evidence

## Status

Closed on 2026-05-25.

## Acceptance

- `shader.uniform` dispatch keeps the existing skip behavior for external sends.
- Dispatch reports record each skipped shader uniform as evidence instead of only incrementing `shaderSkippedCount`.
- Timer state records the latest shader uniform binding, name, value, and sample counter.
- Live IO status JSON can expose the latest shader uniform evidence.
- App-timer proof JSON writes the latest `u_loudness` evidence.
- No shader preview live binding, graph node, mapping editor, dynamic OSC scanner, new MIDI operator kind, or realtime callback send is added.

## Target Line

```text
LiveIOPreferences.outputOperator
-> shader.uniform binding
-> LiveIOControlDispatchReport.shaderUniforms
-> LiveIOControlTimerState latest uniform evidence
-> app status JSON and app timer proof JSON
```

## Evidence

- `source/core/LiveIOControlDispatcher.h`
- `source/core/LiveIOControlDispatcher.cpp`
- `source/core/LiveIOControlTimer.h`
- `source/core/LiveIOControlTimer.cpp`
- `source/core/LiveIOStatusIndicator.h`
- `source/core/LiveIOStatusIndicator.cpp`
- `source/app/LiveIOAppController.cpp`
- `source/app/LiveIOProofRunner.cpp`
- `tests/LiveIOControlDispatcherTests.cpp`
- `tests/LiveIOControlTimerTests.cpp`
- `tests/LiveIOStatusIndicatorTests.cpp`
- `tests/LiveIOAppControllerTests.cpp`
- `tests/LiveIOProofRunnerTests.cpp`

## Contract Notes

P-LIVE25 does not send a uniform to the renderer. It records a control-rate handoff value:

- binding: `uniform.loudness`
- uniform: `u_loudness`
- value: normalized loudness float
- sample: analyzer sample counter

The existing `shaderSkippedCount` remains because the dispatcher still does not have a renderer sink. The new `shaderUniforms` evidence explains what was skipped and preserves the value for the next bridge lane.

## Verification

- RED: `cmake --build build --target my_world_live_io_control_dispatcher_tests my_world_live_io_control_timer_tests my_world_live_io_status_indicator_tests my_world_live_io_app_controller_tests` failed first because `LiveIOControlDispatchReport::shaderUniforms` did not exist.
- GREEN: `cmake --build build --target my_world_live_io_control_dispatcher_tests my_world_live_io_control_timer_tests my_world_live_io_status_indicator_tests my_world_live_io_app_controller_tests && ./build/my_world_live_io_control_dispatcher_tests && ./build/my_world_live_io_control_timer_tests && ./build/my_world_live_io_status_indicator_tests && ./build/my_world_live_io_app_controller_tests`
- GREEN: `cmake --build build --target my_world_live_io_proof_runner_tests && ./build/my_world_live_io_proof_runner_tests`
- App build: `cmake --build build --target my-world`
- Full suite: `ctest --test-dir build --output-on-failure` passed 75/75.
- Diff check: `git diff --check` passed.

## Parked

- Standalone shader uniform proof artifact.
- Shader preview input bridge contract.
- Shader preview smoke-read.
- Full mapping editor.
- Multiple simultaneous user-selected bindings.
- Graph node for live IO input/output.
- Direct realtime MIDI/OSC/uniform send from the audio callback.
