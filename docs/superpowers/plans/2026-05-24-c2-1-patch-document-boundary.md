# C2.1 Patch Document Boundary Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first C2 saved patch-document boundary so compound instances reload root edges, public ports, and expanded child layout without using the temporary interaction serializer.

**Architecture:** Keep `GraphContract` as the in-memory graph truth, but add a storage-level `PatchDocument` JSON shape that can serialize and parse the graph as a saved work patch. Interaction state remains a UI/session serializer; `PatchDocument` becomes the durable file boundary.

**Tech Stack:** C++20, CMake, existing `GraphContract`, `InteractionContract`, `CompoundPatch`, `StorageContract`, focused executable tests.

---

### Task 1: Storage Patch Document Roundtrip

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `source/storage/StorageContract.h`
- Modify: `source/storage/StorageContract.cpp`
- Create: `tests/PatchDocumentTests.cpp`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [x] **Step 1: Write the failing test**

Create `tests/PatchDocumentTests.cpp` with a root graph containing `live_audio -> compound.loudness -> midi`, store an expanded `mono_mix` child layout on the compound instance, serialize through `makePatchDocument(...)` / `toJson(...)`, parse through `parsePatchDocument(...)`, and assert the reloaded `GraphContract` preserves root public-port edges and relayout positions.

- [x] **Step 2: Wire the test target**

Add `my_world_patch_document_tests` to `CMakeLists.txt`, linked against `my_world_storage`, `my_world_interaction`, `my_world_compound_module`, and `my_world_canvas_hands` only as needed by the test setup. Run `cmake --build build --target my_world_patch_document_tests`; expected result: fail because the new `PatchDocument` API does not exist yet.

- [x] **Step 3: Add the storage contract**

Add `PatchDocument`, `PatchDocumentLoadResult`, `makePatchDocument`, `parsePatchDocument`, and `loadPatchDocument` in `StorageContract`. The JSON must contain:

```json
{
  "kind": "patchDocument",
  "id": "patch.c2-main",
  "title": "C2 Main",
  "version": 1,
  "editorGraph": {
    "nodes": [
      {
        "id": "library_loud1",
        "type": "compound.loudness",
        "position": { "x": 220.0, "y": 260.0 },
        "collapsed": true,
        "params": [
          { "id": "compound.childLayout.mono_mix.x", "value": "420" }
        ],
        "portBindings": []
      }
    ],
    "edges": [
      {
        "id": "edge.live_audio.channels.library_loud1.audio.in",
        "from": "live_audio.channels",
        "to": "library_loud1.audio.in",
        "dataType": "audio.channels",
        "streamKind": "continuous"
      }
    ]
  },
  "runtimeGraph": {
    "nodes": [],
    "edges": []
  }
}
```

- [x] **Step 4: Prove roundtrip**

Run `cmake --build build --target my_world_patch_document_tests` and `./build/my_world_patch_document_tests`. Expected result: `patch document boundary ok`.

- [x] **Step 5: Run nearby gates**

Run `./build/my_world_storage_tests`, `./build/my_world_interaction_storage_roundtrip_tests`, and `./build/my_world_compound_interaction_tests`. Expected result: all pass; the old interaction serializer remains intact but no longer carries the C2 durable-patch proof.
