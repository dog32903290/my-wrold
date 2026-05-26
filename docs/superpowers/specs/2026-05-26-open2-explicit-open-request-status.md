# OPEN2 Explicit Open Request Status

## Status

Closed locally as of 2026-05-26 10:24 Asia/Taipei.

## Trigger

OPEN1 proved a freshly created project can be opened as a workbench session through the existing open spine. OPEN2 adds a non-UI explicit open request/status API so callers can say "open this work manifest" without pretending it is the app's active-work environment.

This lane does not add project picker UI, dialogs, active-work environment mutation, or save integration.

## Acceptance

- `WorkbenchSessionOpenStatus` exposes an explicit open request/result API.
- The request takes a work manifest path plus existing candidate roots and status fields.
- The result returns ok/status/error, a WorkbenchSession snapshot, and deterministic status text.
- Success status uses explicit source wording, not active-work wording.
- Missing or blocked manifest produces explicit failure status text.
- Existing current-session open behavior remains unchanged.
- Existing OPEN1 proof and stable app workbench proof remain unchanged.
- No project picker, app UI/dialog, active-work environment mutation, save mutation, canvas UI, mapping editor, runtime cook loop, OpenGL backend expansion, Metal, analyzer DSP, or visual polish is added.

## Contract

| Question | Answer |
| --- | --- |
| Trigger | A non-UI caller asks to open a specific `myworld.work.json` path. |
| Input | Work manifest path, candidate roots, dirty/save/proof/preview status fields. |
| Success | The explicit manifest opens through the existing workbench session spine and returns a snapshot with explicit source/status wording. |
| Failure | Missing path, bad manifest, missing patch, or mapping failure returns not ok with explicit failure status text. |
| Observability | Unit tests assert explicit success/failure status text and that current active-work open remains unchanged. |

## Target Line

```text
ExplicitWorkbenchOpenRequest
-> openExplicitWorkbenchSession()
-> WorkbenchSessionSnapshot explicit source/status
-> statusText
```

## Verification

- Red test: `cmake --build build --target my_world_workbench_session_open_status_tests` failed because `ExplicitWorkbenchOpenRequest` and `openExplicitWorkbenchSession()` did not exist.
- Follow-up red behavior: `./build/my_world_workbench_session_open_status_tests` failed because a missing explicit manifest still fell back to the fixture.
- `cmake --build build --target my_world_workbench_session_open_status_tests && ./build/my_world_workbench_session_open_status_tests` passed.
- `cmake --build build --target my-world` passed, with the existing duplicate static-library linker warning.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-created-project-open-proof-and-exit` passed.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed.
- `debug/created-project-open-proof/created_project_open_report.json` still read back with `ok: true`, `creationStatus: created`, `openStatus: ready`, `workSource: active-work`, `workSourceStatus: active-work-opened`, `documentId: patch.open1-main`, and valid graph IO mapping counts.
- Stable app workbench proof still wrote active-work opened with empty blocking diagnostics.
- `ctest --test-dir build --output-on-failure -R "workbench_session_open_status|created_project_open_proof_runner|app_workbench_session_proof_runner"` passed 3/3.
- `ctest --test-dir build --output-on-failure` passed 89/89.
- `git diff --check` passed before documentation closeout.

## Parked

- Project picker, app UI, buttons, dialogs, or user-triggered open flow.
- Active-work environment mutation.
- Save command mutation or project save integration.
- Canvas UI or workbench panels.
- Mapping editor.
- Runtime cook loop.
- Shader preview live-binding expansion.
- OpenGL backend expansion.
- Metal and visual polish.
