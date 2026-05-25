# PROJECT3 Project Creation Status Integration

## Status

Closed locally as of 2026-05-26 02:12 Asia/Taipei.

## Trigger

PROJECT2 made project creation dumpable through a proof runner and app flag. PROJECT3 adds the narrow status layer: the creation proof result should carry one human-readable status string that the app adapter can show without opening project creation UI or mutating active work/save state.

## Acceptance

- `ProjectCreationProofRunner` returns a deterministic `statusText`.
- Success status text names the created work id, patch id, and no-overwrite duplicate status.
- Failure status text names the failure reason.
- `project_creation_report.json` includes the same `statusText`.
- `MainComponent::dumpProjectCreationProof()` uses the runner `statusText` for the existing app proof status label.
- Existing PROJECT2 proof artifact fields remain intact.
- Existing stable app workbench proof remains unchanged.
- No app UI/dialogs, save mutation changes, active-work mutation, canvas UI, mapping editor, runtime cook loop, OpenGL backend expansion, Metal, analyzer DSP, or visual polish is added.

## Contract

| Question | Answer |
| --- | --- |
| Trigger | PROJECT2 proof runner result is created, or the app runs `--dump-project-creation-proof-and-exit`. |
| Input | The runner result: ok/status/error, created work/patch readback, duplicate status. |
| Success | A concise status text reports created work, created patch, and duplicate guard. |
| Failure | A concise failed status text reports runner/service error. |
| Observability | Unit test and app proof read back `statusText` from `project_creation_report.json`; app adapter reads the same status text. |

## Target Line

```text
ProjectCreationProofRunner
-> statusText
-> project_creation_report.json
-> MainComponent proof status label adapter
```

## Verification

- Red test: `cmake --build build --target my_world_project_creation_proof_runner_tests` failed because `ProjectCreationProofRunResult` had no `statusText`.
- `cmake --build build --target my_world_project_creation_proof_runner_tests && ./build/my_world_project_creation_proof_runner_tests` passed.
- `cmake --build build --target my-world` passed, with the existing duplicate static-library linker warning.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-project-creation-proof-and-exit` passed.
- `debug/project-creation-proof/project_creation_report.json` read back with `statusText: project creation ready: work.project2-proof -> patch.project2-main; duplicate already-exists`.
- `ctest --test-dir build --output-on-failure -R "project_creation_proof_runner|app_workbench_session_proof_runner"` passed 2/2.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed.
- Stable app workbench proof still wrote `active_work_preparation_report.json` with `ok: true`, `status: default-active-work-ready`, and `activeWorkPreparationStatus=default-active-work-ready`.
- Stable app workbench proof still wrote `workbench_open_status_report.json` with `ok: true`, `workSource: active-work`, `workSourceStatus: active-work-opened`, and empty blocking diagnostics.
- `ctest --test-dir build --output-on-failure` passed 88/88.
- `git diff --check` passed.

## Parked

- New app UI, buttons, dialogs, or project picker.
- Active-work environment mutation.
- Save command mutation.
- Canvas UI or workbench panels.
- Mapping editor.
- Runtime cook loop.
- Shader preview live-binding expansion.
- OpenGL backend expansion.
- Metal and visual polish.
