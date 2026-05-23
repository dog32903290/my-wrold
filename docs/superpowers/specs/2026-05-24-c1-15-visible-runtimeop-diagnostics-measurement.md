# C1.15 Visible RuntimeOp Diagnostics Measurement

Date: 2026-05-24 00:21 Asia/Taipei

## Skill Under Test

`compile-first-semantic-porting`

## Chosen Mode

Runtime lightweight gate.

This slice is not a TiXL / Tooll3 C# behavior port. The skill was used because the current C1 runtime/UI boundary is explicitly testing whether the compile-first semantic workflow helps adjacent native graph/runtime work.

## Behavior

RuntimeOp coverage must be visible as author feedback before create/execute.

```text
coverage snapshot
-> UI-facing module diagnostics
-> browser/inspector/left rail status
-> proof dump records the same visible diagnostics
```

## Local Source Witness

```text
source/core/RuntimeRegistry.h
source/core/RuntimeRegistry.cpp
source/ui/ImGuiSmokeOverlay.h
source/ui/ImGuiSmokeOverlay.cpp
source/render/OpenGLShaderPreview.h
source/render/OpenGLShaderPreview.cpp
tests/RuntimeRegistryTests.cpp
fixtures/module-libraries/default.module-library.json
fixtures/module-libraries/missing-runtimeop.module-library.json
```

Source audit parked:

```text
reason: TiXL source cannot falsify this runtime-only coverage-to-UI reporting path.
local witness: RuntimeRegistry coverage snapshots, saved module fixtures, ImGui smoke overlay, and app proof dump define the behavior.
```

## Acceptance Trace

Positive diagnostics:

```text
default module library
-> inspectRuntimeOpCoverage()
-> makeRuntimeOpModuleDiagnostics()
-> entry nodeType: compound.loudness
-> status: runtime-op-ready
-> browserLabel: runtime ready
-> inspectorDetail: 7 RuntimeOps registered; execution not run
```

Saved negative diagnostics:

```text
missing-runtimeop module library
-> inspectRuntimeOpCoverage()
-> makeRuntimeOpModuleDiagnostics()
-> entry nodeType: compound.loudness.missing-runtimeop
-> status: missing-runtime-op
-> browserLabel: missing RuntimeOp
-> inspectorDetail: missing RuntimeOp: debug.unsupported
-> missingNodeTypes: [debug.unsupported]
```

App proof:

```text
OpenGLShaderPreview loads positive and saved negative coverage
-> passes diagnostics into ImGuiSmokeOverlay
-> writes debug/v1-shader-proof/runtime_ui_diagnostics.json
-> frame.png shows Runtime Coverage in the left rail
```

## Semantic Risks Expected

```text
skin truth drift:
  UI labels must come from RuntimeRegistry coverage snapshots, not independent ImGui strings.

false computation claim:
  A supported module can say runtime ready, but not computed; execution remains separate proof.

negative legibility:
  Missing RuntimeOp details must name debug.unsupported before a user creates or executes the module.

browser/inspector parity:
  The same diagnostic contract must serve browser and inspector surfaces.

proof parity:
  Dump Proof must persist the exact diagnostics the UI can show.
```

## C++ Ownership / Container Assumptions

```text
RuntimeOpModuleDiagnostic is a value struct derived from coverage snapshots.
ImGuiSmokeOverlay stores a copied vector of diagnostics for drawing only.
OpenGLShaderPreview builds proof diagnostics from loaded registries; it does not mutate runtime state.
No realtime audio callback path reads or writes the diagnostics.
```

## Compile-First Scaffold

```text
1. Add failing tests for makeRuntimeOpModuleDiagnostics() and makeRuntimeOpModuleDiagnosticsJson().
2. Verify RED: compile fails because the API does not exist.
3. Add value struct and public declarations.
4. Implement diagnostics from RuntimeOpCoverageSnapshot.
5. Wire ImGuiSmokeOverlay to read diagnostics in browser/inspector/left rail.
6. Wire app proof dump to write runtime_ui_diagnostics.json.
```

## Behavior / Proof Trace

```text
cmake --build build --target my_world_runtime_registry_tests
./build/my_world_runtime_registry_tests
cmake --build build
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
rg -n "runtimeOpModuleDiagnostics|visibleIn|runtime ready|missing RuntimeOp|debug\\.unsupported|compound\\.loudness" debug/v1-shader-proof/runtime_ui_diagnostics.json
```

Observed RED:

```text
error: no member named 'makeRuntimeOpModuleDiagnostics' in namespace 'myworld'
error: no member named 'makeRuntimeOpModuleDiagnosticsJson' in namespace 'myworld'
```

## Measurement

Token estimate method: `ceil(utf8_chars / 4)`.

```text
skill_body_chars: 10161
skill_body_token_estimate: 2541
c1_15_gate_doc_chars: 5958
c1_15_gate_doc_token_estimate: 1490
total_skill_plus_gate_token_estimate: 4031
red_result: expected compile failure; UI-facing RuntimeOp diagnostics API did not exist
compile_repairs_after_implementation: 0
test_repairs_after_implementation: 0
semantic_risks_found_before_code: 5
bugs_prevented: 1
manual_semantic_repair_buckets: 0
verdict: useful for C1.15 as a lightweight gate
```

Bug prevented:

```text
Without C1.15, the skin could show runtime confidence that did not come from the RuntimeRegistry coverage contract.
The gate forced browser/inspector text and proof JSON to share the same diagnostics derived from coverage snapshots.
```

## Argument

Claim:

```text
For C1.15, compile-first-semantic-porting helped because it kept Tooll3-like skin feedback downstream of runtime coverage instead of letting ImGui become a second source of truth.
```

Evidence:

```text
The RED failed on missing diagnostics API declarations.
The implementation now serializes UI-facing diagnostics and draws the same status in the workspace.
```

Limit:

```text
No TiXL source was inspected because this is not interaction behavior porting. The current proof shows status, but it does not yet gate module creation.
```

Decision:

```text
Keep using this skill only when a C1 slice can produce a concrete RED API/proof gate. If the next slice is pure UI polish, use ui-skin-pressure-gate alone.
```

Next falsification test:

```text
C1.16 should make node browser creation read RuntimeOp diagnostics. If the skill does not surface a new command/runtime boundary risk, stop applying it to non-porting UI slices.
```
