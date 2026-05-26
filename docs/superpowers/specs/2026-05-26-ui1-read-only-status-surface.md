# UI1 Read-Only Status Surface

## Status

Closed locally on 2026-05-26 11:48 Asia/Taipei.

Branch:

```text
codex/ui1-read-only-status-surface
```

## Contract

UI1 adds a read-only workbench status surface to the app. The UI reads from `WorkbenchAppStatusSnapshot`; it does not mutate graph, work, save state, mappings, preferences, or runtime.

## UI Skin Pressure Gate

```text
1. This UI reads from: WorkbenchAppController::appStatusSnapshot().
2. This UI mutates through: no mutation.
3. This is proven by: JUCE-free WorkbenchStatusSurface tests, app build, focused controller/status proof tests.
```

## Acceptance

- A JUCE-free `WorkbenchStatusSurface` view model turns `WorkbenchAppStatusSnapshot` into stable visible rows.
- The surface includes work/document, source, save, mapping, proof, and preview status.
- Missing values render as readable fallbacks instead of blank fake state.
- `MainComponent` renders the surface as read-only labels and refreshes it after open/save.
- No buttons, pickers, canvas node UI, mapping editor, save mutation, runtime cook, or Metal work enters this lane.

## Result

`WorkbenchStatusSurface` now converts app status into six stable rows. `MainComponent` renders those rows as read-only labels under the app header and refreshes them after workbench open and current-session save.

Closed line:

```text
WorkbenchAppStatusSnapshot
-> WorkbenchStatusSurface rows
-> MainComponent read-only status labels
```

## Verification

```text
cmake --build build --target my_world_workbench_status_surface_tests my-world
./build/my_world_workbench_status_surface_tests
ctest --test-dir build --output-on-failure -R "workbench_status_surface|workbench_app_controller|app_status_proof_runner"
git diff --check
```

All commands passed.

## Parked

- UI2 app UI/status proof readback.
- UI3 UI segment closure.
- Project picker, save UI, canvas node surface, mapping editor, runtime cook, OpenGL expansion, Metal, analyzer DSP, browser polish, and visual polish remain parked.
