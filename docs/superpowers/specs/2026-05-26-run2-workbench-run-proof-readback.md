# RUN2 Workbench Run Proof Readback

## Status

Closed locally on 2026-05-26 14:21 Asia/Taipei.

Branch:

```text
codex/run2-workbench-run-proof-readback
```

## Contract

RUN2 adds an app dump proof for the RUN1 workbench headless run adapter. The proof runner creates a controlled headless-compatible work project, opens it through `WorkbenchAppController`, runs `runWorkbenchHeadlessRender()`, and writes `run_report.json` beside the existing headless artifacts.

This remains a narrow headless proof. It does not add a full scheduler, dirty propagation, shader.fragment execution, audio node execution, graph mutation, save mutation, canvas interactions, mapping editor, Metal, or visual polish.

## One-Line Proof

```text
WorkbenchAppController current session
-> runWorkbenchHeadlessRender()
-> debug/workbench-run-proof/run_report.json + headless artifacts
```

## Acceptance

- `WorkbenchRunProofRunner` owns controlled proof work creation, session open, headless run execution, report serialization, and artifact status.
- Startup proof supports `--dump-workbench-run-proof-and-exit`.
- MainComponent only adapts the startup proof trigger/status to the runner.
- The report proves document id, runtime node/edge counts, active output node, run order, readiness, execution status, headless artifact paths, and empty error on success.
- Unsupported shader execution remains parked and is not hidden by this proof.

## Verification Target

```text
cmake -S . -B build
cmake --build build --target my_world_workbench_run_proof_runner_tests my_world_startup_proof_tests my-world
./build/my_world_workbench_run_proof_runner_tests
./build/my_world_startup_proof_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-workbench-run-proof-and-exit
ctest --test-dir build --output-on-failure -R "workbench_run_proof_runner|startup_proof|workbench_run_surface|headless_render_runtime|workbench_session|workbench_app_controller"
git diff --check
```

## Accepted Result

```text
WorkbenchAppController current session
-> runWorkbenchHeadlessRender()
-> debug/workbench-run-proof/run_report.json + headless artifacts
```

`WorkbenchRunProofRunner` creates a controlled `image.constant -> output.texture_summary`
work project, opens it through `WorkbenchAppController`, runs the existing
`runWorkbenchHeadlessRender()` adapter, and writes `run_report.json` beside:

- `texture_summary.json`
- `cook_order.json`
- `node_stats.json`
- `thumbnail.png`
- `thumbnail_stats.json`
- `errors.json`

The app flag `--dump-workbench-run-proof-and-exit` writes
`debug/workbench-run-proof/run_report.json` with `ok: true`, document
`patch.run2-main`, runtime graph `2 nodes/1 edges`, active output `out1`, run
order `const1 -> out1`, readiness `ready`, execution `ran`, all headless paths,
and empty `error`.

## Verification

```text
cmake -S . -B build
cmake --build build --target my_world_workbench_run_proof_runner_tests my_world_startup_proof_tests my-world
./build/my_world_workbench_run_proof_runner_tests
./build/my_world_startup_proof_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-workbench-run-proof-and-exit
debug/workbench-run-proof/run_report.json written with ok true
ctest --test-dir build --output-on-failure -R "workbench_run_proof_runner|startup_proof|workbench_run_surface|headless_render_runtime|workbench_session|workbench_app_controller"
100% tests passed, 0 tests failed out of 9
```

## Parked

- Full runtime scheduler
- Dirty propagation
- Real shader.fragment execution
- Audio runtime node execution
- Node editor functionality
- Canvas interactions
- Mapping editor
- Save mutation
- Metal / production GPU backend work
- Visual polish
