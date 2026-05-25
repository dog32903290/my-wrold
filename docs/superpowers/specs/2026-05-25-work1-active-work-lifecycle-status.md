# WORK1 Active Work Lifecycle Status

## Status

Closed locally as of 2026-05-25.

## Trigger

APP4 made active-work source truth visible, but the lifecycle vocabulary still lived as inline strings inside `WorkbenchSessionOpenStatus`. WORK1 extracts the active/fixture/blocked source statuses into a small app model so the app can keep using the same session-open behavior without scattering state names.

## Acceptance

- `WorkProjectLifecycle` defines the app work source, source status, and blocking flag for:
  - `active-work-opened`
  - `active-work-blocked`
  - `fixture-fallback-no-active-request`
  - `fixture-fallback-active-missing`
  - `fixture-blocked-no-active-request`
  - `fixture-blocked-active-missing`
- `WorkbenchSessionOpenStatus` reads those lifecycle states instead of owning raw status literals.
- Existing open/status behavior remains intact for active work, fixture fallback, and broken active work.
- Stable app workbench proof command remains `--dump-app-workbench-session-proof-and-exit`.
- No active work creation, save mutation, node functionality, canvas UI, mapping editor, runtime cook, OpenGL backend expansion, Metal, or visual polish is added.

## Target Line

```text
active work request
-> WorkProjectLifecycle
-> WorkbenchSessionOpenStatus
-> WorkbenchSessionSnapshot
-> app workbench session proof
```

## Contract

```text
WORK1 answers:
what lifecycle state describes the work source the app is trying to open?

Inputs:
- active work request present or absent
- active work load success or block
- fixture fallback success or block

Success:
- each lifecycle state has one source string, one status string, and one blocking answer.
- open/status uses the lifecycle model without changing the session loader contract.

Failure:
- blocked active work still blocks instead of silently falling back.
- blocked fixture states remain explicit and readable.

Observability:
- focused lifecycle test covers all six states.
- focused workbench open/status test still covers active, missing fallback, and broken active work.
- stable app dump proof still reports the current workbench session.
```

## Verification

- Red test: `cmake -S . -B build && cmake --build build --target my_world_work_project_lifecycle_tests` failed because `source/app/WorkProjectLifecycle.cpp` did not exist.
- `cmake -S . -B build && cmake --build build --target my_world_work_project_lifecycle_tests my_world_workbench_session_open_status_tests` passed.
- `cmake --build build --target my_world_work_project_lifecycle_tests my_world_workbench_session_open_status_tests my_world_workbench_app_controller_tests my-world` passed.
- `./build/my_world_work_project_lifecycle_tests` passed with `work project lifecycle ok`.
- `./build/my_world_workbench_session_open_status_tests` passed with `workbench session open status ok`.
- `./build/my_world_workbench_app_controller_tests` passed with `workbench app controller ok`.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed and wrote `debug/app-workbench-session-proof/workbench_open_status_report.json` with `ok: true` and `workSourceStatus: fixture-fallback-active-missing`.
- `ctest --test-dir build --output-on-failure` passed 86/86.
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
