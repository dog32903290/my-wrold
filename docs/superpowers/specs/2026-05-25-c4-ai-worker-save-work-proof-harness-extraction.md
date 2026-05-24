# C4 AI Worker Save-Work Proof Harness Extraction

Date: 2026-05-25
Status: closed

## Load-Bearing Goal

Move C4 AI worker save-work proof orchestration out of `MainComponent` while preserving the existing CLI flag, work fixture, report file, move/save command sequence, and persistence evidence.

The app shell may keep:

- startup trigger
- status label text
- quit-after-dump scheduling

The runner must own:

- C2 work fixture candidate lookup
- output directory clearing/creation
- proof work directory fixture copy
- AI worker `move_node` command execution
- AI worker `save_work` command execution
- saved patch / save log reload checks
- public port, layout, command log, collaboration log, and report evidence
- `ai_worker_save_work_report.json` writing

## Preserved External Contract

CLI:

```text
--dump-c4-ai-worker-save-work-proof-and-exit
```

Artifact:

```text
debug/c4-ai-worker-save-work-proof/ai_worker_save_work_report.json
```

Stable JSON fields:

```text
kind = c4AIWorkerSaveWorkProof
ok
allowedSaveWork = true
allowedMoveNode = true
operation = save_work
status = save-ok commit-pending
moveStatus = ok
graphCommandLogStatus = move_node
graphMutationApplied = true
storageCommandLogStatus = save_work:save-ok commit-pending
aiCommandLogStatus = ai_worker:save_work:save-ok commit-pending
patchReloaded = true
saveLogOk = true
saveLogStatus = save-ok commit-pending
collaborationLogEntries = 4
publicInputEdge = true
publicOutputEdge = true
savedMove.matches = true
expandedLayout.matches = true
error
```

## Implementation

- `source/app/C4AIWorkerSaveWorkProofRunner.h`
- `source/app/C4AIWorkerSaveWorkProofRunner.cpp`
- `tests/C4AIWorkerSaveWorkProofRunnerTests.cpp`

`MainComponent::dumpC4AIWorkerSaveWorkProof()` now builds a small request, calls `runC4AIWorkerSaveWorkProof()`, and maps the result to UI status text.

## Verification

Red:

```text
cmake -S . -B build
```

Expected failure before runner implementation:

```text
Cannot find source file:
  source/app/C4AIWorkerSaveWorkProofRunner.cpp
```

Green:

```text
cmake -S . -B build
cmake --build build --target my_world_c4_ai_worker_save_work_proof_runner_tests
cmake --build build --target my-world
ctest --test-dir build --output-on-failure -R "c4_ai_worker_save_work_proof_runner|ai_worker_command|save_work_command"
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c4-ai-worker-save-work-proof-and-exit
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted result:

- `c4_ai_worker_save_work_proof_runner` passed.
- `ai_worker_command` passed.
- `save_work_command` passed.
- app target built.
- CLI proof exited 0.
- `ai_worker_save_work_report.json` retained `ok: true`, `status: "save-ok commit-pending"`, `graphMutationApplied: true`, `patchReloaded: true`, and `collaborationLogEntries: 4`.
- Full `ctest` passed 46/46.
- `git diff --check` passed.

## Parked

- C2-C3 proof orchestration remains in `MainComponent`.
- V1 shader proof remains tied to `OpenGLShaderPreview`.
