# ACTIVE2 Preparation Diagnostics Proof

## Status

Closed locally as of 2026-05-26 00:44 Asia/Taipei.

## Trigger

ACTIVE1 made the app prepare default debug active work before opening the workbench session, and the stable app proof now indirectly proves that by reading `workSourceStatus: active-work-opened`.

ACTIVE2 adds a direct proof surface for the preparation step itself, so a future failure can be located before resolver/session state gets involved.

## Acceptance

- `ActiveWorkPreparationResult` carries diagnostics strings for default prepared, default ready, external requested, and blocked preparation paths.
- Stable app workbench proof writes a second artifact, `active_work_preparation_report.json`, beside `workbench_open_status_report.json`.
- The preparation report includes `ok`, `status`, `workManifestPath`, `error`, and diagnostics.
- `MainComponent` passes the latest preparation result into the stable app proof request.
- The existing workbench session report remains present and keeps reporting `active-work-opened`.
- No save mutation changes, node functionality, canvas UI, mapping editor, runtime cook loop, OpenGL backend expansion, Metal, analyzer DSP, or visual polish is added.

## Contract

| Question | Answer |
| --- | --- |
| Trigger | Stable app proof runner receives the app-held preparation result when proof dump is requested. |
| Input | `ActiveWorkPreparationResult` from `prepareActiveWorkProjectForOpen()`, plus the current `WorkbenchSessionSnapshot`. |
| Success | App proof writes both session report and preparation report; preparation diagnostics name source, status, manifest path, and error when present. |
| Failure | If writing either artifact fails, proof runner returns failed with the write error; preparation failure itself is represented in the report, not hidden. |
| Observability | Tests read the new artifact; app dump proof readback checks preparation status and diagnostics directly. |

## Target Line

```text
prepareActiveWorkProjectForOpen
-> ActiveWorkPreparationResult diagnostics
-> AppWorkbenchSessionProofRunRequest
-> active_work_preparation_report.json
```

## Verification

- Red test: `cmake --build build --target my_world_app_workbench_session_proof_runner_tests my_world_active_work_service_tests` failed because `AppWorkbenchSessionProofRunRequest` had no `activeWorkPreparation`.
- `cmake --build build --target my_world_app_workbench_session_proof_runner_tests my_world_active_work_service_tests my-world` passed.
- `./build/my_world_app_workbench_session_proof_runner_tests && ./build/my_world_active_work_service_tests` passed.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed.
- `debug/app-workbench-session-proof/active_work_preparation_report.json` read back with `ok: true`, `status: default-active-work-ready`, manifest path, empty error, and diagnostics including `activeWorkPreparationStatus=default-active-work-ready`.
- `debug/app-workbench-session-proof/workbench_open_status_report.json` still read back with `ok: true`, `workSource: active-work`, `workSourceStatus: active-work-opened`, and `diagnostics: []`.
- `ctest --test-dir build --output-on-failure -R "active_work_service|app_workbench_session_proof_runner|workbench_app_controller"` passed 3/3.
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
