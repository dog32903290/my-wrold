# APP3 Workbench App Controller Spine

## Status

Closed locally as of 2026-05-25.

## Trigger

APP2 made the running app open and hold a `WorkbenchSessionSnapshot`, but `MainComponent` still directly owned the current session state and proof request assembly. APP3 extracts a small app controller so the UI reads status and proof state through one workbench app boundary.

## Acceptance

- `WorkbenchAppController` owns the current `WorkbenchSessionSnapshot`.
- Controller opens the current session through the APP2 open/status loader.
- Controller exposes readable status text from the held snapshot.
- Controller creates APP2 open/status proof requests from the held snapshot.
- `MainComponent` uses `WorkbenchAppController` instead of directly owning a `WorkbenchSessionSnapshot`.
- Existing APP2 app dump proof still writes the app-held current session report.
- No node functionality, canvas UI, mapping editor, runtime cook, save mutation, OpenGL backend expansion, Metal, or visual polish is added.

## Target Line

```text
WorkbenchSessionOpenStatus
-> WorkbenchAppController::currentSessionSnapshot
-> MainComponent reads controller status
-> APP2 app dump proof reads controller-held snapshot
```

## Contract

```text
WorkbenchAppController answers:
what current workbench session does the app body hold, and what status/proof request should UI adapters read?

Inputs:
- WorkbenchAppControllerOpenRequest
- output directory for proof request creation

Success:
- openCurrentSession stores the snapshot.
- statusText reflects the stored snapshot.
- makeOpenStatusProofRequest returns the stored snapshot.

Failure:
- open failure still stores a blocked snapshot with readable status.

Observability:
- focused controller test proves held snapshot/status/proof request.
- app dump proof remains `workbench_open_status_report.json`.
```

## Verification

- Red test: `cmake --build build --target my_world_workbench_app_controller_tests` failed because `WorkbenchAppController.h` did not exist.
- `cmake -S . -B build` passed.
- `cmake --build build --target my_world_workbench_app_controller_tests my-world` passed.
- `./build/my_world_workbench_app_controller_tests` passed with `workbench app controller ok`.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app2-workbench-open-status-proof-and-exit` passed and wrote `debug/app2-workbench-open-status-proof/workbench_open_status_report.json` with `ok: true`, `status: ready`, `workSource: fixture`, and `graphIOMappingStatus: valid`.
- `ctest --test-dir build --output-on-failure` passed 84/84.
- `git diff --check` passed.

## Parked

- Node functionality.
- Canvas UI or workbench panels.
- Mapping editor.
- Runtime cook loop.
- Save mutation or active-work preparation.
- Shader preview live-binding expansion.
- OpenGL backend expansion.
- Metal and visual polish.
