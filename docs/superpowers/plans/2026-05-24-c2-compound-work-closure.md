# C2 Compound Work Closure Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Close C2 by proving saved compound works use the formal `PatchDocument` path from active `GraphSession` save to file reload and app proof evidence.

**Architecture:** Keep `GraphContract` as the in-memory graph truth and `PatchDocument` as the durable saved patch boundary. C2 adds file save/load, work-manifest main-patch loading, and proof dumps; local git auto-commit, AI worker save commands, and multi-patch work libraries remain parked.

**Tech Stack:** C++20, CMake, JUCE app proof command, existing `StorageContract`, `InteractionContract`, `CompoundPatch`, and focused executable tests.

---

## C2 End Condition

```text
GraphSession
-> PatchDocument file
-> WorkProject main patch reload
-> new GraphSession/editorGraph/runtimeGraph
-> collapsed compound public ports + expanded child layout match
-> app proof dump writes reload evidence
```

C2 is closed only when the formal patch-document path can replace the old `interaction-state-v1` proof for compound work save/reload. C2 does not claim local git commit, AI worker collaboration, raw audio callback runtime, module publishing, or multi-work library sync.

### Task 1: C2.2 Active Patch File Roundtrip

**Files:**
- Modify: `source/storage/StorageContract.h`
- Modify: `source/storage/StorageContract.cpp`
- Modify: `tests/PatchDocumentTests.cpp`

- [x] Add a failing test that saves a `PatchDocument` generated from a dirty `GraphSession` to a real file path, reloads it with `loadPatchDocument`, creates a fresh `GraphSession`, and checks root public-port edges plus the expanded `mono_mix` layout.
- [x] Add `PatchDocumentSaveResult` and `savePatchDocument(path, document)` with explicit `validation-failed`, `write-failed`, and `save-ok commit-pending` statuses.
- [x] Run `cmake --build build --target my_world_patch_document_tests && ./build/my_world_patch_document_tests`.

### Task 2: C2.3 Work Manifest Main Patch Reload

**Files:**
- Modify: `source/storage/StorageContract.h`
- Modify: `source/storage/StorageContract.cpp`
- Modify: `tests/PatchDocumentTests.cpp`

- [x] Add `WorkProjectLoadResult`, `parseWorkProjectManifest`, `loadWorkProjectManifest`, and `loadMainPatchDocumentForWork`.
- [x] Test `fixtures/storage/c2-compound-work/myworld.work.json -> patches/main.patch.json -> PatchDocument -> GraphSession`.
- [x] Verify module-library references stay in the work manifest and are not copied into the patch document.

### Task 3: C2.4 App Storage Proof Dump

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `source/app/Main.cpp`
- Modify: `source/app/MainComponent.h`
- Modify: `source/app/MainComponent.cpp`

- [x] Add `--dump-c2-storage-proof-and-exit`.
- [x] Load the C2 work fixture, save the active graph back to `debug/c2-storage-proof/saved_main.patch.json`, reload it, and write `debug/c2-storage-proof/reload_report.json`.
- [x] The report must include `ok`, `source: "PatchDocument"`, `usesInteractionState: false`, edge counts, public-port edge booleans, and expanded `mono_mix` position.

### Task 4: C2 Closure Docs And Gates

**Files:**
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`
- Create: `docs/superpowers/specs/2026-05-24-c2-compound-work-closure.md`

- [x] Mark C2.2-C2.4 evidence as proven after the tests and app proof pass.
- [x] Park local git commit, AI worker save command, raw callback-buffer runtime, module publishing, and multi-patch work libraries outside C2.
- [x] Run `cmake --build build`, `ctest --test-dir build --output-on-failure`, both existing proof dumps, the new C2 proof dump, and `git diff --check`.
