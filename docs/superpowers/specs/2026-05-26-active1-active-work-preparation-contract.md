# ACTIVE1 Active Work Preparation Contract

## Status

Closed locally as of 2026-05-26 00:35 Asia/Taipei.

## Trigger

APP and WORK segments made the app able to report whether its workbench session came from active work or fixture fallback. The missing line is the first active-work preparation entrypoint for app open: when no `MY_WORLD_ACTIVE_WORK_MANIFEST` override is set, the app should prepare the default debug active work before opening the workbench session.

ACTIVE1 uses the existing `ActiveWorkService` fixture-copy path instead of inventing a second preparation system.

## Acceptance

- `ActiveWorkService` exposes a preparation result for app open/status.
- With no `MY_WORLD_ACTIVE_WORK_MANIFEST`, preparation creates the default active work manifest, main patch, module library, and module manifest under the project debug active-work area if missing.
- Preparation is idempotent when those files already exist.
- After preparation, resolving the current workbench session from the default manifest reports `active-work-opened`.
- `MainComponent::openWorkbenchSession()` calls preparation before opening the session, so stable app proof can read an active-work session by default.
- Explicit `MY_WORLD_ACTIVE_WORK_MANIFEST` paths are not auto-created.
- No save mutation, node functionality, canvas UI, mapping editor, runtime cook loop, OpenGL backend expansion, Metal, analyzer DSP, or visual polish is added.

## Contract

| Question | Answer |
| --- | --- |
| Trigger | App workbench open/status startup path and tests call the preparation function before resolving the current session. |
| Input | Project directory from `MY_WORLD_PROJECT_DIR` or existing `AppPaths` policy; optional `MY_WORLD_ACTIVE_WORK_MANIFEST`; repo fixture candidates for the C2 work, main patch, default module library, and loudness module manifest. |
| Success | Default path: copy missing fixture files into `debug/c3-active-work` and return ok with manifest path and preparation status. External path: return ok without creating files so existing resolver semantics handle it. |
| Failure | If fixture copy or directory creation fails, return not ok with an error string; the caller can still decide whether to continue opening a fallback session. |
| Observability | Tests check file creation and resolver status; stable app proof readback checks `workSourceStatus: active-work-opened`; master plan records commands and report fields. |

## Target Line

```text
ActiveWorkService prepare
-> default debug active work files
-> WorkProjectResolver
-> WorkbenchSessionSnapshot active-work-opened
-> MainComponent stable app proof
```

## Verification

- Red test: `cmake --build build --target my_world_active_work_service_tests` failed because `prepareActiveWorkProjectForOpen()` did not exist.
- `cmake --build build --target my_world_active_work_service_tests my-world` passed.
- `./build/my_world_active_work_service_tests` passed.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed.
- `debug/app-workbench-session-proof/workbench_open_status_report.json` read back with `ok: true`, `workSource: active-work`, `workSourceStatus: active-work-opened`, populated `workDiagnostics`, and `diagnostics: []`.
- `ctest --test-dir build --output-on-failure -R "active_work_service|work_project_resolver|workbench_session_open_status|workbench_app_controller|app_workbench_session_proof_runner"` passed 5/5.
- `ctest --test-dir build --output-on-failure` passed 87/87.
- `git diff --check` passed.

## Parked

- Save mutation changes.
- Node functionality.
- Canvas UI or workbench panels.
- Mapping editor.
- Runtime cook loop.
- Shader preview live-binding expansion.
- OpenGL backend expansion.
- Metal and visual polish.
