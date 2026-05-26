# CANVAS3 Canvas Segment Closure

## Status

Closed locally on 2026-05-26 12:41 Asia/Taipei.

Branch:

```text
codex/canvas3-canvas-segment-closure
```

## Contract

CANVAS3 closes the CANVAS segment from read-only workbench canvas surface to app proof readback. It does not add behavior, source changes, UI controls, canvas drawing widgets, graph commands, canvas interactions, mapping editor, runtime cook, save mutation, or visual polish.

## Closed Line

```text
CANVAS1 WorkbenchSessionSnapshot graph positions
-> WorkbenchCanvasSurface node bounds and edge routes
-> MainComponent read-only canvas labels
-> CANVAS2 app canvas surface proof readback
-> canvas_surface_report.json
```

## Evidence

- CANVAS1: `docs/superpowers/specs/2026-05-26-canvas1-read-only-canvas-surface.md`
- CANVAS2: `docs/superpowers/specs/2026-05-26-canvas2-canvas-surface-proof-readback.md`
- CANVAS2 app artifact: `debug/workbench-canvas-surface-proof/canvas_surface_report.json`

## Acceptance

- CANVAS1 and CANVAS2 are both closed locally.
- The visible app canvas labels read real session graph positions through `WorkbenchCanvasSurface`.
- The app proof flag reads back the same canvas surface model as JSON evidence.
- No save UI, project picker, active-work environment mutation, new save format, analyzer DSP, full mapping editor, graph mutation commands, canvas node hit-test/gestures, selection, drag/connect/delete commands, runtime cook, shader preview live binding expansion, Metal, or visual polish enters this closure.

## Verification Target

```text
ctest --test-dir build --output-on-failure
git diff --check
```

Verification:

```text
ctest --test-dir build --output-on-failure
100% tests passed, 0 tests failed out of 97
git diff --check
passed
```

## Parked

- Next feature surface should be selected from the master plan with a fresh lane name.
- Canvas painting, node hit-test/gesture, selection, drag/connect/delete commands, graph mutation commands, mapping editor, runtime cook, shader preview binding expansion, save UI, project picker, Metal, analyzer DSP, and visual polish remain parked.
