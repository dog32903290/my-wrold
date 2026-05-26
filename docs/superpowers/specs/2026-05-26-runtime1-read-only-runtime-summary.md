# RUNTIME1 Read-Only Runtime Summary

## Status

Closed locally on 2026-05-26 12:55 Asia/Taipei.

Branch:

```text
codex/runtime1-read-only-runtime-summary
```

## Contract

RUNTIME1 turns the current workbench session's existing runtime graph summary into a read-only runtime surface model. It displays whether the current work has a runtime graph shape that can be inspected by the app.

This is not a runtime cook lane. It does not execute nodes, add node functionality, add scheduler state, mutate graph/storage, build mapping editor controls, add canvas interactions, expand shader live binding, or touch Metal.

One line:

```text
WorkbenchSessionSnapshot runtime counts
-> WorkbenchRuntimeSurface rows
-> MainComponent read-only runtime labels
```

## Acceptance

- `WorkbenchRuntimeSurface` builds six stable rows from `WorkbenchSessionSnapshot`.
- The surface reports document id, runtime node/edge count, active output, mapping status/count, readiness, and cook status.
- Ready sessions with runtime nodes/edges show `runtime ready <documentId> nodes <n> edges <m>`.
- Blocked or empty sessions render readable fallbacks instead of fake runtime execution.
- `MainComponent` displays the surface as read-only labels and refreshes after workbench open/save.

## UI Skin Pressure Gate

```text
1. This UI reads from: WorkbenchAppController::currentSession() / WorkbenchSessionSnapshot runtime summary fields.
2. This UI mutates through: no mutation.
3. This is proven by: WorkbenchRuntimeSurface tests, app build, and focused runtime/workbench tests.
```

## Verification Target

```text
cmake --build build --target my_world_workbench_runtime_surface_tests my-world
./build/my_world_workbench_runtime_surface_tests
ctest --test-dir build --output-on-failure -R "workbench_runtime_surface|workbench_canvas_surface|workbench_graph_surface|workbench_session|workbench_app_controller"
git diff --check
```

## Accepted Result

```text
WorkbenchSessionSnapshot runtime counts
-> WorkbenchRuntimeSurface rows
-> MainComponent read-only runtime labels
```

The app now shows document id, runtime node/edge count, active output, graph IO mapping status, readiness, and an explicit `cook parked` row without executing runtime cook.

## Verification

```text
cmake --build build --target my_world_workbench_runtime_surface_tests my-world
./build/my_world_workbench_runtime_surface_tests
ctest --test-dir build --output-on-failure -R "workbench_runtime_surface|workbench_canvas_surface|workbench_graph_surface|workbench_session|workbench_app_controller"
100% tests passed, 0 tests failed out of 10
git diff --check
passed
```

## Parked

- RUNTIME2 app runtime summary proof readback.
- RUNTIME3 runtime segment closure.
- Runtime cook, scheduler/cook order, node execution, graph mutation commands, mapping editor, canvas node hit-test/gestures, selection, drag/connect/delete, save UI, project picker, shader preview live binding expansion, Metal, analyzer DSP, and visual polish remain parked.
