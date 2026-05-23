# C1.13 Saved Negative RuntimeOp Fixture Measurement

Date: 2026-05-23 23:41 Asia/Taipei

## Skill Under Test

`compile-first-semantic-porting`

## Chosen Mode

Runtime lightweight gate.

This slice is not a direct TiXL / Tooll3 C# port. The skill was used because the user explicitly asked to keep testing whether the compile-first semantic workflow helps adjacent native runtime work.

## Behavior

The missing RuntimeOp failure must be represented by saved module files, not only by a test-built registry mutation.

```text
saved module-library fixture
-> saved compound contains debug.unsupported child
-> runtime registry load succeeds
-> dry-run/execution fail as missing-runtime-op
-> app proof dump persists negative JSON artifacts
```

## Local Source Witness

```text
source/core/RuntimeRegistry.h
source/core/RuntimeRegistry.cpp
source/render/OpenGLShaderPreview.cpp
tests/RuntimeRegistryTests.cpp
fixtures/module-libraries/default.module-library.json
fixtures/modules/loudness/module.json
fixtures/compounds/loudness.compound.json
```

Source audit parked:

```text
reason: TiXL source cannot falsify this runtime-only saved fixture path.
local witness: RuntimeRegistry loader, RuntimeOp coverage tests, and app proof dump define the behavior.
```

## Acceptance Trace

Input files:

```text
fixtures/module-libraries/missing-runtimeop.module-library.json
fixtures/modules/loudness-missing-runtimeop/module.json
fixtures/compounds/loudness-missing-runtimeop.compound.json
```

Expected registry load:

```text
ok: true
entry.nodeType: compound.loudness.missing-runtimeop
children.back.id: unsupported_probe
children.back.nodeType: debug.unsupported
cookOrder.back: unsupported_probe
```

Expected dry run:

```text
ok: false
error contains: missing RuntimeOp
entry.status: missing-runtime-op
last child.status: missing-runtime-op
last child.nodeType: debug.unsupported
```

Expected execution:

```text
ok: false
entry.status: missing-runtime-op
last child.status: missing-runtime-op
publicOutputs: empty
```

Expected app proof:

```text
debug/v1-shader-proof/runtime_missing_runtimeop_registry.json
debug/v1-shader-proof/runtime_missing_runtimeop_dry_run.json
debug/v1-shader-proof/runtime_missing_runtimeop_execution.json
```

## Semantic Risks Expected

```text
fixture honesty:
  A negative test-built mutation can pass while no saved module evidence exists.

proof artifact truth:
  Dump Proof must persist the negative failure, not only the positive default registry.

public output leakage:
  Saved negative execution must not publish entry publicOutputs.

path resolution drift:
  App proof dump must find the negative library from both repo cwd and bundled app paths.

status vocabulary:
  Saved negative proof must use the same missing-runtime-op vocabulary as C1.12.
```

## C++ Ownership / Container Assumptions

```text
RuntimeRegistry remains a value snapshot loaded from storage.
Fixture order determines cook order.
OpenGLShaderPreview only loads proof fixtures; it does not mutate runtime state.
No UI-only graph path is introduced.
```

## Compile-First Scaffold

```text
1. Add a failing runtime registry test that loads fixtures/module-libraries/missing-runtimeop.module-library.json.
2. Verify RED: the test fails because the saved module library file does not exist.
3. Add the smallest saved negative fixture files.
4. Re-run my_world_runtime_registry_tests.
5. Wire app proof dump to write negative registry/dry-run/execution JSON.
6. Build app and run --dump-proof-and-exit.
```

## Behavior / Proof Trace

```text
cmake --build build --target my_world_runtime_registry_tests
./build/my_world_runtime_registry_tests
cmake --build build
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
rg -n "missing-runtime-op|debug\\.unsupported|compound\\.loudness\\.missing-runtimeop" debug/v1-shader-proof/runtime_missing_runtimeop_*.json
```

Observed RED:

```text
FAIL: could not open module library: fixtures/module-libraries/missing-runtimeop.module-library.json
```

## Measurement

Token estimate method: `ceil(utf8_chars / 4)`.

```text
skill_body_chars: 10161
skill_body_token_estimate: 2541
c1_13_gate_doc_chars: 5752
c1_13_gate_doc_token_estimate: 1438
total_skill_plus_gate_token_estimate: 3979
red_result: expected fixture-load failure; saved negative module library did not exist
compile_repairs_after_implementation: 0
test_repairs_after_implementation: 0
semantic_risks_found_before_code: 5
bugs_prevented: 1
manual_semantic_repair_buckets: 0
verdict: useful for C1.13 as a lightweight gate
```

Bug prevented:

```text
Without C1.13, the runtime had an honest missing RuntimeOp failure path but no saved fixture or app proof artifact proving it.
The gate turned that storage-backed evidence gap into a failing test and proof dump requirement before more module/UI surface was added.
```

## Argument

Claim:

```text
For C1.13, compile-first-semantic-porting helped by forcing the negative RuntimeOp case to become a saved fixture and persisted proof artifact.
```

Evidence:

```text
The RED failed on the missing saved module library before fixture code was added.
The app proof now writes negative registry, dry-run, and execution JSON files containing debug.unsupported and missing-runtime-op.
```

Limit:

```text
No TiXL source was inspected because this slice is runtime/storage proof, not Tooll3 canvas behavior. The useful part was the lightweight gate, not full porting mode.
```

Decision:

```text
Keep using this skill only as a lightweight runtime gate when it can create a concrete test/proof before code.
```

Next falsification test:

```text
C1.14 should expose RuntimeOp support as a catalog/coverage report. If the skill does not identify a new pre-code semantic risk there, retire it for runtime-only C1 slices.
```
