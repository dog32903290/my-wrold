# C1.12 RuntimeOp Coverage Measurement

Date: 2026-05-23 23:01 Asia/Taipei

## Skill Under Test

`compile-first-semantic-porting`

## Behavior

Unsupported loaded compound child node types must fail explicitly before they can look like partial success.

```text
runtime registry
-> child nodeType with no RuntimeOp
-> dry-run coverage failure
-> execution coverage failure
-> JSON proof records missing runtimeOp evidence
```

## Source Behavior And Files

This slice is not a direct TiXL / Tooll3 C# behavior port. The source witness for this pass is the local C1.11 runtime shape:

```text
source/core/RuntimeRegistry.h
source/core/RuntimeRegistry.cpp
tests/RuntimeRegistryTests.cpp
```

TiXL source inspection is parked for this slice because the behavior is runtime coverage honesty, not canvas interaction, undo/redo, or C# UI state translation.

## Acceptance Trace

Input graph:

```text
default loaded compound.loudness runtime registry
+ appended child:
  id: unsupported_probe
  nodeType: debug.unsupported
  role: Missing RuntimeOp fixture
  cookOrder position: after existing loudness children
```

Expected dry run:

```text
ok: false
error contains: missing RuntimeOp
entry.status: missing-runtime-op
last child.status: missing-runtime-op
last child.runtimeOp: empty
last child.reason contains: debug.unsupported
makeRuntimeDryRunJson contains missing-runtime-op
```

Expected execution:

```text
ok: false
error contains: missing RuntimeOp
entry.status: missing-runtime-op
last child.status: missing-runtime-op
last child.runtimeOp: empty
last child.reason contains: debug.unsupported
makeRuntimeExecutionJson contains missing-runtime-op
```

Unchanged positive trace:

```text
default compound.loudness still dry-runs and executes with seven named synthetic RuntimeOps
publicOutputs and source maps remain unchanged
```

## Semantic Risk Categories Expected

```text
coverage honesty:
  Unsupported node types must not be reported as ok=true partial execution.

debug artifact truth:
  Failure snapshots must still be serializable so proof JSON can show why coverage failed.

collection ordering:
  The failing child must appear in deterministic cook order, after previously evaluated children.

status vocabulary drift:
  "not-executed" means not run yet; "missing-runtime-op" means no registered op exists.

public output leakage:
  Failed coverage must not publish entry publicOutputs as if the full compound succeeded.

future module safety:
  Adding new module child specs should fail fast until RuntimeOp coverage exists.
```

## C++ Ownership / Container Assumptions

```text
RuntimeRegistry and snapshots remain value structs.
std::vector order owns proof order.
RuntimeOp definitions remain static immutable data.
No raw C# / TiXL structures are introduced.
No UI-only mutation path is introduced.
```

## Compile-First Scaffold Step

```text
1. Add test-only unsupported child to a copied RuntimeRegistry value.
2. Compile target: my_world_runtime_registry_tests.
3. Expect RED before implementation.
4. Implement the smallest RuntimeOp coverage gate in dry-run and execution.
5. Rebuild the same target before broader tests.
```

## Behavior Trace Step

```text
./build/my_world_runtime_registry_tests
ctest --test-dir build --output-on-failure
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
rg missing-runtime-op tests/RuntimeRegistryTests.cpp source/core/RuntimeRegistry.cpp
```

The app proof dump uses the default supported registry, so the negative coverage proof lives in tests unless a failing fixture is added later.

## Measurement

Token estimate method: `ceil(utf8_chars / 4)`.

```text
skill_body_chars: 7118
skill_body_token_estimate: 1780
c1_12_gate_doc_chars: 5553
c1_12_gate_doc_token_estimate: 1389
total_skill_plus_gate_token_estimate: 3168
red_result: expected compile failure; RuntimeChildDryRunStatus did not yet expose runtimeOp
compile_repairs_after_implementation: 0
test_repairs_after_implementation: 0
semantic_risks_found_before_code: 6
bugs_prevented: 1
manual_semantic_repair_buckets: 0
verdict: useful for C1.12, but only as a lightweight gate
```

Bug prevented:

```text
Before C1.12, an unsupported child node type could remain not-executed under an ok=true execution result.
The skill gate named this as coverage honesty before production code changed.
The test now catches it, and runtime returns ok=false with a serializable missing-runtime-op snapshot.
```

Error-rate note:

```text
Known coverage-honesty failure paths in this slice:
  before: 1 unhandled path
  after: 0 in tested dry-run/execution paths

Compile repairs after implementation:
  0

Test repairs after implementation:
  0
```

## Argument

Claim:

```text
For C1.12, compile-first-semantic-porting can help only if it turns "unsupported child silently remains not-executed under ok=true" into an explicit pre-code risk and test.
```

Evidence:

```text
The risk report names coverage honesty, debug artifact truth, status vocabulary drift, and public output leakage before production code changes.
```

Counter-evidence / limit:

```text
No TiXL source was inspected because this is not an interaction or C# semantic port. The skill may be too heavy for runtime-only C++ slices.
```

Decision:

```text
Use the skill as a measured gate for C1.12, not as a default full TiXL audit.
```

Next falsification test:

```text
If C1.12 still needs compile/test repairs unrelated to the risk report, or if no bug is prevented, classify the skill as not helpful for runtime-only coverage work.
```
