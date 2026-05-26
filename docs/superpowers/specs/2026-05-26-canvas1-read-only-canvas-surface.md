# CANVAS1 Read-Only Canvas Surface

## Status

Closed locally on 2026-05-26 12:31 Asia/Taipei.

Branch:

```text
codex/canvas1-read-only-canvas-surface
```

## Contract

CANVAS1 turns the current workbench graph summaries into a read-only canvas surface model. It computes stable node bounds and first edge route points from real graph node positions, then lets `MainComponent` display compact read-only canvas labels.

This is not a canvas interaction lane. It does not add drawing widgets, hit-test gestures, selection, drag, connect, delete, undo/redo, graph mutation commands, mapping editor, runtime cook, shader binding expansion, save UI, project picker, visual polish, or Metal.

## UI Skin Pressure Gate

```text
1. This UI reads from: WorkbenchAppController::currentSession() / WorkbenchSessionSnapshot graph summaries.
2. This UI mutates through: no mutation.
3. This is proven by: WorkbenchCanvasSurface tests, app build, and focused canvas/graph/workbench tests.
```

## Acceptance

- `WorkbenchCanvasSurface` builds read-only node surfaces and edge routes from `WorkbenchSessionSnapshot`.
- Node surfaces use the existing default canvas geometry contract.
- Edge routes resolve endpoint node ids and expose deterministic start/end points.
- `MainComponent` displays the surface as read-only labels and refreshes it after workbench open/save.
- Blocked or empty sessions render readable fallbacks instead of fake canvas content.

## Verification Target

```text
cmake --build build --target my_world_workbench_canvas_surface_tests my-world
./build/my_world_workbench_canvas_surface_tests
ctest --test-dir build --output-on-failure -R "workbench_canvas_surface|workbench_graph_surface|workbench_session|workbench_app_controller"
git diff --check
```

## Accepted Result

```text
WorkbenchSessionSnapshot graph summaries
-> WorkbenchCanvasSurface node bounds and edge routes
-> MainComponent read-only canvas labels
```

## Verification

```text
cmake --build build --target my_world_workbench_canvas_surface_tests my-world
./build/my_world_workbench_canvas_surface_tests
ctest --test-dir build --output-on-failure -R "workbench_canvas_surface|workbench_graph_surface|workbench_session|workbench_app_controller"
100% tests passed, 0 tests failed out of 8
git diff --check
passed
```

## Parked

- CANVAS2 app canvas surface proof readback.
- CANVAS3 canvas segment closure.
- Canvas painting, node hit-test/gesture, selection, drag/connect/delete commands, mapping editor, runtime cook, shader preview binding expansion, save UI, project picker, Metal, analyzer DSP, and visual polish remain parked.
