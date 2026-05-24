# C2 Storage Proof Harness Extraction

Date: 2026-05-25
Status: closed

## Load-Bearing Goal

Move C2 storage proof orchestration out of `MainComponent` while preserving the existing CLI flag, fixture lookup, report file, saved patch artifact, and reload evidence.

The app shell may keep:

- startup trigger
- status label text
- quit-after-dump scheduling

The runner must own:

- C2 work fixture candidate lookup
- output directory creation
- `PatchDocument` save to `saved_main.patch.json`
- saved patch reload check
- loudness compound fixture reload for expanded layout proof
- public port, layout, graph count, and report evidence
- `reload_report.json` writing

## Preserved External Contract

CLI:

```text
--dump-c2-storage-proof-and-exit
```

Artifacts:

```text
debug/c2-storage-proof/reload_report.json
debug/c2-storage-proof/saved_main.patch.json
```

Stable JSON fields:

```text
kind = c2StorageProof
ok
source = PatchDocument
usesInteractionState = false
saveStatus = save-ok commit-pending
editorNodeCount = 5
editorEdgeCount = 3
runtimeNodeCount = 5
runtimeEdgeCount = 3
publicInputEdge = true
publicOutputEdge = true
expandedLayout.matches = true
error
```

## Implementation

- `source/app/C2StorageProofRunner.h`
- `source/app/C2StorageProofRunner.cpp`
- `tests/C2StorageProofRunnerTests.cpp`

`MainComponent::dumpC2StorageProof()` now builds a small request, calls `runC2StorageProof()`, and maps the result to UI status text.

## Verification

Red:

```text
cmake -S . -B build
```

Expected failure before runner implementation:

```text
Cannot find source file:
  source/app/C2StorageProofRunner.cpp
```

Green:

```text
cmake -S . -B build
cmake --build build --target my_world_c2_storage_proof_runner_tests
cmake --build build --target my-world
ctest --test-dir build --output-on-failure -R "c2_storage_proof_runner|storage_contract|patch_document"
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c2-storage-proof-and-exit
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted result:

- `c2_storage_proof_runner` passed.
- `storage_contract` passed.
- `patch_document` passed.
- app target built.
- CLI proof exited 0.
- `reload_report.json` retained `ok: true`, `saveStatus: "save-ok commit-pending"`, public input/output edges, matching expanded layout, and 5/3 editor/runtime node/edge counts.
- Full `ctest` passed 48/48.
- `git diff --check` passed.

## Parked

- V1 shader proof remains tied to `OpenGLShaderPreview`.
- A1 audio proof remains in `MainComponent`.
