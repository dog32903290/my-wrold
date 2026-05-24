# Proof Status Facade Cleanup

Date: 2026-05-25

## Goal

Remove repeated proof-result status text and quit-after-dump handling from individual `MainComponent::dump...Proof()` methods while keeping each proof runner request shape unchanged.

This is step 2 of the current MainComponent adapter cleanup:

1. startup proof flags / timers: closed
2. proof dump status facade: closed
3. app/proof path policy
4. active work save/publish adapter

## Boundary

`finishProofDump()` is only a UI/status adapter:

```text
proof display name + status + error + output directory
-> statusLabel text
-> optional quitAfterDelay()
```

It does not choose proof kind, build request fields, resolve fixtures, write reports, or inspect artifact contents.

## Closed Step

Step 2 closed as of 2026-05-25 01:54 Asia/Taipei.

Changes:

- Added `MainComponent::finishProofDump()`.
- Replaced repeated `result.status == "failed"` / success status label blocks across A1, C2, C3, C4, C5, C6, PV, and PV-B1 proof facade methods.
- Centralized the repeated `shouldQuitAfterStartupDump -> quitAfterDelay()` block for file-based proof runners.
- Left V1 shader proof status on `setShaderStatus()` because it still reports through the live preview callback.

Verification:

- `cmake --build build --target my-world`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-attack-detector-proof-and-exit`
- `git diff --check`

Accepted result:

- app build passed.
- PV attack detector proof CLI exited 0 and wrote `debug/pv-attack-detector-proof/attack_detector_report.json`, `cook_order.json`, `errors.json`, and `node_stats.json`.
- whitespace check passed.

## Next Step

Step 3 should extract project/debug path policy from `MainComponent`, keeping proof output folder names and environment variable behavior unchanged.
