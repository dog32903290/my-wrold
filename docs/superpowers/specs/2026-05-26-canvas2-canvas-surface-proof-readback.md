# CANVAS2 Canvas Surface Proof Readback

## Status

Closed locally on 2026-05-26 12:39 Asia/Taipei.

Branch:

```text
codex/canvas2-canvas-surface-proof-readback
```

## Contract

CANVAS2 adds an app proof runner and startup flag that read back the same `WorkbenchCanvasSurface` model used by MainComponent canvas labels. The proof opens a real workbench session, builds the canvas surface from the controller-held session snapshot, and writes `canvas_surface_report.json`.

This lane proves read-only canvas geometry visibility. It does not add a canvas drawing widget, hit-test, gestures, selection, drag, connect, delete, graph mutation commands, mapping editor, runtime cook, save UI, project picker, shader binding expansion, visual polish, or Metal.

## Proof Harness Boundary

```text
trigger adapter: CLI flag and MainComponent startup task
status adapter: existing finishProofDump()
proof runner: WorkbenchCanvasSurfaceProofRunner
artifact writer: canvas_surface_report.json
domain proof: WorkbenchSessionSnapshot graph summaries -> WorkbenchCanvasSurface node bounds and edge routes
```

## UI Skin Pressure Gate

```text
1. This UI reads from: WorkbenchAppController::currentSession() through WorkbenchCanvasSurface.
2. This UI mutates through: no mutation.
3. This is proven by: WorkbenchCanvasSurfaceProofRunner test, startup proof adapter test, app proof dump readback.
```

## Acceptance

- `WorkbenchCanvasSurfaceProofRunner` creates a proof work, opens it through `WorkbenchAppController`, builds `WorkbenchCanvasSurface`, and writes a stable report.
- The report includes canvas identity, row count, node surface count, edge route count, first node bounds, first route endpoints, route points, and row texts.
- The app accepts `--dump-workbench-canvas-surface-proof-and-exit` and dumps the report under `debug/workbench-canvas-surface-proof`.
- `MainComponent` remains a trigger/status adapter and does not build JSON.
- Blocked or empty sessions remain covered by CANVAS1 surface fallbacks.

## Verification Target

```text
cmake -S . -B build
cmake --build build --target my_world_workbench_canvas_surface_proof_runner_tests my_world_startup_proof_tests my-world
./build/my_world_workbench_canvas_surface_proof_runner_tests
./build/my_world_startup_proof_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-workbench-canvas-surface-proof-and-exit
ctest --test-dir build --output-on-failure -R "workbench_canvas_surface_proof_runner|startup_proof|workbench_canvas_surface|workbench_graph_surface|workbench_session|workbench_app_controller"
git diff --check
```

## Result

Closed line:

```text
WorkbenchAppController current session
-> WorkbenchCanvasSurface
-> WorkbenchCanvasSurfaceProofRunner
-> debug/workbench-canvas-surface-proof/canvas_surface_report.json
```

The app proof readback contains `ok: true`, `documentId: patch.canvas2-main`, `nodeSurfaceCount: 2`, `edgeRouteCount: 1`, first node bounds `shader1 80,80 140x60`, first route `shader1.output -> out1.input`, route points `220,110 -> 320,110`, and the six row texts used by the visible canvas surface.

## Verification

```text
cmake -S . -B build
cmake --build build --target my_world_workbench_canvas_surface_proof_runner_tests my_world_startup_proof_tests my-world
./build/my_world_workbench_canvas_surface_proof_runner_tests
./build/my_world_startup_proof_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-workbench-canvas-surface-proof-and-exit
debug/workbench-canvas-surface-proof/canvas_surface_report.json written with ok true
ctest --test-dir build --output-on-failure -R "workbench_canvas_surface_proof_runner|startup_proof|workbench_canvas_surface|workbench_graph_surface|workbench_session|workbench_app_controller"
100% tests passed, 0 tests failed out of 10
git diff --check
passed
```

## Parked

- CANVAS3 canvas segment closure.
- Canvas painting, node hit-test/gesture, selection, drag/connect/delete commands, graph mutation commands, mapping editor, runtime cook, shader preview binding expansion, save UI, project picker, Metal, analyzer DSP, and visual polish remain parked.
