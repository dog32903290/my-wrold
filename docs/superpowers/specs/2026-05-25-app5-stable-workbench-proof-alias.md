# APP5 Stable Workbench Proof Alias

## Status

Closed locally as of 2026-05-25.

## Trigger

APP3 and APP4 kept using the APP2-named dump command even after the proof became the general app-held workbench session proof. APP5 adds a stable app-level proof command and directory while preserving the old APP2 flag as a compatibility alias.

## Acceptance

- Startup proof options include a stable app workbench session proof task.
- New CLI flag `--dump-app-workbench-session-proof-and-exit` writes the app-held workbench session report.
- Stable proof directory is `debug/app-workbench-session-proof/`.
- Existing `--dump-app2-workbench-open-status-proof-and-exit` still works and writes the same app-held session report through the same app proof path.
- No node functionality, canvas UI, mapping editor, runtime cook, save mutation, OpenGL backend expansion, Metal, or visual polish is added.

## Target Line

```text
--dump-app-workbench-session-proof-and-exit
or legacy --dump-app2-workbench-open-status-proof-and-exit
-> WorkbenchAppController held snapshot
-> debug/app-workbench-session-proof/workbench_open_status_report.json
```

## Contract

```text
APP5 answers:
what stable proof command should future app-spine work use?

Inputs:
- startup proof command-line flags
- WorkbenchAppController held snapshot

Success:
- stable flag schedules appWorkbenchSession startup proof task.
- legacy APP2 flag remains accepted.
- proof JSON shape stays the same.

Observability:
- StartupProof focused test covers stable task.
- app proof dump verifies stable flag.
- legacy flag is smoke-tested for compatibility.
```

## Verification

- Red test: `cmake --build build --target my_world_startup_proof_tests` failed because `dumpAPPWorkbenchSessionProof` and `StartupProofTaskId::appWorkbenchSession` did not exist.
- `cmake --build build --target my_world_startup_proof_tests my_world_app2_workbench_open_status_proof_runner_tests my-world` passed.
- `./build/my_world_startup_proof_tests` passed.
- `./build/my_world_app2_workbench_open_status_proof_runner_tests` passed with `app2 workbench open status proof runner ok`.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit` passed and wrote `debug/app-workbench-session-proof/workbench_open_status_report.json` with `ok: true`.
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app2-workbench-open-status-proof-and-exit` passed as the legacy alias.
- `ctest --test-dir build --output-on-failure` passed 84/84.
- `git diff --check` passed.

## Parked

- Removing legacy APP2 names from internal test target names.
- Node functionality.
- Canvas UI or workbench panels.
- Mapping editor.
- Runtime cook loop.
- Save mutation.
- Shader preview live-binding expansion.
- OpenGL backend expansion.
- Metal and visual polish.
