# Startup Proof Adapter Cleanup

Date: 2026-05-25

## Goal

Move startup proof flag/timer coordination out of the `MainComponent` constructor without changing any existing proof CLI flag, delay, output folder, artifact file, or quit-after-dump behavior.

This is step 1 of the current MainComponent adapter cleanup:

1. startup proof flags / timers
2. proof dump status facade
3. app/proof path policy
4. active work save/publish adapter

## Boundary

`MainComponent` may still trigger proof methods and translate proof results to visible status text. It should not own a constructor API with one boolean per proof family or repeat `callAfterDelay` blocks for every CLI proof.

`StartupProof` is deliberately small:

```text
CLI command line
-> StartupProofOptions
-> startupProofTasks(options)
-> MainComponent schedules task id
-> existing dump proof method
```

It does not know artifact schemas, fixture paths, proof runner internals, JUCE labels, or app shutdown.

## Closed Step

Step 1 closed as of 2026-05-25 01:51 Asia/Taipei.

Changes:

- Added `source/app/StartupProof.h` / `.cpp`.
- Replaced the 18-argument `MainComponent` constructor with `StartupProofOptions`.
- Replaced repeated startup `juce::Timer::callAfterDelay` blocks with `scheduleStartupProofs()` and `runStartupProofTask()`.
- Added `tests/StartupProofTests.cpp` to characterize task count, order, and delays.

Verification:

- `cmake -S . -B build`
- `cmake --build build --target my_world_startup_proof_tests my-world`
- `ctest --test-dir build --output-on-failure -R startup_proof`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c2-storage-proof-and-exit`
- `git diff --check`

Accepted result:

- `startup_proof` passed.
- app build passed.
- C2 storage proof CLI exited 0 and wrote `debug/c2-storage-proof/reload_report.json` plus `saved_main.patch.json`.
- whitespace check passed.

## Next Step

Step 2 should consolidate repeated proof-result status text and quit scheduling in `MainComponent` without changing each proof runner request/result shape.
