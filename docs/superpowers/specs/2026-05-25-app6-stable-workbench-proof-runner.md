# APP6 Stable Workbench Proof Runner

## Status

Closed locally as of 2026-05-25.

## Trigger

APP5 added a stable app-level proof command, but internal C++ still routed the app-held session proof through APP2-named request/result APIs. APP6 adds stable `AppWorkbenchSessionProofRunner` names and leaves the APP2 runner as a compatibility wrapper.

## Acceptance

- Stable `AppWorkbenchSessionProofRunRequest` / `Result` and `runAppWorkbenchSessionProof()` exist.
- `WorkbenchAppController` returns stable proof requests.
- `MainComponent` calls the stable app proof runner.
- Existing APP2 proof runner target/test still passes as compatibility.
- App proof JSON shape and stable proof directory remain unchanged.
- No node functionality, canvas UI, mapping editor, runtime cook, save mutation, OpenGL backend expansion, Metal, or visual polish is added.

## Target Line

```text
WorkbenchAppController held snapshot
-> AppWorkbenchSessionProofRunner
-> debug/app-workbench-session-proof/workbench_open_status_report.json

legacy APP2 runner
-> wrapper over AppWorkbenchSessionProofRunner
```

## Contract

```text
APP6 answers:
what stable C++ proof runner should app workbench sessions use?

Inputs:
- AppWorkbenchSessionProofRunRequest
- WorkbenchSessionSnapshot

Success:
- stable runner writes the existing report JSON.
- legacy APP2 wrapper keeps old focused tests and callers working.

Observability:
- focused stable runner test passes.
- legacy APP2 runner test passes.
- controller test and app dump proof pass through stable runner.
```

## Verification

- Red test: `cmake --build build --target my_world_app_workbench_session_proof_runner_tests` failed because `AppWorkbenchSessionProofRunner.h` did not exist.
- `cmake -S . -B build` passed.
- `cmake --build build --target my_world_app_workbench_session_proof_runner_tests my_world_app2_workbench_open_status_proof_runner_tests my_world_workbench_app_controller_tests my-world` passed.
- `./build/my_world_app_workbench_session_proof_runner_tests` passed with `app workbench session proof runner ok`.
- `./build/my_world_app2_workbench_open_status_proof_runner_tests` passed with `app2 workbench open status proof runner ok`.
- `./build/my_world_workbench_app_controller_tests` passed with `workbench app controller ok`.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed and wrote `debug/app-workbench-session-proof/workbench_open_status_report.json` with `ok: true`.
- `ctest --test-dir build --output-on-failure` passed 85/85.
- `git diff --check` passed.

## Parked

- Removing legacy APP2 compatibility target/test.
- Node functionality.
- Canvas UI or workbench panels.
- Mapping editor.
- Runtime cook loop.
- Save mutation.
- Shader preview live-binding expansion.
- OpenGL backend expansion.
- Metal and visual polish.
