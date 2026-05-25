# OPEN1 Created Project Workbench Open Proof

## Status

Closed locally as of 2026-05-26 02:27 Asia/Taipei.

## Trigger

PROJECT1-PROJECT4 closed project creation as a service/proof/status segment. OPEN1 starts the next surface: prove a freshly created project can be opened by the existing workbench session open spine.

This lane does not create a new opener. It uses `createActiveWorkProject()` and the existing `openCurrentWorkbenchSession()` path.

## Acceptance

- A proof runner creates a deterministic work project through `createActiveWorkProject()`.
- The runner opens that created project's `myworld.work.json` through `openCurrentWorkbenchSession()`.
- The runner writes `created_project_open_report.json` into a stable proof directory.
- The report includes creation status, workbench open status, work source/status, created manifest path, opened manifest path, document id, mapping status/count, and status text.
- The app exposes a stable CLI proof flag for this artifact.
- Existing PROJECT creation proof and stable app workbench proof remain unchanged.
- No project picker, app UI/dialog, active-work environment mutation, save mutation, canvas UI, mapping editor, runtime cook loop, OpenGL backend expansion, Metal, analyzer DSP, or visual polish is added.

## Contract

| Question | Answer |
| --- | --- |
| Trigger | Unit test or app startup proof flag asks to prove created-project open. |
| Input | Output directory and repo candidate roots for existing fixture/mapping lookup. |
| Success | The runner creates files, opens the created manifest as an active-work session, writes a JSON report, and returns ok with artifact paths. |
| Failure | Creation, workbench open, mapping load, directory, or write failures return not ok with status/error and report as much as possible. |
| Observability | `created_project_open_report.json` records creation result plus workbench session readback and source status. |

## Target Line

```text
OPEN1 proof flag/test
-> createActiveWorkProject()
-> openCurrentWorkbenchSession(created manifest)
-> created_project_open_report.json
```

## Verification

- Red test: `cmake -S . -B build && cmake --build build --target my_world_created_project_open_proof_runner_tests` failed because `CreatedProjectOpenProofRunner.h` did not exist.
- `cmake --build build --target my_world_created_project_open_proof_runner_tests && ./build/my_world_created_project_open_proof_runner_tests` passed.
- `cmake --build build --target my_world_startup_proof_tests my_world_created_project_open_proof_runner_tests && ./build/my_world_startup_proof_tests && ./build/my_world_created_project_open_proof_runner_tests` passed.
- `cmake --build build --target my-world` passed, with the existing duplicate static-library linker warning.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-created-project-open-proof-and-exit` passed.
- `debug/created-project-open-proof/created_project_open_report.json` read back with `ok: true`, `creationStatus: created`, `openStatus: ready`, `workSource: active-work`, `workSourceStatus: active-work-opened`, `documentId: patch.open1-main`, `graphIOMappingStatus: valid`, `graphIOMappingCount: 1`, `validGraphIOMappingCount: 1`, and `statusText: created project open ready: work.project-open1 -> patch.open1-main source active-work-opened mappings 1/1`.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-project-creation-proof-and-exit` passed; `project_creation_report.json` still reads back with `ok: true`, `status: created`, `duplicateStatus: already-exists`, and the PROJECT status text.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed; stable app workbench proof still reads active-work opened with empty blocking diagnostics.
- `ctest --test-dir build --output-on-failure -R "created_project_open_proof_runner|project_creation_proof_runner|app_workbench_session_proof_runner|startup_proof"` passed 4/4.
- `ctest --test-dir build --output-on-failure` passed 89/89.
- `git diff --check` passed.

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
