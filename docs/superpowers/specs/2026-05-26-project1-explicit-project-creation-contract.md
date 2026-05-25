# PROJECT1 Explicit Project Creation Contract

## Status

Closed locally as of 2026-05-26 01:56 Asia/Taipei.

## Trigger

ACTIVE1-ACTIVE3 closed default active-work preparation for app startup, but that path still prepares a fixed debug fixture. PROJECT1 starts a separate project-creation surface: create a new work project on disk from explicit inputs, then prove existing storage loaders can read it.

This lane is not UI. It is a service contract and test proof for later UX or command integration.

## Acceptance

- `ActiveWorkService` exposes a project creation request/result API.
- Given a target directory, work id/title, and patch id/title, the service writes:
  - `myworld.work.json`
  - `patches/main.patch.json`
- The work manifest uses the existing `WorkProjectManifest` shape and points to `patches/main.patch.json`.
- The patch document uses the existing `PatchDocument` shape and a default graph contract.
- The result includes status, manifest path, patch path, and diagnostics.
- Existing `loadWorkProjectManifest()` and `loadMainPatchDocumentForWork()` can read the created project back.
- Existing files are not overwritten unless explicitly requested.
- No app UI, save mutation changes, canvas UI, mapping editor, runtime cook loop, OpenGL backend expansion, Metal, analyzer DSP, or visual polish is added.

## Contract

| Question | Answer |
| --- | --- |
| Trigger | A caller invokes the service API with explicit project creation inputs. |
| Input | Target directory, work id/title, patch id/title, and overwrite flag. |
| Success | The service creates directories/files, writes manifest and main patch JSON, returns ok with paths and diagnostics, and loaders can read the result. |
| Failure | Missing required identity fields fail validation; existing files fail unless overwrite is true; filesystem write/load failures return not ok with error and diagnostics. |
| Observability | Unit test verifies files, loader readback, no-overwrite failure, and diagnostics. Master plan records verification commands. |

## Target Line

```text
CreateActiveWorkProjectRequest
-> ActiveWorkService
-> myworld.work.json + patches/main.patch.json
-> loadWorkProjectManifest + loadMainPatchDocumentForWork
```

## Verification

- Red test: `cmake --build build --target my_world_active_work_service_tests` failed before implementation because `myworld::CreateActiveWorkProjectRequest` and `myworld::createActiveWorkProject()` did not exist.
- `cmake --build build --target my_world_active_work_service_tests` passed.
- `./build/my_world_active_work_service_tests` passed.
- `cmake --build build --target my-world` passed, with the existing duplicate static-library linker warning.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed.
- Stable app proof still writes `debug/app-workbench-session-proof/active_work_preparation_report.json` with `ok: true`, `status: default-active-work-ready`, and `activeWorkPreparationStatus=default-active-work-ready`.
- Stable app proof still writes `debug/app-workbench-session-proof/workbench_open_status_report.json` with `ok: true`, `workSource: active-work`, `workSourceStatus: active-work-opened`, and empty blocking diagnostics.
- `ctest --test-dir build --output-on-failure -R "active_work_service|app_workbench_session_proof_runner|workbench_app_controller"` passed 3/3.
- `ctest --test-dir build --output-on-failure` passed 87/87.
- `git diff --check` passed.

## Parked

- App UI or dialogs.
- Environment variable mutation.
- Save command mutation.
- Canvas UI or workbench panels.
- Mapping editor.
- Runtime cook loop.
- Shader preview live-binding expansion.
- OpenGL backend expansion.
- Metal and visual polish.
