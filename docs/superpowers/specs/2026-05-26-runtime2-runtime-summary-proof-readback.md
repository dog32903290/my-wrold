# RUNTIME2 Runtime Summary Proof Readback

## Status

Closed locally on 2026-05-26 13:01 Asia/Taipei.

Branch:

```text
codex/runtime2-runtime-summary-proof-readback
```

## Contract

RUNTIME2 adds an app proof runner and startup flag that read back the same `WorkbenchRuntimeSurface` model used by MainComponent runtime labels. The proof opens a real workbench session, builds the runtime surface from the controller-held session snapshot, and writes `runtime_surface_report.json`.

This lane proves read-only runtime summary visibility. It does not execute runtime cook, add scheduler state, add node execution, mutate graph/storage, build mapping editor controls, expand shader live binding, or touch Metal.

## Proof Harness Boundary

```text
trigger adapter: CLI flag and MainComponent startup task
status adapter: existing finishProofDump()
proof runner: WorkbenchRuntimeSurfaceProofRunner
artifact writer: runtime_surface_report.json
domain proof: WorkbenchSessionSnapshot runtime counts -> WorkbenchRuntimeSurface rows
```

## Acceptance

- `WorkbenchRuntimeSurfaceProofRunner` creates a proof work, opens it through `WorkbenchAppController`, builds `WorkbenchRuntimeSurface`, and writes a stable report.
- The report includes runtime identity, row count, runtime node/edge count, active output, mapping status/count, readiness, cook status, and row texts.
- The app accepts `--dump-workbench-runtime-surface-proof-and-exit` and dumps the report under `debug/workbench-runtime-surface-proof`.
- `MainComponent` remains a trigger/status adapter and does not build JSON.
- Runtime cook remains parked and is reported as parked, not simulated.

## Verification Target

```text
cmake -S . -B build
cmake --build build --target my_world_workbench_runtime_surface_proof_runner_tests my_world_startup_proof_tests my-world
./build/my_world_workbench_runtime_surface_proof_runner_tests
./build/my_world_startup_proof_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-workbench-runtime-surface-proof-and-exit
ctest --test-dir build --output-on-failure -R "workbench_runtime_surface_proof_runner|startup_proof|workbench_runtime_surface|workbench_canvas_surface|workbench_graph_surface|workbench_session|workbench_app_controller"
git diff --check
```

## Result

Closed line:

```text
WorkbenchAppController current session
-> WorkbenchRuntimeSurface
-> WorkbenchRuntimeSurfaceProofRunner
-> debug/workbench-runtime-surface-proof/runtime_surface_report.json
```

The app proof readback contains `ok: true`, `documentId: patch.runtime2-main`, runtime counts `2/1`, `activeOutputNodeId: out1`, `graphIOMappingStatus: valid`, mapping counts `1/1`, `readiness: ready`, `cookStatus: parked`, and the six row texts used by the visible runtime surface.

## Verification

```text
cmake -S . -B build
cmake --build build --target my_world_workbench_runtime_surface_proof_runner_tests my_world_startup_proof_tests my-world
./build/my_world_workbench_runtime_surface_proof_runner_tests
./build/my_world_startup_proof_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-workbench-runtime-surface-proof-and-exit
debug/workbench-runtime-surface-proof/runtime_surface_report.json written with ok true
ctest --test-dir build --output-on-failure -R "workbench_runtime_surface_proof_runner|startup_proof|workbench_runtime_surface|workbench_canvas_surface|workbench_graph_surface|workbench_session|workbench_app_controller"
100% tests passed, 0 tests failed out of 12
git diff --check
passed
```

## Parked

- RUNTIME3 runtime segment closure.
- Runtime cook, scheduler/cook order, node execution, graph mutation commands, mapping editor, canvas node hit-test/gestures, selection, drag/connect/delete, save UI, project picker, shader preview live binding expansion, Metal, analyzer DSP, and visual polish remain parked.
