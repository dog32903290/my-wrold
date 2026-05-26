# RUN1 Workbench Headless Run

## Status

Closed locally on 2026-05-26 14:11 Asia/Taipei.

Branch:

```text
codex/run1-workbench-headless-run
```

## Contract

RUN1 connects a current `WorkbenchSessionSnapshot` runtime graph to the existing headless render runtime path through a narrow adapter and read-only run surface. It supports only the current headless runtime's proven graph shape:

```text
image.constant -> output.texture_summary
```

Unsupported graphs, including the default `shader.fragment -> output.preview` graph, must report `unsupported` rather than pretending shader/audio node execution exists.

## One-Line Proof

```text
WorkbenchSessionSnapshot runtimeGraph
-> WorkbenchRunSurface / headless fixture adapter
-> HeadlessRenderRuntime artifacts
```

## Acceptance

- `WorkbenchSessionSnapshot::GraphNodeSummary` carries runtime param summaries needed by the headless adapter.
- `WorkbenchRunSurface` reports blocked, unsupported, ready, and run states in six read-only rows.
- `runWorkbenchHeadlessRender()` converts a supported current-session runtime graph into the existing headless fixture shape and writes `texture_summary.json`, `cook_order.json`, `node_stats.json`, `thumbnail.png`, `thumbnail_stats.json`, and `errors.json`.
- MainComponent displays read-only run labels from the current session state.
- Unsupported shader graphs are visible as unsupported and do not become fake shader execution.

## Accepted Result

```text
WorkbenchSessionSnapshot runtimeGraph
-> WorkbenchRunSurface / headless fixture adapter
-> HeadlessRenderRuntime artifacts
```

## Verification Target

```text
cmake -S . -B build
cmake --build build --target my_world_workbench_run_surface_tests my_world_workbench_session_tests my-world
./build/my_world_workbench_run_surface_tests
./build/my_world_workbench_session_tests
ctest --test-dir build --output-on-failure -R "workbench_run_surface|workbench_cook_plan_surface|workbench_session|workbench_app_controller|headless_render_runtime"
git diff --check
```

Verification:

```text
cmake -S . -B build
cmake --build build --target my_world_workbench_run_surface_tests my_world_workbench_session_tests my-world
./build/my_world_workbench_run_surface_tests
workbench run surface ok
./build/my_world_workbench_session_tests
workbench session ok
ctest --test-dir build --output-on-failure -R "workbench_run_surface|workbench_cook_plan_surface|workbench_session|workbench_app_controller|headless_render_runtime"
100% tests passed, 0 tests failed out of 9
git diff --check
passed
```

## Parked

- Full runtime scheduler
- Dirty propagation
- Real shader.fragment execution
- Audio runtime node execution
- Node editor functionality
- Canvas interactions
- Mapping editor
- Save mutation
- Metal / production GPU backend work
- Visual polish
