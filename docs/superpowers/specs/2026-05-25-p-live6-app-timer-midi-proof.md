# P-LIVE6 Controlled App Timer MIDI Proof Closure

Date: 2026-05-25 13:36 Asia/Taipei.

Status: closed.

## Question

Can the opt-in live IO proof path switch the app timer body from `dryRun` to `controlledSend` once and prove the MIDI sender boundary is reached?

## Closed Line

```text
--dump-live-io-proof-and-exit
-> LiveIOProofRunner
-> LiveIOControlTimerConfig(sendMode: controlledSend)
-> active AudioAnalyzerSnapshot
-> LiveIOControlPump
-> LiveIOControlDispatcher
-> injected MIDI sender
-> live_io_app_timer_midi_report.json
```

## Contract

Trigger:
- explicit app proof flag `--dump-live-io-proof-and-exit`
- focused `LiveIOProofRunner` test

Input:
- MIDI output inventory with selected identifier
- injected MIDI sender
- active analyzer snapshot
- live IO bindings for loudness to MIDI CC, OSC float, and shader uniform

Success:
- app timer body runs in `controlledSend`
- MIDI is enabled and OSC is disabled for this proof
- injected MIDI sender is called once
- state records `midiControlledSendCount: 1`
- state records `oscControlledSendCount: 0`
- shader uniform is counted as skipped
- report writes `live_io_app_timer_midi_report.json`

Failure:
- missing MIDI inventory or sender fails with readable errors
- proof runner returns failed status and does not hide diagnostics

Observability:
- report fields include `kind`, `ok`, `status`, `message`, `sendMode`, dry-run counts, controlled-send counts, `shaderSkippedCount`, `lastLoudness`, `lastSampleCounter`, and `errors`

## Realtime Law

This lane keeps the realtime callback clean:
- audio callback updates analyzer state only
- no MIDI send from the audio callback
- no device discovery/opening from the audio callback
- no allocation, file IO, logging, or JSON work from the audio callback

This lane does not change normal app behavior:
- `MainComponent` still defaults to `dryRun`
- controlled MIDI happens only inside the explicit proof run

Existing preference MIDI streaming remains separate and unchanged.

## Evidence

Code:
- `source/app/LiveIOProofRunner.cpp`
- `tests/LiveIOProofRunnerTests.cpp`
- `docs/superpowers/specs/2026-05-25-p-live6-app-timer-midi-proof.md`

Artifact:
- `debug/p-live1-live-io-proof/live_io_app_timer_midi_report.json`

RED evidence:
- `cmake --build build --target my_world_live_io_proof_runner_tests && ./build/my_world_live_io_proof_runner_tests` failed first on artifact count before `live_io_app_timer_midi_report.json` existed

Verification:
- `cmake --build build --target my_world_live_io_proof_runner_tests my-world && ./build/my_world_live_io_proof_runner_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-live-io-proof-and-exit`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:
- `live io proof runner ok`
- app target `my-world` builds
- app proof writes `live_io_app_timer_midi_report.json`
- `70/70 tests passed`
- `git diff --check passed`

## Parked

- flipping normal app timer mode from UI or preferences
- MIDI teach/learn mode
- realtime audio callback delivery
- OSC app timer loopback proof, split to P-LIVE7
- full live UI indicator
- broader MIDI output operators
