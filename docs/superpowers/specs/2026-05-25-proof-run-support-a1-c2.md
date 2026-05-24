# ProofRunSupport A1/C2 Cleanup

Date: 2026-05-25
Status: closed

## Load-Bearing Goal

Move the remaining A1 and C2 proof file/path primitives onto `ProofRunSupport` while preserving the existing proof runners, CLI flags, artifact names, and report schemas.

This is a support cleanup only. It must not change:

- A1 audio snapshot/runtime execution behavior
- C2 patch save/reload behavior
- artifact filenames
- stable JSON fields
- MainComponent trigger/status facade

## Scope

`ProofRunSupport` now serves these A1/C2 primitives:

- proof text file writing
- proof output directory creation
- candidate fixture path expansion and deduplication

A1 and C2 still own their proof-family orchestration and semantic checks.

## Preserved External Contract

A1:

```text
--dump-audio-proof-and-exit
debug/a1-audio-proof/audio_stats.json
debug/a1-audio-proof/loudness_compound.json
debug/a1-audio-proof/loudness_runtime_execution.json
debug/a1-audio-proof/loudness_runtime_bridge.json
```

C2:

```text
--dump-c2-storage-proof-and-exit
debug/c2-storage-proof/reload_report.json
debug/c2-storage-proof/saved_main.patch.json
```

Stable fields retained in proof output:

```text
A1: audio_stats sampleRate / bufferSize / rms / peak / loudness / gate / confidence / active / midi / sampleCounter
A1: loudness_runtime_execution kind = runtimeExecution, nodeType = compound.loudness, computed child entries
C2: kind = c2StorageProof, ok, source = PatchDocument, usesInteractionState = false, saveStatus, publicInputEdge, publicOutputEdge, expandedLayout.matches, error
```

## Implementation

- `source/app/A1AudioProofRunner.cpp`
- `source/app/C2StorageProofRunner.cpp`
- `CMakeLists.txt`

Both runner targets now link `my_world_proof_run_support`.

## Verification

```text
cmake -S . -B build
cmake --build build --target my_world_a1_audio_proof_runner_tests my_world_c2_storage_proof_runner_tests my_world_proof_run_support_tests
ctest --test-dir build --output-on-failure -R "proof_run_support|a1_audio_proof_runner|c2_storage_proof_runner|runtime_registry|performance_preferences|storage_contract|patch_document"
cmake --build build --target my-world
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-audio-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c2-storage-proof-and-exit
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted targeted result:

- `proof_run_support` passed.
- `a1_audio_proof_runner` passed.
- `c2_storage_proof_runner` passed.
- `runtime_registry` passed.
- `performance_preferences` passed.
- `storage_contract` passed.
- `patch_document` passed.
- app target built.
- A1 CLI proof exited 0 and kept `audio_stats.json`, `loudness_runtime_execution.json`, and runtime `nodeType: "compound.loudness"` evidence.
- C2 CLI proof exited 0 and kept `ok: true`, `saveStatus: "save-ok commit-pending"`, public input/output edges, and matching expanded layout.
- Full `ctest` passed 51/51.
- `git diff --check` passed.

## Parked

- Do not migrate every remaining proof runner in one sweep.
- C5/C6/PV/PV-B1 helper cleanup stays separate selected slices.
- This remains support extraction, not a new generic proof runner.
