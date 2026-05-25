# WORK2 Active Work Resolver

## Status

Closed locally as of 2026-05-25.

## Trigger

WORK1 gave the app a named lifecycle vocabulary for active/fixture/blocked work states, but `WorkbenchSessionOpenStatus` still owned the active-work file check, fixture fallback loop, and PatchDocument load. WORK2 extracts that resolution into `WorkProjectResolver` so open/status can focus on turning a resolved work document plus G1 mappings into a `WorkbenchSessionSnapshot`.

## Acceptance

- `WorkProjectResolver` resolves the current work document from:
  - existing active work manifest
  - missing active work manifest with fixture fallback
  - broken active work manifest that must block
  - missing fixture when no active work is requested
- Resolver returns `PatchDocument`, `workManifestPath`, `activeWorkManifestPath`, lifecycle, and error text.
- `WorkbenchSessionOpenStatus` consumes resolver output instead of owning active-work lookup and fixture fallback.
- Existing workbench open/status behavior remains intact.
- Stable app workbench proof command remains `--dump-app-workbench-session-proof-and-exit`.
- No active-work preparation, save mutation, node functionality, canvas UI, mapping editor, runtime cook, OpenGL backend expansion, Metal, or visual polish is added.

## Target Line

```text
active work path + candidate roots
-> WorkProjectResolver
-> PatchDocument + lifecycle
-> WorkbenchSessionOpenStatus
-> WorkbenchSessionSnapshot
```

## Contract

```text
WORK2 answers:
which work document should the app open, and why?

Inputs:
- activeWorkManifestPath, optional
- candidateRoots
- fallbackWorkManifestPath

Success:
- existing valid active work resolves as active-work-opened.
- missing active work resolves through fixture fallback as fixture-fallback-active-missing.
- no active request can resolve fixture as fixture-fallback-no-active-request.

Failure:
- existing invalid active work resolves as active-work-blocked and does not fall back.
- missing fixture resolves as fixture-blocked-no-active-request or fixture-blocked-active-missing.
- error text remains readable for the caller.

Observability:
- focused resolver test covers active, fallback, broken active, and missing fixture.
- existing open/status test still covers the app session behavior.
- stable app dump proof still reports the resolved work source status.
```

## Verification

- Red test: `cmake -S . -B build && cmake --build build --target my_world_work_project_resolver_tests` failed because `source/app/WorkProjectResolver.cpp` did not exist.
- `cmake -S . -B build && cmake --build build --target my_world_work_project_resolver_tests my_world_workbench_session_open_status_tests` passed.
- `cmake --build build --target my_world_work_project_resolver_tests my_world_work_project_lifecycle_tests my_world_workbench_session_open_status_tests my_world_workbench_app_controller_tests my-world` passed.
- `./build/my_world_work_project_resolver_tests` passed with `work project resolver ok`.
- `./build/my_world_work_project_lifecycle_tests` passed with `work project lifecycle ok`.
- `./build/my_world_workbench_session_open_status_tests` passed with `workbench session open status ok`.
- `./build/my_world_workbench_app_controller_tests` passed with `workbench app controller ok`.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed and wrote `debug/app-workbench-session-proof/workbench_open_status_report.json` with `ok: true` and `workSourceStatus: fixture-fallback-active-missing`.
- `ctest --test-dir build --output-on-failure` passed 87/87.
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
