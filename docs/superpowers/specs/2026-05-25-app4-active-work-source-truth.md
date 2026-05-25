# APP4 Active Work Source Truth

## Status

Closed locally as of 2026-05-25.

## Trigger

APP2 and APP3 made the app hold a current workbench session, but the report did not say enough about why that session came from active work or fixture fallback. APP4 makes work source truth explicit without creating, saving, or mutating active work.

## Acceptance

- If active work exists and loads, the snapshot records `workSource: active-work`.
- If active work is missing, the snapshot records fixture fallback with the requested active work path.
- If active work exists but is invalid, the session blocks and does not silently fall back to fixture.
- Report JSON includes `workSourceStatus` and `activeWorkManifestPath`.
- Existing APP2 app dump proof writes the new source truth fields.
- No active-work preparation, save mutation, node functionality, canvas UI, mapping editor, runtime cook, OpenGL backend expansion, Metal, or visual polish is added.

## Target Line

```text
active work manifest path
-> active-work-opened | fixture-fallback-active-missing | active-work-blocked
-> WorkbenchSessionSnapshot
-> workbench_open_status_report.json
```

## Contract

```text
APP4 answers:
why did the current app session come from active work or fixture?

Inputs:
- activeWorkManifestPath
- C2 fixture fallback
- G1 mapping fixture

Success:
- active work opened records active-work-opened.
- missing active work records fixture-fallback-active-missing.
- broken active work records active-work-blocked and blocks the session.

Failure:
- missing fixture or G1 mapping still blocks with readable diagnostics.

Observability:
- focused open/status test covers active, missing fallback, and broken active work.
- app dump proof reports source truth fields.
```

## Verification

- Red test: `cmake --build build --target my_world_workbench_session_open_status_tests` failed because `WorkbenchSessionSnapshot` had no `workSourceStatus` or `activeWorkManifestPath`.
- `cmake --build build --target my_world_workbench_session_open_status_tests my_world_workbench_app_controller_tests my-world` passed.
- `./build/my_world_workbench_session_open_status_tests` passed with `workbench session open status ok`.
- `./build/my_world_workbench_app_controller_tests` passed with `workbench app controller ok`.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app2-workbench-open-status-proof-and-exit` passed and wrote `debug/app2-workbench-open-status-proof/workbench_open_status_report.json` with `workSourceStatus: fixture-fallback-active-missing`.
- `ctest --test-dir build --output-on-failure` passed 84/84.
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
