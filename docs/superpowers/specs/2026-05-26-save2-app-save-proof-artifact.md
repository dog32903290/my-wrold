# SAVE2 App Save Proof Artifact

## Status

Closed locally as of 2026-05-26 10:56 Asia/Taipei.

## Trigger

SAVE1 routed the existing app save action through `WorkbenchAppController::saveCurrentSession()`. SAVE2 adds a stable app-level proof artifact for that path so the save segment has readback evidence outside a unit test.

This lane adds a proof runner and startup proof flag only. It does not add save UI, project picker, active-work environment mutation, new save format, mapping editor, runtime cook loop, OpenGL backend expansion, Metal, analyzer DSP, or visual polish.

## Contract

| Question | Answer |
| --- | --- |
| Trigger | Non-UI proof caller or app startup proof flag asks to prove app current-session save. |
| Input | Output directory plus repo candidate roots for fixture/mapping lookup. |
| Success | Runner creates a repeatable proof work, opens it through `WorkbenchAppController`, dirties a `GraphSession`, saves through `saveCurrentSession()`, reloads evidence, and writes `app_save_report.json`. |
| Failure | Directory, create/open, save, reload, or artifact write failure returns `ok: false`, status `failed`, and a readable error/status text. |
| Observability | Report records open/save status, controller status text, manifest/patch/save-log paths, dirty before/after, command log evidence, document id, and source status. |

## Acceptance

- `AppSaveProofRunner` owns proof orchestration and report writing.
- App proof flag `--dump-app-save-proof-and-exit` writes `debug/app-save-proof/app_save_report.json`.
- Proof uses `WorkbenchAppController::saveCurrentSession()`, not direct `saveWork()` from `MainComponent`.
- Report proves dirty-before-save and clean-after-save.
- Report proves command log contains `save_work:save-ok commit-pending`.
- Existing SAVE1 controller save behavior, stable app workbench proof, and storage command tests remain unchanged.
- No save UI, project picker, active-work environment mutation, new save format, mapping editor, runtime cook loop, OpenGL backend expansion, Metal, analyzer DSP, or visual polish is added.

## Target Line

```text
AppSaveProofRunner
-> create proof work
-> WorkbenchAppController.openCurrentSession()
-> WorkbenchAppController.saveCurrentSession()
-> app_save_report.json
-> --dump-app-save-proof-and-exit
```

## Verification

- Red test: `cmake -S . -B build` failed because `source/app/AppSaveProofRunner.cpp` did not exist after adding the SAVE2 test/CMake target.
- `cmake -S . -B build && cmake --build build --target my_world_app_save_proof_runner_tests && ./build/my_world_app_save_proof_runner_tests` passed.
- Startup proof red test: `cmake --build build --target my_world_startup_proof_tests` failed because `StartupProofOptions::dumpAppSaveProof` and `StartupProofTaskId::appSave` did not exist.
- `cmake --build build --target my_world_startup_proof_tests my_world_app_save_proof_runner_tests my-world` passed, with the existing duplicate static-library linker warning.
- `./build/my_world_startup_proof_tests && ./build/my_world_app_save_proof_runner_tests` passed.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-save-proof-and-exit` passed.
- `debug/app-save-proof/app_save_report.json` read back with `ok: true`, `openStatus: ready`, `saveStatus: save-ok commit-pending`, `controllerStatusText: save_work: save-ok commit-pending`, `documentId: patch.save2-main`, `workSourceStatus: active-work-opened`, `dirtyBeforeSave: true`, `dirtyAfterSave: false`, `commandLogContainsSaveWork: true`, `reloadedAfterSave: true`, and empty `error`.
- Follow-up red test: `./build/my_world_app_save_proof_runner_tests` failed until successful reports wrote empty `error`.
- `ctest --test-dir build --output-on-failure -R "app_save_proof_runner|startup_proof|workbench_app_controller|save_work_command|app_workbench_session_proof_runner"` passed 5/5.
- `ctest --test-dir build --output-on-failure` passed 90/90.

## Parked

- Save button or menu UI changes.
- Project picker or save/open dialogs.
- Active-work environment mutation.
- New save format or storage command behavior.
- Git commit UI/status beyond existing `saveWork()` result.
- Canvas UI or mapping editor.
- Runtime cook loop.
- Shader preview live-binding expansion.
- OpenGL backend expansion.
- Metal, analyzer DSP, and visual polish.
