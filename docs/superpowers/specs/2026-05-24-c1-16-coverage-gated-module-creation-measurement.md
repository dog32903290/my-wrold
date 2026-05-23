# C1.16 Coverage-Gated Module Creation Measurement

Date: 2026-05-24 00:39 Asia/Taipei

## Skill Under Test

`compile-first-semantic-porting`

## Chosen Mode

Runtime lightweight gate.

This slice is not a TiXL / Tooll3 C# behavior port. The skill was used because the current C1 UI/runtime/command boundary is explicitly testing whether the compile-first semantic flow helps adjacent native graph/runtime work.

## Behavior

RuntimeOp diagnostics must gate normal module creation.

```text
RuntimeOp coverage diagnostics
-> creation affordance state
-> command-level NodeCreationGate
-> browser and compatible-node popup pass the gate
-> missing-runtime modules do not mutate the graph
```

## Local Source Witness

```text
source/core/RuntimeRegistry.h
source/core/RuntimeRegistry.cpp
source/core/InteractionContract.h
source/core/InteractionContract.cpp
source/ui/ImGuiSmokeOverlay.cpp
tests/RuntimeRegistryTests.cpp
tests/T3T5CommandTests.cpp
debug/v1-shader-proof/runtime_ui_diagnostics.json
```

Source audit parked:

```text
reason: TiXL source cannot falsify this runtime diagnostic to native command gate path.
local witness: RuntimeRegistry diagnostics, InteractionContract create commands, ImGui browser wiring, and proof dump define the behavior.
```

## Acceptance Trace

Diagnostic state:

```text
compound.loudness
-> creationStatus: create-enabled
-> creationLabel: create
-> runtimeOpDiagnosticAllowsCreation(): true

compound.loudness.missing-runtimeop
-> creationStatus: create-blocked
-> creationLabel: blocked
-> creationBlockReason: missing RuntimeOp: debug.unsupported
-> runtimeOpDiagnosticAllowsCreation(): false
```

Command gate:

```text
NodeCreationGate { nodeType: compound.loudness.missing-runtimeop, canCreate: false }
-> createNode(...)
-> ok: false
-> message names debug.unsupported
-> graph has no blocked_loud1 node

NodeCreationGate { nodeType: compound.loudness.missing-runtimeop, canCreate: false }
-> createNodeAndConnect(...)
-> ok: false
-> message names debug.unsupported
-> graph has no blocked_loud2 node

NodeCreationGate { nodeType: compound.loudness, canCreate: true }
-> createNode(...)
-> ok: true
-> graph has gated_loud1 node
```

UI path:

```text
ImGuiSmokeOverlay runtimeOpDiagnostics
-> makeNodeCreationGates()
-> drawWorkspaceNodeBrowser passes gates into createNode()
-> drawCreateNodePopup passes gates into createNodeAndConnect()
-> blocked diagnostics disable normal selectable rows
```

Proof dump:

```text
debug/v1-shader-proof/runtime_ui_diagnostics.json
-> creationStatus create-enabled for compound.loudness
-> creationStatus create-blocked for compound.loudness.missing-runtimeop
-> creationBlockReason names debug.unsupported
```

## Semantic Risks Expected

```text
UI-only gate:
  Disabling an ImGui row is not enough; the command path must be able to refuse the create.

false repair insertion:
  Missing RuntimeOp modules should not enter the graph through normal create while appearing broken only later.

diagnostic drift:
  Browser labels, JSON proof, and command gates must all derive from RuntimeOp diagnostics.

spec mismatch:
  The compatible-node popup must use the passed visible registry, not seed-only specs.

future override:
  Blocking is not the same as repair workflow; explicit debug override remains a separate command.
```

## C++ Ownership / Container Assumptions

```text
NodeCreationGate is a small value struct in InteractionContract.
RuntimeOpModuleDiagnostic remains a runtime-derived value struct.
ImGuiSmokeOverlay maps diagnostics into gates per popup draw; it does not persist graph state in widgets.
createNode/createNodeAndConnect check gates before graph mutation.
No realtime audio callback path reads or writes creation gates.
```

## Compile-First Scaffold

```text
1. Add failing RuntimeRegistry tests for creationStatus, creationLabel, creationBlockReason, and runtimeOpDiagnosticAllowsCreation().
2. Add failing T3-T5 command tests for NodeCreationGate blocking graph mutation.
3. Verify RED: compile fails because diagnostic fields, helper, and NodeCreationGate do not exist.
4. Add diagnostic fields and JSON serialization.
5. Add command-level gated create overloads.
6. Wire ImGui browser and compatible-node popup through gates.
7. Rebuild, run targeted tests, full ctest, app proof dump, and diff check.
```

## Behavior / Proof Trace

```text
cmake --build build --target my_world_runtime_registry_tests
cmake --build build --target my_world_t3_t5_command_tests
./build/my_world_runtime_registry_tests
./build/my_world_t3_t5_command_tests
cmake --build build
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
rg -n "creationStatus|create-enabled|create-blocked|creationBlockReason|debug\\.unsupported" debug/v1-shader-proof/runtime_ui_diagnostics.json
```

Observed RED:

```text
error: no member named 'creationStatus' in 'myworld::RuntimeOpModuleDiagnostic'
error: no member named 'runtimeOpDiagnosticAllowsCreation' in namespace 'myworld'
error: no member named 'NodeCreationGate' in namespace 'myworld'
```

## Measurement

Token estimate method: `ceil(utf8_chars / 4)`.

```text
skill_body_chars: 10161
skill_body_token_estimate: 2541
c1_16_gate_doc_chars: 6962
c1_16_gate_doc_token_estimate: 1741
total_skill_plus_gate_token_estimate: 4282
red_result: expected compile failure; creation affordance fields and command gate did not exist
compile_repairs_after_implementation: 0
test_repairs_after_implementation: 0
semantic_risks_found_before_code: 5
bugs_prevented: 1
manual_semantic_repair_buckets: 0
verdict: useful for C1.16 as a lightweight gate
```

Bug prevented:

```text
Without C1.16, missing-runtime modules could be visually labeled but still enter the graph through the normal browser create path.
The gate moved that risk into a command-level refusal before mutation.
```

## Argument

Claim:

```text
For C1.16, compile-first-semantic-porting helped because it forced creation affordance to become a command-level contract, not an ImGui-only disabled row.
```

Evidence:

```text
The RED failed on missing diagnostics fields and missing NodeCreationGate.
The implementation now serializes creation status, blocks missing-runtime creation before mutation, and wires both browser create surfaces through the same gate.
```

Limit:

```text
No TiXL source was inspected because this is not interaction behavior porting. The current proof blocks normal creation; it does not yet define a deliberate debug override.
```

Decision:

```text
Keep using this skill only when it creates a concrete RED test or proof gate. For pure browser layout polish, use ui-skin-pressure-gate without the porting wrapper.
```

Next falsification test:

```text
C1.17 should define an explicit override command if blocked modules need to be inserted for repair. If that can be done with existing InteractionContract gates and no new semantic risk, stop applying compile-first-semantic-porting to this local UI lane.
```
