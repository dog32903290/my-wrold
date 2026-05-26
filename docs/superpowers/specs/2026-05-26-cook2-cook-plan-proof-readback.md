# COOK2 Cook Plan Proof Readback

## Status

Closed locally on 2026-05-26 13:34 Asia/Taipei.

Branch:

```text
codex/cook2-cook-plan-proof-readback
```

## Contract

COOK2 adds an app dump proof for the COOK1 read-only cook plan surface. The proof runner creates a controlled work project, opens it through `WorkbenchAppController`, reads `WorkbenchCookPlanSurface`, and writes `cook_plan_report.json`.

This is still not runtime cook execution. It does not run a scheduler loop, execute shader/audio nodes, mutate save state, edit mappings, add canvas interactions, or touch Metal.

## One-Line Proof

```text
WorkbenchAppController current session
-> WorkbenchCookPlanSurface
-> WorkbenchCookPlanSurfaceProofRunner
-> debug/workbench-cook-plan-proof/cook_plan_report.json
```

## Acceptance

- `WorkbenchCookPlanSurfaceProofRunner` owns proof project creation, session open, surface readback, report serialization, and artifact status.
- Startup proof supports `--dump-workbench-cook-plan-proof-and-exit`.
- MainComponent only adapts the startup proof trigger/status to the runner.
- The report proves document id, runtime node/edge counts, active output node, cook order, readiness, execution status, rows, and empty error on success.
- Runtime cook execution remains parked.

## Accepted Result

```text
WorkbenchAppController current session
-> WorkbenchCookPlanSurface
-> WorkbenchCookPlanSurfaceProofRunner
-> debug/workbench-cook-plan-proof/cook_plan_report.json
```

## Verification Target

```text
cmake -S . -B build
cmake --build build --target my_world_workbench_cook_plan_surface_proof_runner_tests my_world_startup_proof_tests my-world
./build/my_world_workbench_cook_plan_surface_proof_runner_tests
./build/my_world_startup_proof_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-workbench-cook-plan-proof-and-exit
ctest --test-dir build --output-on-failure -R "workbench_cook_plan_surface_proof_runner|startup_proof|workbench_cook_plan_surface|workbench_runtime_surface|workbench_session|workbench_app_controller"
git diff --check
```

Verification:

```text
cmake -S . -B build
cmake --build build --target my_world_workbench_cook_plan_surface_proof_runner_tests my_world_startup_proof_tests my-world
./build/my_world_workbench_cook_plan_surface_proof_runner_tests
workbench cook plan surface proof runner ok
./build/my_world_startup_proof_tests
passed
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-workbench-cook-plan-proof-and-exit
debug/workbench-cook-plan-proof/cook_plan_report.json written with ok true, cookOrderText "shader1 -> out1", readiness "ready", executionStatus "parked"
ctest --test-dir build --output-on-failure -R "workbench_cook_plan_surface_proof_runner|startup_proof|workbench_cook_plan_surface|workbench_runtime_surface|workbench_session|workbench_app_controller"
100% tests passed, 0 tests failed out of 10
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
