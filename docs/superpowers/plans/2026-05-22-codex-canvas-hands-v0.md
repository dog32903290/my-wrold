# Codex Canvas Hands V0 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give Codex a low-token, precise hand for internal canvas UI testing.

**Architecture:** Add a small `CanvasHands` core module above `InteractionContract`. Codex emits semantic targets such as `node:shader1`, `port:shader1.output`, or `point:320,240`; the module resolves them through graph data, transforms them with `CanvasViewState`, checks `hitTestGraph`, applies pointer-state gestures, and records compact command/state evidence.

**Tech Stack:** C++20, existing graph/interaction contracts, CMake/CTest, local Codex AgentSkill.

---

### Task 1: Core Canvas Hands Contract

**Files:**
- Create: `source/core/CanvasHands.h`
- Create: `source/core/CanvasHands.cpp`
- Test: `tests/CanvasHandsTests.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Write failing tests**

Create `tests/CanvasHandsTests.cpp` with tests for:
- resolving `port:shader1.output` to a viewport-local screen point and `outputPort` hit.
- dragging `node:shader1` to a canvas point and producing `move_node`.
- dragging `port:shader1.output` to `port:out1.input` on a disconnected graph and producing `connect`.
- right click, middle click, and wheel producing pointer-state evidence without graph mutation.

- [x] **Step 2: Run RED**

Run:

```bash
cmake --build build --target my_world_canvas_hands_tests
```

Expected: FAIL because `CanvasHands.h` and the target do not exist.

- [x] **Step 3: Implement minimal contract**

Add `CanvasHandTarget`, `CanvasHandPointerState`, `CanvasHandReport`, `resolveCanvasHandTarget`, and helper actions `canvasHandMove`, `canvasHandClick`, `canvasHandDrag`, `canvasHandWheel`.

- [x] **Step 4: Run GREEN**

Run:

```bash
cmake --build build --target my_world_canvas_hands_tests
./build/my_world_canvas_hands_tests
```

Expected: PASS.

### Task 2: Documentation And Skill

**Files:**
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`
- Modify: `docs/superpowers/plans/2026-05-22-codex-canvas-hands-v0.md`
- Create: `/Users/chenbaiwei/.agents/skills/codex-canvas-hands/SKILL.md`
- Modify: `/Users/chenbaiwei/.codex/CORE_SKILLS.md`

- [x] **Step 1: Update repo spec**

Mark Codex Hands V0 as a planned/proven internal canvas operation layer. State that it does not use screenshots or global macOS mouse control.

- [x] **Step 2: Create local skill**

Use `skill-creator/scripts/init_skill.py` to initialize `codex-canvas-hands`, then replace the template with a concise SKILL.md that tells future Codex instances to use semantic canvas targets, resolve before acting, and verify with command logs/invariants.

- [x] **Step 3: Validate skill**

Run:

```bash
python /Users/chenbaiwei/.agents/skills/skill-creator/scripts/quick_validate.py /Users/chenbaiwei/.agents/skills/codex-canvas-hands
```

Expected: PASS.

### Task 3: Full Pressure Test

**Files:**
- Existing tests and docs only.

- [x] **Step 1: Run focused tests**

Run:

```bash
cmake --build build --target my_world_canvas_hands_tests
./build/my_world_canvas_hands_tests
```

- [x] **Step 2: Run interaction regression tests**

Run:

```bash
ctest --test-dir build --output-on-failure -R "canvas_hands|canvas_view_state|node_hit_tests|graph_commands|graph_invariants|interaction_traces"
```

- [x] **Step 3: Update plan checkboxes**

Mark completed tasks after evidence is produced.
