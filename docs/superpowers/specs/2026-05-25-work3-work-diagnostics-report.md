# WORK3 Work Diagnostics Report

## Status

Closed locally as of 2026-05-25.

## Trigger

WORK2 extracted active-work resolution into `WorkProjectResolver`, but the app proof still only exposed the final source/status fields. WORK3 adds a non-blocking `workDiagnostics` report path so fallback, active request, resolved manifest, and work source status can be read back without turning a ready session into a blocked one.

## Acceptance

- `WorkProjectResolver` emits `workDiagnostics` for active, fallback, and blocked work resolution.
- `WorkbenchSessionRequest` and `WorkbenchSessionSnapshot` carry `workDiagnostics`.
- `makeWorkbenchSessionReportJson` writes a `workDiagnostics` array before blocking `diagnostics`.
- Work diagnostics do not affect `snapshot.ok`; existing `diagnostics` remains the blocking diagnostics vector.
- Stable app workbench proof command remains `--dump-app-workbench-session-proof-and-exit`.
- No active-work preparation, save mutation, node functionality, canvas UI, mapping editor, runtime cook, OpenGL backend expansion, Metal, or visual polish is added.

## Target Line

```text
WorkProjectResolver
-> workDiagnostics
-> WorkbenchSessionSnapshot
-> workbench_open_status_report.json
```

## Contract

```text
WORK3 answers:
what work source decision did the app make, and where can proof read it back?

Inputs:
- resolved work source and source status
- active work manifest request path
- resolved work manifest path
- resolver error text if resolution blocks

Success:
- ready fixture fallback still has ok: true.
- workDiagnostics records source, source status, active work request, and resolved manifest.
- blocking diagnostics remain separate in diagnostics.

Failure:
- blocked work resolution carries error text in workDiagnostics and still blocks through existing error/status path.

Observability:
- resolver test checks diagnostics for active, fallback, broken active, and missing fixture.
- workbench session test checks JSON contains workDiagnostics while diagnostics remains separate.
- app workbench proof runner test checks stable proof JSON includes workDiagnostics.
```

## Verification

- Red test: `cmake --build build --target my_world_work_project_resolver_tests my_world_workbench_session_tests my_world_app_workbench_session_proof_runner_tests` failed because `WorkProjectResolveResult` had no `workDiagnostics`.
- `cmake --build build --target my_world_work_project_resolver_tests my_world_workbench_session_tests my_world_app_workbench_session_proof_runner_tests my_world_workbench_session_open_status_tests` passed.
- `./build/my_world_work_project_resolver_tests` passed with `work project resolver ok`.
- `./build/my_world_workbench_session_tests` passed with `workbench session ok`.
- `./build/my_world_app_workbench_session_proof_runner_tests` passed with `app workbench session proof runner ok`.
- `./build/my_world_workbench_session_open_status_tests` passed with `workbench session open status ok`.
- `cmake --build build --target my-world` passed.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed and wrote `debug/app-workbench-session-proof/workbench_open_status_report.json` with `workDiagnostics` and `diagnostics: []`.
- `ctest --test-dir build --output-on-failure` passed 87/87.
- `git diff --check` passed.

## Parked

- Active-work preparation.
- Save mutation.
- Node functionality.
- Canvas UI or workbench panels.
- Mapping editor.
- Runtime cook loop.
- Shader preview live-binding expansion.
- OpenGL backend expansion.
- Metal and visual polish.
