# PROJECT2 Project Creation Proof Artifact

## Status

Closed locally as of 2026-05-26 02:05 Asia/Taipei.

## Trigger

PROJECT1 created the explicit project creation service contract. PROJECT2 adds a narrow proof runner and app dump path so that project creation can be verified through a stable artifact without opening UI, save mutation, canvas editing, or runtime cook work.

## Acceptance

- A PROJECT2 proof runner creates a test work project through `createActiveWorkProject()`.
- The runner writes `project_creation_report.json` into a stable proof directory.
- The report includes kind, ok, status, manifest path, patch path, loader readback fields, and diagnostics.
- The runner test proves the artifact exists, reads back the created work/patch fields, and blocks accidental overwrite through the service result.
- The app exposes a stable CLI proof flag for this artifact.
- Existing stable app workbench proof remains unchanged.
- No app UI/dialogs, save mutation changes, canvas UI, mapping editor, runtime cook loop, OpenGL backend expansion, Metal, analyzer DSP, or visual polish is added.

## Contract

| Question | Answer |
| --- | --- |
| Trigger | Unit test or app startup proof flag asks for project creation proof. |
| Input | Output directory. The runner owns deterministic PROJECT2 test work/patch ids under that output directory. |
| Success | It creates a project directory, writes the report JSON, reads the created manifest/main patch back, and returns ok with artifact paths. |
| Failure | Directory/write/service/load failures return not ok with status/error and still avoid app UI mutation. |
| Observability | `project_creation_report.json` records service status, created paths, readback identities, and diagnostics. |

## Target Line

```text
PROJECT2 proof flag/test
-> ProjectCreationProofRunner
-> createActiveWorkProject()
-> project_creation_report.json
-> loader readback evidence
```

## Verification

- Red test: `cmake -S . -B build && cmake --build build --target my_world_project_creation_proof_runner_tests` failed because `ProjectCreationProofRunner.h` did not exist.
- Repeat-run red test: `./build/my_world_project_creation_proof_runner_tests` failed with `active work project files already exist`.
- `cmake --build build --target my_world_project_creation_proof_runner_tests && ./build/my_world_project_creation_proof_runner_tests` passed.
- `./build/my_world_startup_proof_tests` passed.
- `cmake --build build --target my-world` passed, with the existing duplicate static-library linker warning.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-project-creation-proof-and-exit` passed twice in a row.
- `debug/project-creation-proof/project_creation_report.json` read back with `ok: true`, `status: created`, `manifestExists: true`, `patchExists: true`, `workId: work.project2-proof`, `patchId: patch.project2-main`, `duplicateStatus: already-exists`, and diagnostics including `createActiveWorkProjectStatus=created`.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed.
- Stable app workbench proof still wrote `active_work_preparation_report.json` with `ok: true`, `status: default-active-work-ready`, and `activeWorkPreparationStatus=default-active-work-ready`.
- Stable app workbench proof still wrote `workbench_open_status_report.json` with `ok: true`, `workSource: active-work`, `workSourceStatus: active-work-opened`, and empty blocking diagnostics.
- `ctest --test-dir build --output-on-failure -R "project_creation_proof_runner|startup_proof|app_workbench_session_proof_runner|active_work_service"` passed 4/4.
- `ctest --test-dir build --output-on-failure` passed 88/88.
- `git diff --check` passed.

## Parked

- App UI or dialogs.
- Save command mutation.
- Canvas UI or workbench panels.
- Mapping editor.
- Runtime cook loop.
- Shader preview live-binding expansion.
- OpenGL backend expansion.
- Metal and visual polish.
