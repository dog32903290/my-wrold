# STATUS2 App Status Proof Artifact

## Status

Closed locally on 2026-05-26 11:45 Asia/Taipei.

Branch:

```text
codex/status2-app-status-proof-artifact
```

## Contract

STATUS2 adds a file-based app proof for the STATUS1 snapshot. The proof runner exercises the app controller through no-session, open, and save states, then writes `app_status_report.json`.

## Five Questions

| Question | Answer |
| --- | --- |
| Trigger | Unit test calls `runAppStatusProof()`, or the app starts with `--dump-app-status-proof-and-exit`. |
| Input | Output directory and proof candidate roots. The runner creates its own repeatable proof work project. |
| Success | The runner writes `app_status_report.json` with stable initial/open/save status fields derived from `WorkbenchAppController::appStatusSnapshot()`, returns `ok: true`, and `MainComponent` reports the runner status through the existing proof status adapter. |
| Failure | Directory, create/open/load/move/save/write failures return `ok: false`, `status: failed`, and a readable error/status text. |
| Observability | Focused runner test checks report path and stable fields; startup proof test checks the new task selection; app CLI proof writes the artifact under `debug/app-status-proof`. |

## Acceptance

- `AppStatusProofRunner` owns fixture creation, controller exercise, report serialization, and artifact writing.
- `app_status_report.json` includes no-session, open, and save snapshot evidence.
- Startup proof options include an app status task and stable command-line flag.
- `MainComponent` delegates app status proof dumping to the runner.
- Existing app workbench/session/save proof paths stay compatible.

## Result

`AppStatusProofRunner` writes `app_status_report.json` and the app accepts `--dump-app-status-proof-and-exit`. The report proves a readable blocked no-session snapshot, an opened active-work snapshot, and a saved snapshot from `WorkbenchAppController::appStatusSnapshot()`.

Closed line:

```text
WorkbenchAppController::appStatusSnapshot()
-> AppStatusProofRunner
-> MainComponent startup proof adapter
-> debug/app-status-proof/app_status_report.json
```

## Verification

```text
cmake -S . -B build
cmake --build build --target my_world_app_status_proof_runner_tests my_world_startup_proof_tests my-world
./build/my_world_app_status_proof_runner_tests
./build/my_world_startup_proof_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-status-proof-and-exit
ctest --test-dir build --output-on-failure -R "app_status_proof_runner|startup_proof|workbench_app_controller|app_save_proof_runner|app_workbench_session_proof_runner"
git diff --check
```

All commands passed. App proof report readback:

```text
debug/app-status-proof/app_status_report.json
kind: appStatusReport
ok: true
statusText: app status ready: patch.status2-main source active-work-opened save save-ok commit-pending
initialStatusText: workbench blocked: no session
openedStatusText: workbench ready patch.status2-main source active-work-opened mappings 1/1
savedStatusText: save_work: save-ok commit-pending
```

## Parked

- STATUS segment closure is STATUS3.
- Save UI, project picker, active-work environment mutation, new save format, canvas UI, mapping editor, runtime cook, OpenGL backend expansion, Metal, analyzer DSP, browser polish, and visual polish remain parked.
