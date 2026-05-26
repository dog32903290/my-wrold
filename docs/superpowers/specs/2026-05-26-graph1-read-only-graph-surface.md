# GRAPH1 Read-Only Graph Surface

## Status

Closed locally on 2026-05-26 12:09 Asia/Taipei.

Branch:

```text
codex/graph1-read-only-graph-surface
```

## Contract

GRAPH1 lets the app read the current workbench session's real graph as a small read-only surface. It exposes graph node/edge summaries from `WorkbenchSessionSnapshot`, converts them into a JUCE-free `WorkbenchGraphSurface` view model, and lets `MainComponent` display compact graph labels.

This is not a canvas interaction lane. It does not add hit-test, selection, drag, connect, delete, undo/redo, mapping editing, runtime cook, save mutation, visual polish, or Metal.

## UI Skin Pressure Gate

```text
1. This UI reads from: WorkbenchAppController::currentSession() / WorkbenchSessionSnapshot graph summaries.
2. This UI mutates through: no mutation.
3. This is proven by: WorkbenchSession snapshot tests, WorkbenchGraphSurface tests, app build, and focused graph/status tests.
```

## Acceptance

- `WorkbenchSessionSnapshot` carries read-only editor graph node and edge summaries from the loaded `PatchDocument` graph.
- `WorkbenchGraphSurface` renders a stable compact surface from that snapshot: graph identity, editor counts, runtime counts, active output, first visible node, and first visible edge.
- `MainComponent` displays the surface as read-only labels and refreshes it after workbench open/save.
- Blocked or empty sessions render readable fallbacks instead of fake graph content.
- No graph mutation or canvas UI enters this lane.

## Result

Closed line:

```text
PatchDocument graph
-> WorkbenchSessionSnapshot editor node/edge summaries
-> WorkbenchGraphSurface rows
-> MainComponent read-only graph labels
```

The app now has a compact graph row that reads the current workbench session's real graph identity, editor/runtime counts, active output, first editor node, and first editor edge.

## Verification Target

```text
cmake --build build --target my_world_workbench_graph_surface_tests my_world_workbench_session_tests my-world
./build/my_world_workbench_graph_surface_tests
./build/my_world_workbench_session_tests
ctest --test-dir build --output-on-failure -R "workbench_graph_surface|workbench_session|workbench_app_controller|workbench_status_surface"
git diff --check
```

All commands passed.

## Parked

- GRAPH2 app graph surface proof readback.
- GRAPH3 graph segment closure.
- Canvas node rendering, node hit-test/gesture, graph mutation commands, mapping editor, runtime cook, shader preview binding expansion, save UI, project picker, Metal, analyzer DSP, and visual polish remain parked.
