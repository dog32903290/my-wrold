# P-LIVE26 Shader Uniform Evidence JSON Shape

## Status

Closed on 2026-05-25.

## Acceptance

- Status JSON includes a nested `shaderUniformEvidence` object.
- App-timer proof JSON includes the same nested evidence shape.
- The object records source, binding id, uniform name, value, and sample counter.
- Existing flat fields from P-LIVE25 remain in place for compatibility.
- No standalone artifact, shader preview bridge, shader preview smoke-read, live binding, graph node, or mapping editor is added.

## Target Line

```text
P-LIVE25 flat uniform fields
-> nested shaderUniformEvidence object
-> status JSON
-> app timer proof JSON
```

## Evidence

- `source/core/LiveIOStatusIndicator.cpp`
- `source/app/LiveIOProofRunner.cpp`
- `tests/LiveIOStatusIndicatorTests.cpp`
- `tests/LiveIOProofRunnerTests.cpp`

## Contract Notes

P-LIVE26 is shape-only. It does not change how `u_loudness` is produced. The nested object gives the next lane a single stable JSON object to lift into a standalone artifact.

```json
{
  "source": "LiveIOControlTimerState",
  "bindingId": "uniform.loudness",
  "uniformName": "u_loudness",
  "value": 0.5,
  "sampleCounter": 64
}
```

## Verification

- RED: `cmake --build build --target my_world_live_io_status_indicator_tests my_world_live_io_proof_runner_tests && ./build/my_world_live_io_status_indicator_tests && ./build/my_world_live_io_proof_runner_tests` failed first because `shaderUniformEvidence` did not exist.
- GREEN: `cmake --build build --target my_world_live_io_status_indicator_tests my_world_live_io_proof_runner_tests && ./build/my_world_live_io_status_indicator_tests && ./build/my_world_live_io_proof_runner_tests`
- App build: `cmake --build build --target my-world`
- Full suite: `ctest --test-dir build --output-on-failure` passed 75/75.
- Diff check: `git diff --check` passed.

## Parked

- Standalone shader uniform proof artifact.
- Shader preview input bridge contract.
- Shader preview smoke-read.
- Shader preview live binding.
- Full mapping editor.
