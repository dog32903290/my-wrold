# GRAPH3 Graph Segment Closure

## Status

Closed locally on 2026-05-26 12:17 Asia/Taipei.

Branch:

```text
codex/graph3-graph-segment-closure
```

## Contract

GRAPH3 closes the GRAPH segment from read-only workbench graph surface to app proof readback. It does not add behavior, source changes, UI controls, graph commands, canvas interactions, mapping editor, runtime cook, or visual polish.

## Closed Line

```text
GRAPH1 PatchDocument graph
-> WorkbenchSessionSnapshot editor node/edge summaries
-> WorkbenchGraphSurface rows
-> MainComponent read-only graph labels
-> GRAPH2 app graph surface proof readback
-> graph_surface_report.json
```

## Evidence

- GRAPH1: `docs/superpowers/specs/2026-05-26-graph1-read-only-graph-surface.md`
- GRAPH2: `docs/superpowers/specs/2026-05-26-graph2-graph-surface-proof-readback.md`
- GRAPH2 app artifact: `debug/workbench-graph-surface-proof/graph_surface_report.json`

## Acceptance

- GRAPH1 and GRAPH2 are both closed locally.
- The app graph surface reads real session graph summaries.
- The app proof flag reads back the same graph surface model as JSON evidence.
- No save UI, project picker, active-work environment mutation, new save format, canvas node surface, graph mutation commands, mapping editor, runtime cook, OpenGL backend expansion, Metal, analyzer DSP, browser polish, or visual polish enters this closure.

## Verification Target

```text
ctest --test-dir build --output-on-failure
git diff --check
```

Verification:

```text
ctest --test-dir build --output-on-failure
100% tests passed, 0 tests failed out of 95
git diff --check
passed
```

## Parked

- Next feature surface should be selected from the master plan with a fresh lane name.
- Canvas node rendering, node hit-test/gesture, graph mutation commands, mapping editor, runtime cook, shader preview binding expansion, save UI, project picker, Metal, analyzer DSP, and visual polish remain parked.
