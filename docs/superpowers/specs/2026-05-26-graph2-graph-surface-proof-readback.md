# GRAPH2 Graph Surface Proof Readback

## Status

Closed locally on 2026-05-26 12:16 Asia/Taipei.

Branch:

```text
codex/graph2-graph-surface-proof-readback
```

## Contract

GRAPH2 adds an app proof runner and startup flag that read back the same `WorkbenchGraphSurface` model used by MainComponent graph labels. The proof opens a real workbench session, builds the graph surface from the controller-held session snapshot, and writes `graph_surface_report.json`.

This lane proves read-only graph visibility. It does not add canvas drawing, hit-test, gestures, graph mutation, mapping editor, runtime cook, save UI, project picker, shader binding expansion, or Metal.

## Proof Harness Boundary

```text
trigger adapter: CLI flag and MainComponent startup task
status adapter: existing finishProofDump()
proof runner: WorkbenchGraphSurfaceProofRunner
artifact writer: graph_surface_report.json
domain proof: WorkbenchSessionSnapshot graph summaries -> WorkbenchGraphSurface rows
```

## Acceptance

- `WorkbenchGraphSurfaceProofRunner` creates a proof work, opens it through `WorkbenchAppController`, builds `WorkbenchGraphSurface`, and writes a stable report.
- The report includes graph identity, editor/runtime counts, active output, first node, first edge, row texts, and node/edge summary arrays.
- The app accepts `--dump-workbench-graph-surface-proof-and-exit` and dumps the report under `debug/workbench-graph-surface-proof`.
- `MainComponent` remains a trigger/status adapter and does not build JSON.

## Result

Closed line:

```text
WorkbenchAppController current session
-> WorkbenchGraphSurface
-> WorkbenchGraphSurfaceProofRunner
-> debug/workbench-graph-surface-proof/graph_surface_report.json
```

The app proof readback contains `ok: true`, `documentId: patch.graph2-main`, editor/runtime counts `2/1`, `activeOutputNodeId: out1`, first node `shader1 shader.fragment`, first edge `shader1.output -> out1.input`, and the six row texts used by the visible graph surface.

## Verification Target

```text
cmake -S . -B build
cmake --build build --target my_world_workbench_graph_surface_proof_runner_tests my_world_startup_proof_tests my-world
./build/my_world_workbench_graph_surface_proof_runner_tests
./build/my_world_startup_proof_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-workbench-graph-surface-proof-and-exit
ctest --test-dir build --output-on-failure -R "workbench_graph_surface_proof_runner|startup_proof|workbench_graph_surface|workbench_session|workbench_app_controller"
git diff --check
```

All commands passed. The app proof command wrote:

```text
debug/workbench-graph-surface-proof/graph_surface_report.json
```

## Parked

- GRAPH3 graph segment closure.
- Canvas node rendering, node hit-test/gesture, graph mutation commands, mapping editor, runtime cook, shader preview binding expansion, save UI, project picker, Metal, analyzer DSP, and visual polish remain parked.
