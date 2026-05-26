# UI2 Status Surface Proof Readback

## Status

Closed locally on 2026-05-26 11:55 Asia/Taipei.

Branch:

```text
codex/ui2-status-surface-proof-readback
```

## Contract

UI2 adds a proof runner and app startup flag that read back the same `WorkbenchStatusSurface` model used by the app labels. This lane proves the visible work/source/save/mapping/proof/preview surface through JSON evidence; it does not add new UI controls or mutate app/work/project state beyond the existing proof fixture save action.

## UI Skin Pressure Gate

```text
1. This UI reads from: WorkbenchAppController::appStatusSnapshot() through WorkbenchStatusSurface.
2. This UI mutates through: no UI mutation; proof runner uses the existing save_work command path only inside its proof fixture.
3. This is proven by: WorkbenchStatusSurfaceProofRunner test, startup proof adapter test, app proof dump readback.
```

## Acceptance

- `WorkbenchStatusSurfaceProofRunner` creates a proof work, opens it through `WorkbenchAppController`, saves the current graph session through the existing controller save path, and builds a `WorkbenchStatusSurface` from the saved app status snapshot.
- The runner writes `workbench_status_surface_report.json` with stable status fields and the six surface row texts.
- The app accepts `--dump-workbench-status-surface-proof-and-exit` and dumps the same report under `debug/workbench-status-surface-proof`.
- `MainComponent` remains a trigger/status adapter; report schema, fixture setup, and artifact writing live in the runner.
- No save UI, project picker, active-work env mutation, canvas node surface, mapping editor, runtime cook, OpenGL expansion, Metal, analyzer DSP, or visual polish enters this lane.

## Result

Closed line:

```text
WorkbenchAppController::appStatusSnapshot()
-> WorkbenchStatusSurface
-> WorkbenchStatusSurfaceProofRunner
-> debug/workbench-status-surface-proof/workbench_status_surface_report.json
```

The app proof readback contains `ok: true`, `headline: save_work: save-ok commit-pending`, `rowCount: 6`, `documentId: patch.ui2-main`, `workSourceStatus: active-work-opened`, `saveStatus: save-ok commit-pending`, `graphIOMappingStatus: valid`, `validGraphIOMappingCount: 1`, `commandLogContainsSaveWork: true`, and the six row texts used by the visible surface.

## Verification Target

```text
cmake -S . -B build
cmake --build build --target my_world_workbench_status_surface_proof_runner_tests my_world_startup_proof_tests my-world
./build/my_world_workbench_status_surface_proof_runner_tests
./build/my_world_startup_proof_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-workbench-status-surface-proof-and-exit
ctest --test-dir build --output-on-failure -R "workbench_status_surface_proof_runner|startup_proof|workbench_status_surface|workbench_app_controller|app_status_proof_runner"
git diff --check
```

All commands passed. The app proof command wrote:

```text
debug/workbench-status-surface-proof/workbench_status_surface_report.json
```

## Parked

- UI3 UI segment closure.
- Save UI, project picker, active-work environment mutation, new save format, canvas node surface, mapping editor, runtime cook, OpenGL backend expansion, Metal, analyzer DSP, browser polish, and visual polish remain parked.
