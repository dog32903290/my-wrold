# Tooll3 Interaction T0-T7 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the Tooll3-style interaction spine from T0 through T7 as native command-driven C++ proof code.

**Architecture:** Keep drawing separate from truth. `GraphContract` gains editor positions, edge ids, and port metadata; `InteractionContract` owns canvas transforms, hit-tests, commands, undo/redo, storage roundtrip, and behavior trace replay. Tests prove behavior before UI polish.

**Tech Stack:** C++20, CMake, existing lightweight test executables, no JSON dependency.

---

## File Structure

- Modify `source/core/GraphContract.h/.cpp`: editor node position, editor edges, edge ids, data type, stream kind.
- Modify `source/core/GraphLanguage.cpp`: add `move_node`, `disconnect`, `reconnect`.
- Create `source/core/InteractionContract.h/.cpp`: T0-T7 interaction state, hit-test, commands, invariants, trace runner, serialization.
- Create `fixtures/interaction/tooll3-t0-t7.behavior.json`: v1 behavior traces.
- Create tests:
  - `tests/CanvasViewStateTests.cpp`
  - `tests/NodeHitTestTests.cpp`
  - `tests/GraphCommandTests.cpp`
  - `tests/GraphInvariantTests.cpp`
  - `tests/T3T5CommandTests.cpp`
  - `tests/InteractionTraceTests.cpp`
  - `tests/InteractionStorageRoundtripTests.cpp`
- Modify `CMakeLists.txt`: build `my_world_interaction` and new tests.
- Modify `source/ui/ImGuiSmokeOverlay.h/.cpp`: visible T0-T7 canvas smoke wired to `InteractionContract`.
- Update design/spec docs after proof.

## Tasks

### Task 1: T0 Canvas Transform

- [x] Write failing `CanvasViewStateTests.cpp` for screen/canvas roundtrip, pan, and zoom-around-focus.
- [x] Add `CanvasViewState`, `CanvasPoint`, `ScreenPoint`, `canvasToScreen`, `screenToCanvas`, `panView`, and `zoomViewAround`.
- [x] Run `cmake --build build` and `ctest --test-dir build --output-on-failure`.

### Task 2: T1/T2 Graph Shape

- [x] Write failing graph command tests for node positions, edge ids, `move_node`, `connect`, `disconnect`, undo/redo.
- [x] Extend `GraphContract` with editor positions and editor edges while preserving existing tests.
- [x] Add command names to `GraphLanguage`.
- [x] Implement `GraphSession`, command stack, move/connect/disconnect.
- [x] Verify graph command tests and existing tests.

### Task 3: L2 Hit-Test

- [x] Write failing hit-test tests for node body, ports, edge, pan/zoom stability, and moved node endpoints.
- [x] Implement node/port bounds and hit-test helpers derived from graph data and `NodeSpec`.
- [x] Verify hit-test tests.

### Task 4: T3-T5 Operations

- [x] Write failing command tests for create-node insert, enter/exit patch, collapse/expand, set param, and set port binding.
- [x] Implement native command functions for T3-T5 in `InteractionContract`.
- [x] Verify command tests and graph invariants.

### Task 5: T6 Dirty State And Storage Roundtrip

- [x] Write failing storage roundtrip tests for positions, edges, collapsed state, current patch path, dirty status, and save status.
- [x] Implement interaction-state serialization/deserialization and save result handling.
- [x] Verify storage roundtrip tests.

### Task 6: T7 Behavior Trace Suite

- [x] Create v1 Tooll3-style trace fixture.
- [x] Write failing trace runner tests for move/connect/delete/create/compound/inspector/undo/save traces.
- [x] Implement simple v1 trace runner that replays known native gestures and asserts command log/evidence.
- [x] Verify trace tests.

### Task 7: Docs And Final Verification

- [x] Update `docs/superpowers/specs/2026-05-22-tooll3-interaction-borrowing-design.md` with proven status.
- [x] Run full `cmake --build build`.
- [x] Run full `ctest --test-dir build --output-on-failure`.
- [x] Review git diff for accidental UI-only mutation paths or scope creep.

## Self-Review

- Covers T0-T7 roadmap from the design spec.
- Starts with tests before implementation.
- Keeps C++ proof under core contracts; visible ImGui smoke is diagnostic, not production node editor styling.
- Explicitly parks visual polish while proving command, graph, storage, trace behavior, and app-visible command evidence.
- Final proof: `cmake --build build`, `--dump-proof-and-exit`, and `ctest --test-dir build --output-on-failure` pass; tests pass 15/15.
- Follow-up proof: visible canvas now has empty-canvas pan, wheel zoom, node drag, output-to-input connection drag, edge selection, and selected-edge disconnect.
- T3 follow-up: output-to-empty drag now opens a compatible-node popup filtered by `NodeSpec`; selecting a candidate runs `create_node+connect`.
- T4-T7 follow-up: compound navigation, inspector param/binding commands, interaction state save/reload, and in-app behavior trace replay are visible in the ImGui smoke canvas.
