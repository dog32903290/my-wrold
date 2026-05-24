# ProofRunSupport C5/C6 Cleanup

Date: 2026-05-25
Status: closed

## Load-Bearing Goal

Move C5 and C6 proof file/path primitives onto `ProofRunSupport` while preserving publish, analyzer-family, and repair-loop proof behavior.

This is a support cleanup only. It must not change:

- C5 direct module publish behavior
- C5 AI worker publish_module behavior
- C5 visible module publish behavior
- C6 analyzer family raw-energy proof behavior
- C6 AI repair-loop behavior
- artifact filenames
- stable JSON fields
- MainComponent trigger/status facade

## Scope

`ProofRunSupport` now serves these C5/C6 primitives:

- proof text file writing
- proof output directory clearing
- proof output directory creation
- candidate work/module-library path expansion and deduplication

C5 and C6 runners still own their proof-family orchestration, command execution, report selection, and semantic checks.

## Preserved External Contract

C5 CLI flags and artifacts:

```text
--dump-c5-module-publish-proof-and-exit
debug/c5-module-publish-proof/module_publish_report.json

--dump-c5-ai-worker-module-publish-proof-and-exit
debug/c5-ai-worker-module-publish-proof/ai_worker_module_publish_report.json

--dump-c5-visible-module-publish-proof-and-exit
debug/c5-visible-module-publish-proof/visible_module_publish_report.json
```

C6 CLI flags and artifacts:

```text
--dump-c6-analyzer-family-proof-and-exit
debug/c6-analyzer-family-proof/analyzer_family_report.json

--dump-c6-ai-repair-loop-proof-and-exit
debug/c6-ai-repair-loop-proof/ai_repair_loop_report.json
```

Stable fields retained:

```text
C5 direct: kind, ok, operation, status, publishedNodeType, runtimeCoverageStatus, createdPublishedNode, error
C5 AI: kind, ok, operation, status, publishedNodeType, error
C5 visible: kind, ok, operation, status, publishedNodeType, createdPublishedNode, error
C6 analyzer: kind, ok, familyEntryCount, runtimeCoverageStatus, createdRawEnergyNode, loudnessStillPresent, error
C6 repair: kind, ok, status, attemptsRun, maxAttempts, successfulAttemptIndex, finalOperation, finalCommandLogStatus, graphMutationApplied, collaborationLogEntries, error
```

## Implementation

- `source/app/C5ModulePublishProofRunner.cpp`
- `source/app/C6AnalyzerFamilyProofRunner.cpp`
- `source/app/C6AIRepairLoopProofRunner.cpp`
- `CMakeLists.txt`

All three runner targets now link `my_world_proof_run_support`.

## Verification

```text
cmake -S . -B build
cmake --build build --target my_world_c5_module_publish_proof_runner_tests my_world_c6_analyzer_family_proof_runner_tests my_world_c6_ai_repair_loop_proof_runner_tests my_world_proof_run_support_tests
ctest --test-dir build --output-on-failure -R "proof_run_support|c5_module_publish_proof_runner|c6_analyzer_family_proof_runner|c6_ai_repair_loop_proof_runner|module_publish|ai_worker_command|analyzer_compound_family|runtime_registry|compound_module"
cmake --build build --target my-world
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-module-publish-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-ai-worker-module-publish-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-visible-module-publish-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-analyzer-family-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-ai-repair-loop-proof-and-exit
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted targeted result:

- `proof_run_support` passed.
- `c5_module_publish_proof_runner` passed.
- `c6_analyzer_family_proof_runner` passed.
- `c6_ai_repair_loop_proof_runner` passed.
- focused module publish, AI worker, analyzer family, runtime registry, and compound module tests passed.
- app target built.
- All three C5 CLI proofs exited 0 and kept `ok: true`, expected `kind`, `operation: "publish_module"`, and empty `error`.
- C6 analyzer family CLI proof exited 0 and kept `ok: true`, `familyEntryCount: 2`, `runtimeCoverageStatus: "ready"`, `createdRawEnergyNode: true`, and `loudnessStillPresent: true`.
- C6 AI repair-loop CLI proof exited 0 and kept `ok: true`, `status: "repaired"`, `attemptsRun: 2`, `maxAttempts: 3`, `successfulAttemptIndex: 2`, `finalCommandLogStatus: "ai_worker_repair_loop:repaired"`, `graphMutationApplied: true`, and `collaborationLogEntries: 7`.
- Full `ctest` passed 51/51.
- `git diff --check` passed.

## Parked

- This closes the current proof runner file/path helper cleanup wave.
- `ProofRunSupport` remains a small helper layer, not a generic proof runner.
- Future cleanup should target adapter duplication in `MainComponent`, not proof-family semantics.
