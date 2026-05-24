# C6 AI Repair Loop Proof Harness Extraction

Date: 2026-05-25
Status: closed

## Load-Bearing Goal

Move C6 AI repair-loop proof orchestration out of `MainComponent` while preserving the existing CLI flag, report file, retry plan semantics, and repair evidence.

The app shell may keep:

- startup trigger
- status label text
- quit-after-dump scheduling

The runner must own:

- C2 work fixture candidate lookup
- output directory clearing/creation
- failed attempt + repaired attempt + unused attempt plan
- `executeAIWorkerRepairLoop()` proof execution
- final node / graph mutation / collaboration evidence checks
- `ai_repair_loop_report.json` writing

## Preserved External Contract

CLI:

```text
--dump-c6-ai-repair-loop-proof-and-exit
```

Artifact:

```text
debug/c6-ai-repair-loop-proof/ai_repair_loop_report.json
```

Stable JSON fields:

```text
kind = c6AIRepairLoopProof
ok
operation = ai_repair_loop
repairId = c6.2-ai-repair-loop
status = repaired
attemptsRun = 2
maxAttempts = 3
firstAttemptStatus = failed
successfulAttemptIndex = 2
finalCommandLogStatus = ai_worker_repair_loop:repaired
graphMutationApplied = true
collaborationLogEntries = 7
error
```

## Implementation

- `source/app/C6AIRepairLoopProofRunner.h`
- `source/app/C6AIRepairLoopProofRunner.cpp`
- `tests/C6AIRepairLoopProofRunnerTests.cpp`

`MainComponent::dumpC6AIRepairLoopProof()` now builds a small request, calls `runC6AIRepairLoopProof()`, and maps the result to UI status text.

## Verification

Red:

```text
cmake -S . -B build
```

Expected failure before runner implementation:

```text
Cannot find source file:
  source/app/C6AIRepairLoopProofRunner.cpp
```

Green:

```text
cmake -S . -B build
cmake --build build --target my_world_c6_ai_repair_loop_proof_runner_tests
cmake --build build --target my-world
ctest --test-dir build --output-on-failure -R "c6_ai_repair_loop_proof_runner|ai_worker_command"
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-ai-repair-loop-proof-and-exit
```

Accepted result:

- `c6_ai_repair_loop_proof_runner` passed.
- `ai_worker_command` passed.
- app target built.
- CLI proof exited 0.
- `ai_repair_loop_report.json` retained `ok: true`, `status: "repaired"`, `attemptsRun: 2`, and `collaborationLogEntries: 7`.

## Parked

- C2-C4 proof orchestration remains in `MainComponent`.
- V1 shader proof remains tied to `OpenGLShaderPreview`.
