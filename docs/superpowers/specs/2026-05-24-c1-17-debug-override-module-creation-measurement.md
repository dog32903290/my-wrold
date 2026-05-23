# C1.17 Debug Override Module Creation Measurement

Date: 2026-05-24 00:52 Asia/Taipei

## Skill Under Test

`compile-first-semantic-porting`

## Chosen Mode

Runtime lightweight gate.

This slice is not a TiXL / Tooll3 C# behavior port. The skill was used because the current C1 command/UI/runtime boundary is explicitly testing whether the compile-first semantic flow helps adjacent native graph/runtime work.

## Behavior

Blocked modules may be inserted only through an explicit debug override command.

```text
create-blocked diagnostic
-> normal create remains blocked
-> explicit debug override with reason
-> graph mutation carries debug evidence
-> command log and undo prove intent
```

## Local Source Witness

```text
source/core/InteractionContract.h
source/core/InteractionContract.cpp
source/ui/ImGuiSmokeOverlay.cpp
tests/T3T5CommandTests.cpp
docs/superpowers/handoffs/2026-05-23-native-canvas-progress.md
```

Source audit parked:

```text
reason: TiXL source cannot falsify this local RuntimeOp repair insertion path.
local witness: InteractionContract commands, NodeCreationGate, ImGui browser wiring, and T3-T5 command tests define the behavior.
```

## Acceptance Trace

Normal blocked create remains blocked:

```text
NodeCreationGate { nodeType: compound.loudness.missing-runtimeop, canCreate: false }
-> createNode(...)
-> ok: false
-> graph has no blocked_loud1 node
```

Empty override reason is rejected:

```text
createNodeWithDebugOverride(..., overrideReason: "")
-> ok: false
-> graph has no override_empty node
```

Explicit override create:

```text
createNodeWithDebugOverride(..., overrideReason: repair missing RuntimeOp)
-> ok: true
-> graph has override_loud1
-> node params include:
   debug.creationOverride = true
   debug.creationOverrideReason = repair missing RuntimeOp
   debug.creationBlockedReason = missing RuntimeOp: debug.unsupported
-> commandLog: create_node_debug_override
-> undo removes override_loud1
-> commandLog: undo:create_node_debug_override
```

Explicit override create-and-connect:

```text
audio1.channels
-> createNodeAndConnectWithDebugOverride(..., overrideReason: wire for RuntimeOp repair)
-> ok: true
-> graph has override_loud2
-> edge target is override_loud2.audio.in
-> commandLog: create_node+connect_debug_override
```

UI path:

```text
blocked browser row
-> normal selectable disabled
-> visible Override button
-> createNodeWithDebugOverride() or createNodeAndConnectWithDebugOverride()
-> reason derived from diagnostic creationBlockReason
```

## Semantic Risks Expected

```text
hidden bypass:
  An override must not silently call the normal create path or erase evidence of the block.

reasonless repair:
  Empty override reasons would recreate the same false confidence C1.16 prevented.

undo boundary:
  Override insertion must be a normal command transaction, not a side mutation.

type compatibility:
  create-and-connect override still has to obey port type invariants.

UI-only override:
  The visible button must lower into InteractionContract, not just flip an ImGui flag.
```

## C++ Ownership / Container Assumptions

```text
Debug override evidence is stored as GraphNode params for now.
The graph remains serializable through existing GraphNode fields.
The override command uses the same snapshot/undo stack as other InteractionContract mutations.
No runtime execution path treats debug override params as computation success.
```

## Compile-First Scaffold

```text
1. Add failing T3-T5 command tests for createNodeWithDebugOverride() and createNodeAndConnectWithDebugOverride().
2. Verify RED: compile fails because override APIs do not exist.
3. Add InteractionContract declarations and implementations.
4. Store debug override params and distinct command names.
5. Wire ImGui blocked rows to visible Override buttons.
6. Rebuild, run targeted tests, full ctest, app proof dump, and diff check.
```

## Behavior / Proof Trace

```text
cmake --build build --target my_world_t3_t5_command_tests
./build/my_world_t3_t5_command_tests
cmake --build build
./build/my_world_storage_tests
./build/my_world_compound_module_tests
./build/my_world_runtime_registry_tests
ctest --test-dir build --output-on-failure
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
git diff --check
```

Observed RED:

```text
error: no member named 'createNodeWithDebugOverride' in namespace 'myworld'
error: no member named 'createNodeAndConnectWithDebugOverride' in namespace 'myworld'
```

## Measurement

Token estimate method: `ceil(utf8_chars / 4)`.

```text
skill_body_chars: 10161
skill_body_token_estimate: 2541
c1_17_gate_doc_chars: 6605
c1_17_gate_doc_token_estimate: 1652
total_skill_plus_gate_token_estimate: 4193
red_result: expected compile failure; debug override command APIs did not exist
compile_repairs_after_implementation: 0
test_repairs_after_implementation: 1
semantic_risks_found_before_code: 5
bugs_prevented: 1
manual_semantic_repair_buckets: 1
verdict: useful for C1.17 as a lightweight gate
```

Test repair:

```text
The first create-and-connect override test used shader1.output against compound.loudness.missing-runtimeop.audio.in.
That correctly failed type validation. The test was repaired to create audio1 and connect audio1.channels into audio.in.
```

Bug prevented:

```text
Without C1.17, the next repair workflow would have had to choose between no insertion and silently bypassing C1.16.
The gate turns repair insertion into an explicit command with reason, params, command log, and undo evidence.
```

## Argument

Claim:

```text
For C1.17, compile-first-semantic-porting helped because it forced debug insertion to preserve command semantics and graph evidence instead of becoming an ImGui-only escape hatch.
```

Evidence:

```text
The RED failed on missing override APIs.
The implementation now rejects reasonless overrides, stores debug params, logs distinct commands, and preserves undo behavior.
```

Limit:

```text
No TiXL source was inspected because this is not interaction behavior porting. The current proof covers command semantics and first UI affordance, not a full repair workflow.
```

Decision:

```text
Keep this skill only for C slices where a semantic risk can become a failing compile/test gate. The next compound drag/drop proof may use Tooll3 interaction compatibility more directly.
```

Next falsification test:

```text
C1.18 should prove production expanded/collapsed compound drag/drop. If this becomes pure gesture work, use tooll3-interaction-compatibility and ui-skin-pressure-gate instead of this runtime lightweight gate.
```
