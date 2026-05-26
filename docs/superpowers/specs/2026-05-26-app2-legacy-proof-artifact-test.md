# APP2 Legacy Proof Artifact Test Cleanup

## Status

Closed locally on 2026-05-26 11:36 Asia/Taipei.

Branch:

```text
codex/app2-legacy-proof-artifact-test
```

## Scope

This cleanup takes over the previously dirty `tests/APP2WorkbenchOpenStatusProofRunnerTests.cpp` file. It does not reopen the APP segment and does not change source code.

## Contract

`APP2WorkbenchOpenStatusProofRunner` is now a legacy wrapper over `AppWorkbenchSessionProofRunner`. The stable runner writes two artifacts:

```text
workbench_open_status_report.json
active_work_preparation_report.json
```

The APP2 legacy test should expect both artifacts so the test matches the current compatibility wrapper behavior.

## Acceptance

- The APP2 legacy proof runner test expects both artifact paths.
- The test verifies `active_work_preparation_report.json` exists and has `kind: activeWorkPreparationReport`.
- No source files are changed.
- The working tree no longer carries this dirty test file after commit.

## Result

The legacy APP2 wrapper test now matches the stable app workbench proof runner artifact contract. The previously dirty test file is intentionally owned by this cleanup.

Closed line:

```text
APP2 legacy wrapper
-> stable AppWorkbenchSessionProofRunner
-> workbench report + active-work preparation report
-> test expectation synced
```

## Verification

```text
cmake --build build --target my_world_app2_workbench_open_status_proof_runner_tests
./build/my_world_app2_workbench_open_status_proof_runner_tests
ctest --test-dir build --output-on-failure -R "app2_workbench_open_status_proof_runner|app_workbench_session_proof_runner"
ctest --test-dir build --output-on-failure
git diff --check
```

All commands passed. Full test result:

```text
100% tests passed, 0 tests failed out of 91
```

## Parked

- Removing legacy APP2 compatibility target/test remains parked.
- No app behavior, UI, save/open/project flow, proof runner source, mapping editor, runtime cook, or Metal work enters this cleanup.
