# COOK1 Read-Only Cook Plan

## Status

Closed locally on 2026-05-26 13:29 Asia/Taipei.

Branch:

```text
codex/cook1-read-only-cook-plan
```

## Contract

COOK1 exposes a read-only cook plan from the current `WorkbenchSessionSnapshot` runtime graph summaries. It does not execute nodes, run a scheduler loop, mutate graph/save state, open node functionality, edit mappings, add canvas interactions, or touch Metal.

## One-Line Proof

```text
WorkbenchSessionSnapshot runtimeGraph summaries
-> WorkbenchCookPlanSurface rows
-> MainComponent read-only cook labels
```

## Acceptance

- `WorkbenchSessionSnapshot` carries runtime node and edge summaries from `PatchDocument.graph.runtimeGraph`.
- `WorkbenchCookPlanSurface` derives deterministic cook order/readiness from the current session snapshot.
- MainComponent displays six read-only cook labels: cook, order, target, graph, readiness, execution.
- Blocked and empty-runtime sessions have readable fallback rows.
- Existing runtime surface behavior stays parked: cook execution remains `parked`.

## Accepted Result

```text
WorkbenchSessionSnapshot runtimeGraph summaries
-> WorkbenchCookPlanSurface deterministic order/readiness rows
-> MainComponent read-only cook labels
```

## Verification Target

```text
cmake --build build --target my_world_workbench_cook_plan_surface_tests my_world_workbench_session_tests my-world
./build/my_world_workbench_cook_plan_surface_tests
./build/my_world_workbench_session_tests
ctest --test-dir build --output-on-failure -R "workbench_cook_plan_surface|workbench_runtime_surface|workbench_session|workbench_app_controller"
git diff --check
```

Verification:

```text
cmake -S . -B build
cmake --build build --target my_world_workbench_cook_plan_surface_tests my_world_workbench_session_tests my-world
./build/my_world_workbench_cook_plan_surface_tests
workbench cook plan surface ok
./build/my_world_workbench_session_tests
workbench session ok
ctest --test-dir build --output-on-failure -R "workbench_cook_plan_surface|workbench_runtime_surface|workbench_session|workbench_app_controller"
100% tests passed, 0 tests failed out of 8
git diff --check
passed
```

## Parked

- Runtime cook execution
- Scheduler loop / dirty propagation
- Node functionality
- Shader/audio node execution
- Canvas node interactions
- Mapping editor
- Save mutation
- Metal / production GPU backend work
- Visual polish
