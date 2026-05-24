# ProofRunSupport C3/C4 Cleanup

Date: 2026-05-25
Status: closed

## Load-Bearing Goal

Remove duplicated proof file/path primitives from the C3 and C4 proof runners without creating a new all-knowing proof runner.

`ProofRunSupport` may own:

- proof text file writing
- proof output directory clearing/creation
- candidate fixture path expansion and deduplication
- first-existing-candidate copy into proof work directories

It must not own:

- proof-family orchestration
- command execution
- report schema decisions
- UI status / app quit behavior
- C3/C4 semantic checks

## Preserved External Contract

No new CLI flags or artifacts were introduced.

The existing C3 contract remains:

```text
--dump-c3-save-work-proof-and-exit
debug/c3-save-work-proof/save_work_report.json
```

The existing C4 contract remains:

```text
--dump-c4-ai-worker-save-work-proof-and-exit
debug/c4-ai-worker-save-work-proof/ai_worker_save_work_report.json
```

Stable fields retained in the CLI reports:

```text
C3: kind, ok, saveStatus, commandLogStatus, saveLogEntries, commitStatus, expandedLayout.matches, error
C4: kind, ok, status, graphMutationApplied, patchReloaded, collaborationLogEntries, savedMove.matches, expandedLayout.matches, error
```

## Implementation

- `source/app/ProofRunSupport.h`
- `source/app/ProofRunSupport.cpp`
- `tests/ProofRunSupportTests.cpp`
- `source/app/C3SaveWorkProofRunner.cpp`
- `source/app/C4AIWorkerSaveWorkProofRunner.cpp`

`C3SaveWorkProofRunner` and `C4AIWorkerSaveWorkProofRunner` still own their proof flows. They now call shared support primitives for file/directory/candidate/copy behavior.

## Verification

Red:

```text
cmake -S . -B build
```

Expected failure before support implementation:

```text
Cannot find source file:
  source/app/ProofRunSupport.cpp
```

Green:

```text
cmake -S . -B build
cmake --build build --target my_world_proof_run_support_tests my_world_c3_save_work_proof_runner_tests my_world_c4_ai_worker_save_work_proof_runner_tests
ctest --test-dir build --output-on-failure -R "proof_run_support|c3_save_work_proof_runner|c4_ai_worker_save_work_proof_runner|save_work_command|ai_worker_command"
cmake --build build --target my-world
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c3-save-work-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c4-ai-worker-save-work-proof-and-exit
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted result:

- `proof_run_support` passed.
- `c3_save_work_proof_runner` passed.
- `c4_ai_worker_save_work_proof_runner` passed.
- `save_work_command` passed.
- `ai_worker_command` passed.
- app target built.
- C3 CLI proof exited 0 and kept `ok: true`, `saveStatus: "save-ok commit-pending"`, `commandLogStatus: "save_work:save-ok commit-pending"`, `saveLogEntries: 1`, and `commitStatus: "not-started"`.
- C4 CLI proof exited 0 and kept `ok: true`, `status: "save-ok commit-pending"`, `graphMutationApplied: true`, `patchReloaded: true`, and `collaborationLogEntries: 4`.
- Full `ctest` passed 51/51.
- `git diff --check` passed.

## Parked

- Do not migrate every proof runner in one sweep.
- C2/A1/V1 and other proof helper cleanup can move later only as selected small slices.
- This is support extraction, not a new generic `ProofRunner`.
