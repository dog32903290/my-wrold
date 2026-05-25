# APP1 Workbench Session Spine

## Status

Closed as of 2026-05-25.

## Trigger

V1/A1/C1/G1 now have proof lines, but the app still reads like several proven organs rather than one program body. APP1 creates the first non-UI workbench session spine: one state snapshot that gathers the current patch document, graph IO mapping, preview/proof/save status, graph counts, output view, and timeline state.

## Acceptance

- A `WorkbenchSessionSnapshot` can be built from a `PatchDocument`, loaded G1 graph IO mappings, dirty/save/proof/preview status, and work manifest path.
- The snapshot reports document id/title/version, graph counts, dirty state, output target, timeline transport, graph IO mapping count, mapping validity, preview status, proof status, and save status.
- A focused proof runner writes `workbench_session_report.json` from existing fixtures.
- App startup proof command `--dump-app1-workbench-session-proof-and-exit` writes the same report under `debug/app1-workbench-session-proof/`.
- Mapping diagnostics are visible when the session contains invalid graph IO mappings.
- No canvas UI, mapping editor, graph node surface, runtime cook loop, save mutation, audio callback work, Metal, or visual polish is added.

## Target Line

```text
fixtures/storage/c2-compound-work/myworld.work.json
+ fixtures/graphs/g1_loudness_to_shader_uniform.graph.json
-> WorkbenchSession snapshot
-> debug/app1-workbench-session-proof/workbench_session_report.json
```

## Contract

```text
WorkbenchSession answers:
what program state does the current workbench believe is open and provable?

Inputs:
- workManifestPath
- PatchDocument
- GraphIOMapping list
- dirty flag
- saveStatus
- proofStatus
- previewStatus

Success:
- produce a pure snapshot and JSON report.
- write a proof artifact through a runner.

Failure:
- missing work fixture or mapping fixture blocks the proof runner.
- invalid mapping keeps the snapshot readable and records diagnostics.

Observability:
- `workbench_session_report.json` carries stable keys for document, graph counts, mapping status, output/timeline, dirty/save/proof/preview status, and diagnostics.
```

## Verification

- `cmake --build build --target my_world_workbench_session_tests` passed.
- `./build/my_world_workbench_session_tests` passed with `workbench session ok`.
- `cmake --build build --target my_world_app1_workbench_session_proof_runner_tests` passed.
- `./build/my_world_app1_workbench_session_proof_runner_tests` passed with `app1 workbench session proof runner ok`.
- `cmake --build build --target my_world_startup_proof_tests` passed.
- `./build/my_world_startup_proof_tests` passed.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app1-workbench-session-proof-and-exit` passed and wrote `debug/app1-workbench-session-proof/workbench_session_report.json` with `ok: true`.
- `ctest --test-dir build --output-on-failure` passed 81/81.
- `cmake --build build --target my-world` passed.
- `git diff --check` passed.

## Parked

- Workbench UI panels.
- Canvas workbench gestures.
- Mapping editor.
- Runtime cook loop.
- Save command integration.
- AI worker visible task surface.
- Shader preview live-binding expansion.
- Metal and visual polish.
