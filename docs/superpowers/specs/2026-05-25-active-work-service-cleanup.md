# Active Work Service Cleanup

Date: 2026-05-25

## Goal

Move visible active-work save and publish preparation out of `MainComponent` while preserving the existing `saveWork()` / `publishModule()` command paths, default active work fixture preparation, module publish package naming, and visible UI status behavior.

This is step 4 of the current MainComponent adapter cleanup:

1. startup proof flags / timers: closed
2. proof dump status facade: closed
3. app/proof path policy: closed
4. active work save/publish adapter: closed

## Boundary

`ActiveWorkService` owns active work orchestration:

```text
active work manifest path
-> optional default fixture preparation
-> saveWork()

active work manifest path + source node id
-> visible module publish request
-> publishModule()
```

`MainComponent` keeps only the live UI adapter:

```text
preview callback
-> ActiveWorkService
-> statusLabel text
-> CommandResult returned to preview
```

The service does not know UI labels, buttons, timers, proof status text, or live rendering state.

## Closed Step

Step 4 closed as of 2026-05-25 02:05 Asia/Taipei.

Changes:

- Added `source/app/ActiveWorkService.h` / `.cpp`.
- Moved active work fixture copy/prepare logic, `saveWork()` call, visible publish source manifest resolution, publish request construction, and safe module id generation out of `MainComponent`.
- Removed `PublishModuleResult` from the `MainComponent` private API.
- Added `tests/ActiveWorkServiceTests.cpp` to exercise save and visible publish without UI.

Verification:

- `cmake -S . -B build`
- `cmake --build build --target my_world_active_work_service_tests my-world`
- `ctest --test-dir build --output-on-failure -R active_work_service`
- `ctest --test-dir build --output-on-failure -R "active_work_service|save_work_command|module_publish"`
- `git diff --check`

Accepted result:

- app build passed.
- `active_work_service` passed.
- `save_work_command`, `module_publish`, `c5_module_publish_proof_runner`, and `active_work_service` passed together.
- whitespace check passed.

## Closure

The 1-4 MainComponent adapter cleanup is closed. Remaining cleanup should be selected as a new lane, not appended to this one.
