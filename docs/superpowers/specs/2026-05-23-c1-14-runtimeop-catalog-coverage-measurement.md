# C1.14 RuntimeOp Catalog Coverage Measurement

Date: 2026-05-23 23:54 Asia/Taipei

## Skill Under Test

`compile-first-semantic-porting`

## Chosen Mode

Runtime lightweight gate.

This slice is not a TiXL / Tooll3 C# behavior port. The skill was used because the current C1 runtime work is explicitly measuring whether the compile-first semantic flow still reduces errors for adjacent native graph/runtime proofs.

## Behavior

RuntimeOp support must be inspectable before execution.

```text
synthetic RuntimeOp table
-> serializable catalog
-> registry coverage snapshot
-> supported/missing child nodeTypes named before dry-run/execution
-> app proof dump persists positive and saved-negative coverage JSON
```

## Local Source Witness

```text
source/core/RuntimeRegistry.h
source/core/RuntimeRegistry.cpp
source/render/OpenGLShaderPreview.cpp
tests/RuntimeRegistryTests.cpp
fixtures/module-libraries/default.module-library.json
fixtures/module-libraries/missing-runtimeop.module-library.json
```

Source audit parked:

```text
reason: TiXL source cannot falsify this runtime-only coverage report path.
local witness: RuntimeRegistry's synthetic RuntimeOp table and saved module fixtures define the behavior.
```

## Acceptance Trace

Catalog:

```text
makeRuntimeOpCatalog()
-> 7 entries
-> first: audio.input / synthetic.audio.input
-> last: analyzer.loudness_out / synthetic.analyzer.loudness_out
-> makeRuntimeOpCatalogJson contains runtimeOpCatalog
```

Positive coverage:

```text
default module library
-> inspectRuntimeOpCoverage()
-> ok: true
-> supportedChildCount: 7
-> missingChildCount: 0
-> all children status: supported-runtime-op
-> app proof writes runtime_op_coverage.json
```

Saved negative coverage:

```text
missing-runtimeop module library
-> inspectRuntimeOpCoverage()
-> ok: false
-> supportedChildCount: 7
-> missingChildCount: 1
-> unsupported_probe status: missing-runtime-op
-> app proof writes runtime_missing_runtimeop_coverage.json
```

## Semantic Risks Expected

```text
author-feedback timing:
  Coverage must be inspectable before execution, not only after dry-run/execution fails.

catalog drift:
  RuntimeOp ids in coverage must come from the same table used for execution dispatch.

false confidence:
  Positive coverage can say "registered", not "computed"; execution is still a separate proof.

negative completeness:
  A saved negative registry should report supported siblings and missing children, not stop at the first failure.

UI readiness:
  The report must be serializable so browser/inspector diagnostics can consume it later.
```

## C++ Ownership / Container Assumptions

```text
RuntimeOp catalog entries are value structs.
The synthetic RuntimeOp function table stays private; public API exposes only nodeType/runtimeOp ids.
Coverage walks RuntimeRegistry cookOrder without mutating registry state.
Coverage does not allocate inside realtime callbacks.
```

## Compile-First Scaffold

```text
1. Add failing tests for makeRuntimeOpCatalog(), inspectRuntimeOpCoverage(), and JSON writers.
2. Verify RED: compile fails because the API does not exist.
3. Add value structs and public function declarations.
4. Implement catalog and coverage by reusing the existing synthetic RuntimeOp table.
5. Add app proof dump files after the unit path passes.
```

## Behavior / Proof Trace

```text
cmake --build build --target my_world_runtime_registry_tests
./build/my_world_runtime_registry_tests
cmake --build build
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
rg -n "runtimeOpCatalog|runtimeOpCoverage|supported-runtime-op|missing-runtime-op|debug\\.unsupported" debug/v1-shader-proof/runtime_op_catalog.json debug/v1-shader-proof/runtime_op_coverage.json debug/v1-shader-proof/runtime_missing_runtimeop_coverage.json
```

Observed RED:

```text
error: no member named 'makeRuntimeOpCatalog' in namespace 'myworld'
error: no member named 'inspectRuntimeOpCoverage' in namespace 'myworld'
error: no member named 'makeRuntimeOpCoverageJson' in namespace 'myworld'
```

## Measurement

Token estimate method: `ceil(utf8_chars / 4)`.

```text
skill_body_chars: 10161
skill_body_token_estimate: 2541
c1_14_gate_doc_chars: 5827
c1_14_gate_doc_token_estimate: 1457
total_skill_plus_gate_token_estimate: 3998
red_result: expected compile failure; RuntimeOp catalog/coverage API did not exist
compile_repairs_after_implementation: 0
test_repairs_after_implementation: 0
semantic_risks_found_before_code: 5
bugs_prevented: 1
manual_semantic_repair_buckets: 0
verdict: useful for C1.14 as a lightweight gate
```

Bug prevented:

```text
Without C1.14, future module authors would only learn RuntimeOp support by running dry-run/execution and hitting failure.
The gate moved that risk into a pre-execution coverage report with positive and negative proof artifacts.
```

## Argument

Claim:

```text
For C1.14, compile-first-semantic-porting helped because it forced "registered RuntimeOp" to remain distinct from "computed output" and made the author-feedback timing explicit before code.
```

Evidence:

```text
The RED failed on missing API declarations.
The implementation now exposes the same dispatch table as serializable catalog/coverage JSON, while execution proof remains separate.
```

Limit:

```text
No TiXL source was inspected because this is runtime coverage infrastructure, not interaction porting. The report is headless proof only; visible browser/inspector diagnostics are still C1.15.
```

Decision:

```text
Keep using this skill only while it produces a pre-code failing test or proof gate. Do not run full TiXL audit for runtime-only C1 slices.
```

Next falsification test:

```text
C1.15 should consume this coverage report in visible UI without widening runtime state. If the skill does not identify a useful UI/runtime boundary risk there, stop using it for non-porting C1 work.
```
