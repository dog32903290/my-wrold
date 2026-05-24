# App Path Policy Cleanup

Date: 2026-05-25

## Goal

Move project/debug path policy out of `MainComponent` while preserving existing environment variable behavior, proof output directories, executable fallback paths, and active work manifest defaults.

This is step 3 of the current MainComponent adapter cleanup:

1. startup proof flags / timers: closed
2. proof dump status facade: closed
3. app/proof path policy: closed
4. active work save/publish adapter

## Boundary

`AppPaths` owns path policy:

```text
MY_WORLD_PROJECT_DIR / working directory / desktop fallback
-> projectDirectory()
-> debug proof directories
-> candidate roots for proof runners
-> active work manifest default / env override
```

It does not know proof runner schemas, proof result status text, storage command semantics, or UI labels.

## Closed Step

Step 3 closed as of 2026-05-25 01:58 Asia/Taipei.

Changes:

- Added `source/app/AppPaths.h` / `.cpp`.
- Moved project root detection, debug proof folder construction, active work manifest path selection, repo candidate paths, and proof candidate roots out of `MainComponent`.
- Updated proof dump methods to call `proofDumpDirectory(<runner directory name>)`.
- Kept active work file copy/save/publish behavior in `MainComponent` for step 4.

Verification:

- `cmake --build build --target my-world`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c2-storage-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-visible-module-publish-proof-and-exit`
- `git diff --check`

Accepted result:

- app build passed.
- C2 storage proof CLI exited 0 and wrote `debug/c2-storage-proof/reload_report.json` plus `saved_main.patch.json`.
- C5 visible module publish proof CLI exited 0 and wrote `debug/c5-visible-module-publish-proof/visible_module_publish_report.json`.
- whitespace check passed.

## Next Step

Step 4 should move active work save/publish preparation and request construction out of `MainComponent`, keeping the visible UI callbacks as thin status adapters.
