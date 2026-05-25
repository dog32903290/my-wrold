# P-LIVE27 Shader Uniform Proof Artifact

## Status

Closed on 2026-05-25.

## Acceptance

- Live IO proof runner writes `live_io_shader_uniform_report.json`.
- The artifact is listed in `LiveIOProofRunResult::artifactPaths`.
- The artifact records `kind`, `ok`, `status`, nested `shaderUniformEvidence`, binding id, uniform name, value, sample counter, and errors.
- Existing app-timer proof artifacts remain unchanged.
- No shader preview bridge, shader preview smoke-read, live binding, graph node, mapping editor, or realtime callback work is added.

## Target Line

```text
LiveIOControlTimerState shader uniform evidence
-> live_io_shader_uniform_report.json
-> artifact list
-> proof runner test
```

## Evidence

- `source/app/LiveIOProofRunner.cpp`
- `tests/LiveIOProofRunnerTests.cpp`

## Contract Notes

The new artifact is the first standalone proof of `u_loudness` as a shader-facing value. It still comes from app/control-rate evidence, not the renderer. P-LIVE28 is allowed to define the preview input bridge after this artifact exists.

## Verification

- RED: `cmake --build build --target my_world_live_io_proof_runner_tests && ./build/my_world_live_io_proof_runner_tests` failed first because the artifact count was still 10.
- GREEN: `cmake --build build --target my_world_live_io_proof_runner_tests && ./build/my_world_live_io_proof_runner_tests`
- App build: `cmake --build build --target my-world`
- Full suite: `ctest --test-dir build --output-on-failure` passed 75/75.
- Diff check: `git diff --check` passed.

## Parked

- Shader preview input bridge contract.
- Shader preview smoke-read.
- Shader preview live binding.
- Full mapping editor.
- Graph IO node.
