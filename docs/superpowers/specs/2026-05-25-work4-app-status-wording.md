# WORK4 App Status Wording

## Status

Closed locally as of 2026-05-25.

## Trigger

WORK1-WORK3 made active/fixture work state explicit in snapshots and proof JSON, but the app/controller status text still only said `workbench ready <document> mappings 1/1`. WORK4 makes the visible status text name the work source status directly, without changing session behavior or report schema.

## Acceptance

- Ready status text includes `source <workSourceStatus>`.
- Blocked status text includes `source <workSourceStatus>`.
- `WorkbenchAppController::statusText()` exposes the same source-aware wording.
- Stable app workbench proof JSON schema is unchanged.
- No active-work preparation, save mutation, node functionality, canvas UI, mapping editor, runtime cook, OpenGL backend expansion, Metal, or visual polish is added.

## Target Line

```text
WorkbenchSessionSnapshot
-> makeWorkbenchSessionStatusText
-> WorkbenchAppController::statusText
-> MainComponent status label
```

## Contract

```text
WORK4 answers:
what source state is the app status line talking about?

Inputs:
- snapshot.status
- snapshot.documentId
- snapshot.workSourceStatus
- graph IO mapping counts
- snapshot.message for blocked sessions

Success:
- ready status still starts with workbench ready.
- ready status still names document and mapping count.
- ready and blocked status now name source status.

Failure:
- blocked status still uses the existing snapshot message, now with source status beside it.

Observability:
- open/status test checks ready and blocked source wording.
- app controller test checks controller status source wording.
- app proof still passes without schema change.
```

## Verification

- Red test: `cmake --build build --target my_world_workbench_session_open_status_tests my_world_workbench_app_controller_tests && ./build/my_world_workbench_session_open_status_tests` failed because status text did not include `source fixture-fallback-active-missing`.
- `cmake --build build --target my_world_workbench_session_open_status_tests my_world_workbench_app_controller_tests` passed.
- `./build/my_world_workbench_session_open_status_tests` passed with `workbench session open status ok`.
- `./build/my_world_workbench_app_controller_tests` passed with `workbench app controller ok`.
- `cmake --build build --target my-world` passed.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed and kept the stable app proof JSON fields.
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
