# COOK3 Cook Segment Closure

## Status

Closed locally on 2026-05-26 13:37 Asia/Taipei.

Branch:

```text
codex/cook3-cook-segment-closure
```

## Contract

COOK3 closes the COOK segment from read-only current-session cook plan to app proof readback. It does not add behavior, source changes, UI controls, runtime cook, scheduler/cook order execution, node functionality, graph commands, canvas interactions, mapping editor, save mutation, or visual polish.

## Closed Line

```text
COOK1 WorkbenchSessionSnapshot runtimeGraph summaries
-> WorkbenchCookPlanSurface rows
-> MainComponent read-only cook labels
-> COOK2 app cook plan proof readback
-> cook_plan_report.json
```

## Evidence

- COOK1: `docs/superpowers/specs/2026-05-26-cook1-read-only-cook-plan.md`
- COOK2: `docs/superpowers/specs/2026-05-26-cook2-cook-plan-proof-readback.md`
- COOK2 app artifact: `debug/workbench-cook-plan-proof/cook_plan_report.json`

## Acceptance

- COOK1 and COOK2 are both closed locally.
- The visible app cook labels read real session runtime graph summaries through `WorkbenchCookPlanSurface`.
- The app proof flag reads back the same cook plan surface model as JSON evidence.
- Runtime cook is explicitly parked; this segment does not simulate node execution.
- No save UI, project picker, active-work environment mutation, new save format, analyzer DSP, full mapping editor, graph mutation commands, canvas node hit-test/gestures, selection, drag/connect/delete commands, runtime cook, shader preview live binding expansion, Metal, or visual polish enters this closure.

## Verification Target

```text
ctest --test-dir build --output-on-failure
git diff --check
```

Verification:

```text
ctest --test-dir build --output-on-failure
100% tests passed, 0 tests failed out of 101
git diff --check
passed
```

## Parked

- Real runtime cook execution
- Scheduler loop / dirty propagation
- Node functionality
- Shader/audio node execution
- Canvas node interactions
- Mapping editor
- Save mutation
- Metal / production GPU backend work
- Visual polish
