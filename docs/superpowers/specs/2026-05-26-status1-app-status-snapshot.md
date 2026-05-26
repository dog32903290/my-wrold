# STATUS1 App Status Snapshot

## Status

Closed locally on 2026-05-26 11:25 Asia/Taipei.

Branch:

```text
codex/status1-app-status-snapshot
```

## Contract

STATUS1 gives the app a single read-only status snapshot for the current workbench session. It does not add UI controls, project picking, save formats, graph editing, runtime cook, shader binding expansion, or Metal.

## Five Questions

| Question | Answer |
| --- | --- |
| Trigger | `WorkbenchAppController` is asked for current app status after construction, after `openCurrentSession()`, or after `saveCurrentSession()`. |
| Input | No new external input. The snapshot is derived from the controller-held `WorkbenchSessionSnapshot` and controller status text. |
| Success | The caller receives stable fields for app readiness, controller status text, session status, work source/status, document id/title/version, save/proof/preview status, dirty state, work manifest paths, graph IO mapping status/counts, and diagnostics. |
| Failure | With no session, the snapshot is blocked and says `workbench blocked: no session`. With a blocked open or failed save, the snapshot remains readable and carries the blocked/failed status text instead of throwing or inventing fallback state. |
| Observability | Focused controller tests prove initial, opened, and saved snapshots. The master plan records verification commands and keeps the existing dirty APP2 proof test out of this lane. |

## Acceptance

- `WorkbenchAppController` exposes a value snapshot method for app status.
- Initial controller status is readable before any open call and reports no current session.
- Opened controller status includes source-aware session fields and graph IO mapping counts.
- Saved controller status includes `save-ok commit-pending` and `dirty == false`.
- Existing `statusText()`, `currentSession()`, save path, and open proof request behavior stay compatible.

## Result

`WorkbenchAppStatusSnapshot` is now a read-only value returned by `WorkbenchAppController::appStatusSnapshot()`. It copies the held session fields plus the controller status text, and it provides a stable blocked snapshot before the app has opened a session.

Closed line:

```text
controller-held WorkbenchSessionSnapshot
-> WorkbenchAppStatusSnapshot
-> readable no-session/open/save status
```

## Verification

```text
cmake --build build --target my_world_workbench_app_controller_tests
./build/my_world_workbench_app_controller_tests
cmake --build build --target my-world
ctest --test-dir build --output-on-failure -R "workbench_app_controller|workbench_session_open_status|app_workbench_session_proof_runner|app_save_proof_runner"
git diff --check
```

All commands passed.

## Parked

- App status dump artifact is STATUS2.
- STATUS segment closure is STATUS3.
- Save UI, project picker, active-work environment mutation, new save format, canvas UI, mapping editor, runtime cook, OpenGL backend expansion, Metal, analyzer DSP, browser polish, and visual polish remain parked.
