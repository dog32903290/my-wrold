# APP2 Workbench Open / Status Spine

## Status

Closed locally as of 2026-05-25.

## Trigger

APP1 proved a pure `WorkbenchSession` snapshot and proof runner, but the running app still did not own that session as current state. APP2 makes the app body open and hold the current workbench session on startup, using active work when present and the C2 fixture fallback otherwise, plus the closed G1 mapping fixture.

## Acceptance

- A pure open/status loader builds the current `WorkbenchSessionSnapshot` from active work if it exists.
- If active work is absent, the loader falls back to the C2 compound work fixture without mutating or preparing debug active work.
- G1 graph IO mapping fixture is loaded into the current session and reports valid mapping status.
- `MainComponent` stores `currentWorkbenchSession` and reads it for status text.
- App startup proof command `--dump-app2-workbench-open-status-proof-and-exit` writes `debug/app2-workbench-open-status-proof/workbench_open_status_report.json` from the app-held snapshot.
- No node functionality, canvas UI, mapping editor, runtime cook, save mutation, OpenGL backend expansion, Metal, or visual polish is added.

## Target Line

```text
active work manifest if present
or fixtures/storage/c2-compound-work/myworld.work.json
+ fixtures/graphs/g1_loudness_to_shader_uniform.graph.json
-> openCurrentWorkbenchSession()
-> MainComponent::currentWorkbenchSession
-> debug/app2-workbench-open-status-proof/workbench_open_status_report.json
```

## Contract

```text
APP2 answers:
what workbench session does the running app believe is currently open?

Inputs:
- activeWorkManifestPath
- candidate repo roots
- C2 work fixture fallback
- G1 graph IO mapping fixture
- save/proof/preview status strings

Success:
- current session snapshot is ready.
- status text is readable from the snapshot.
- app dump proof writes the same snapshot the app holds.

Failure:
- unreadable active work fails if explicitly present.
- missing active work falls back to fixture.
- missing fixture or G1 mapping fixture blocks the current session proof.

Observability:
- `workbench_open_status_report.json` carries work source, work manifest path, G1 mapping source path, document identity, graph counts, mapping validity, output/timeline, dirty/save/proof/preview status, and diagnostics.
```

## Verification

- `cmake -S . -B build` passed.
- `cmake --build build --target my_world_workbench_session_open_status_tests my_world_app2_workbench_open_status_proof_runner_tests my_world_startup_proof_tests` passed.
- `./build/my_world_workbench_session_open_status_tests` passed with `workbench session open status ok`.
- `./build/my_world_app2_workbench_open_status_proof_runner_tests` passed with `app2 workbench open status proof runner ok`.
- `./build/my_world_startup_proof_tests` passed.
- `cmake --build build --target my-world` passed.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app2-workbench-open-status-proof-and-exit` passed and wrote `debug/app2-workbench-open-status-proof/workbench_open_status_report.json` with `ok: true`, `status: ready`, `workSource: fixture`, and `graphIOMappingStatus: valid`.
- `ctest --test-dir build --output-on-failure` passed 83/83.
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
