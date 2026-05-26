# RUN3 Run Segment Closure

## Status

Closed locally on 2026-05-26 14:25 Asia/Taipei.

Branch:

```text
codex/run3-run-segment-closure
```

## Contract

RUN3 closes the RUN segment from current-session runtime graph summaries to app
proof readback and headless artifacts. It adds no runtime behavior.

This closure does not add a full scheduler, dirty propagation, shader.fragment
execution, audio node execution, graph mutation, save mutation, canvas
interactions, mapping editor, Metal, or visual polish.

## One-Line Proof

```text
RUN1 workbench headless run
-> RUN2 workbench run proof readback
-> RUN3 run segment closure
```

## Evidence

- RUN1: `docs/superpowers/specs/2026-05-26-run1-workbench-headless-run.md`
- RUN2: `docs/superpowers/specs/2026-05-26-run2-workbench-run-proof-readback.md`
- RUN2 app artifact: `debug/workbench-run-proof/run_report.json`

## Acceptance

- RUN1 and RUN2 are both closed locally.
- Visible app run labels read the current session through `WorkbenchRunSurface`.
- App proof flag `--dump-workbench-run-proof-and-exit` writes `run_report.json`
  and headless artifacts from `runWorkbenchHeadlessRender()`.
- Unsupported shader.fragment execution remains explicit and parked.
- Full `ctest` passes after closure docs.
- `git diff --check` passes.

## Verification Target

```text
ctest --test-dir build --output-on-failure
git diff --check
```

## Accepted Result

```text
RUN1 workbench headless run
-> RUN2 workbench run proof readback
-> RUN3 run segment closure
```

RUN1 and RUN2 close the RUN segment from current-session runtime graph summaries
to app proof readback and headless artifacts. The app can show read-only run
status, generate the headless fixture for the supported
`image.constant -> output.texture_summary` shape, run it through the existing
`HeadlessRenderRuntime`, and dump `debug/workbench-run-proof/run_report.json`
with artifact paths and `executionStatus: ran`.

## Verification

```text
ctest --test-dir build --output-on-failure
100% tests passed, 0 tests failed out of 103
git diff --check
passed
```

## Parked

- Full runtime scheduler
- Dirty propagation
- Real shader.fragment execution
- Audio runtime node execution
- Node editor functionality
- Canvas interactions
- Mapping editor
- Save mutation
- Metal / production GPU backend work
- Visual polish
