# SAVE1 Current Session Save Status

## Status

Closed locally as of 2026-05-26 10:47 Asia/Taipei.

## Trigger

OPEN1-OPEN3 closed the non-UI open segment. The app now has a held workbench session, but the existing save callback still routes through the active-work environment helper. SAVE1 makes save status follow the app-held current session.

This lane may use the existing `saveWork()` storage command. It does not add save UI, project picker, active-work environment mutation, mapping editor, runtime cook loop, OpenGL backend expansion, Metal, analyzer DSP, or visual polish.

## Contract

| Question | Answer |
| --- | --- |
| Trigger | Existing app save callback or controller caller asks to save the currently held workbench session. |
| Input | `WorkbenchAppController` current session snapshot plus a mutable `GraphSession`. |
| Success | Save uses the current session `workManifestPath`, calls existing `saveWork()`, clears dirty graph state through that command, and returns deterministic save status text. |
| Failure | No opened session, blocked session, or missing `workManifestPath` returns a blocked command result without writing through active-work fallback. |
| Observability | Controller status text records `save_work: <status>` or `save_work failed: <reason>`; existing save log remains owned by `saveWork()`. |

## Acceptance

- `WorkbenchAppController` exposes a save-current-session API over the existing held session.
- Saving uses `currentSession().workManifestPath`, not `ActiveWorkService::activeWorkManifestFile()`.
- A dirty graph session saves through existing `saveWork()` and clears dirty state.
- A controller with no opened/valid session returns a failure result and status text.
- `MainComponent::saveActiveWork()` delegates to the controller-held session path.
- Build linkage gives `WorkbenchAppController` the existing storage command dependency without duplicating save logic.
- Existing `saveActiveWorkProject()` behavior and storage command tests remain unchanged.
- No save UI, project picker, active-work environment mutation, mapping editor, runtime cook loop, OpenGL backend expansion, Metal, analyzer DSP, or visual polish is added.

## Target Line

```text
WorkbenchAppController currentSessionSnapshot
-> saveCurrentSession(GraphSession&)
-> saveWork(session, snapshot.workManifestPath)
-> controller/MainComponent save status
```

## Verification

- Red test: `cmake --build build --target my_world_workbench_app_controller_tests` failed because `WorkbenchAppController::saveCurrentSession()` did not exist.
- `cmake --build build --target my_world_workbench_app_controller_tests` passed.
- `./build/my_world_workbench_app_controller_tests` passed.
- `cmake --build build --target my_world_save_work_command_tests my_world_active_work_service_tests my_world_workbench_app_controller_tests my-world` passed, with the existing duplicate static-library linker warning.
- `./build/my_world_save_work_command_tests` passed on retry after one direct chained run hit a temp git-template copy failure outside SAVE1 behavior; `ctest` also passed the same test.
- `./build/my_world_active_work_service_tests` passed.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed.
- `debug/app-workbench-session-proof/workbench_open_status_report.json` read back with `ok: true`, `workSource: active-work`, `workSourceStatus: active-work-opened`, `documentId: patch.c2-main`, `saveStatus: clean`, and empty blocking diagnostics.
- `ctest --test-dir build --output-on-failure -R "workbench_app_controller|save_work_command|active_work_service|app_workbench_session_proof_runner"` passed 4/4.
- `ctest --test-dir build --output-on-failure` passed 89/89.
- `git diff --check` passed.

## Parked

- Save button or menu UI changes.
- Project picker or user-triggered open/save dialogs.
- Active-work environment mutation.
- New save format or storage command behavior.
- Git commit UI/status beyond existing `saveWork()` result.
- Canvas UI or mapping editor.
- Runtime cook loop.
- Shader preview live-binding expansion.
- OpenGL backend expansion.
- Metal, analyzer DSP, and visual polish.
