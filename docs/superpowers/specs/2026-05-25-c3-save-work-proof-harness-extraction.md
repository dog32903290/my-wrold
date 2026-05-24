# C3 Save-Work Proof Harness Extraction

Date: 2026-05-25
Status: closed

## Load-Bearing Goal

Move C3 save-work proof orchestration out of `MainComponent` while preserving the existing CLI flag, work fixture, report file, `moveNode -> saveWork` sequence, and persistence evidence.

The app shell may keep:

- startup trigger
- status label text
- quit-after-dump scheduling

The runner must own:

- C2 work fixture candidate lookup
- output directory clearing/creation
- proof work directory fixture copy
- direct `moveNode()` mutation
- direct `saveWork()` persistence command
- saved patch / save log reload checks
- public port, layout, command log, and report evidence
- `save_work_report.json` writing

## Preserved External Contract

CLI:

```text
--dump-c3-save-work-proof-and-exit
```

Artifact:

```text
debug/c3-save-work-proof/save_work_report.json
```

Stable JSON fields:

```text
kind = c3SaveWorkProof
ok
source = PatchDocument
usesInteractionState = false
saveStatus = save-ok commit-pending
commandLogStatus = save_work:save-ok commit-pending
saveLogOk = true
saveLogEntries = 1
saveLogStatus = save-ok commit-pending
commitStatus = not-started
publicInputEdge = true
publicOutputEdge = true
expandedLayout.matches = true
error
```

## Implementation

- `source/app/C3SaveWorkProofRunner.h`
- `source/app/C3SaveWorkProofRunner.cpp`
- `tests/C3SaveWorkProofRunnerTests.cpp`

`MainComponent::dumpC3SaveWorkProof()` now builds a small request, calls `runC3SaveWorkProof()`, and maps the result to UI status text.

## Verification

Red:

```text
cmake -S . -B build
```

Expected failure before runner implementation:

```text
Cannot find source file:
  source/app/C3SaveWorkProofRunner.cpp
```

Green:

```text
cmake -S . -B build
cmake --build build --target my_world_c3_save_work_proof_runner_tests
cmake --build build --target my-world
ctest --test-dir build --output-on-failure -R "c3_save_work_proof_runner|save_work_command|patch_document"
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c3-save-work-proof-and-exit
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted result:

- `c3_save_work_proof_runner` passed.
- `save_work_command` passed.
- `patch_document` passed.
- app target built.
- CLI proof exited 0.
- `save_work_report.json` retained `ok: true`, `saveStatus: "save-ok commit-pending"`, `commandLogStatus: "save_work:save-ok commit-pending"`, `saveLogEntries: 1`, and `commitStatus: "not-started"`.
- Full `ctest` passed 47/47.
- `git diff --check` passed.

## Parked

- C2 proof orchestration remains in `MainComponent`.
- V1 shader proof remains tied to `OpenGLShaderPreview`.
