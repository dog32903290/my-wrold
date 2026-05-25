# P-LIVE7 Controlled App Timer OSC Loopback Closure

Date: 2026-05-25 13:36 Asia/Taipei.

Status: closed.

## Question

Can the opt-in live IO proof path switch the app timer body to `controlledSend` and prove the OSC boundary by sending one float packet to a controlled localhost receiver?

## Closed Line

```text
--dump-live-io-proof-and-exit
-> LiveIOProofRunner
-> LiveIOControlTimerConfig(sendMode: controlledSend)
-> active AudioAnalyzerSnapshot
-> LiveIOControlPump
-> LiveIOControlDispatcher
-> injected OSC sender
-> LiveIOSendAdapter controlled loopback
-> localhost UDP/OSC packet received
-> live_io_app_timer_osc_loopback_report.json
```

## Contract

Trigger:
- explicit app proof flag `--dump-live-io-proof-and-exit`
- focused `LiveIOProofRunner` test

Input:
- active analyzer snapshot
- local OSC receiver bound to loopback
- live IO bindings for loudness to MIDI CC, OSC float, and shader uniform

Success:
- app timer body runs in `controlledSend`
- MIDI is disabled and OSC is enabled for this proof
- injected OSC sender is called once
- `LiveIOSendAdapter` sends one OSC float packet to localhost
- receiver reads `/my-world/loudness`
- received float is `0.5`
- report writes `live_io_app_timer_osc_loopback_report.json`

Failure:
- receiver bind/read failure returns failed proof status
- OSC send failure returns failed proof status
- proof runner does not hide diagnostics

Observability:
- report fields include `kind`, `ok`, `status`, `timerStatus`, `sendMode`, controlled-send counts, `shaderSkippedCount`, receive flag, endpoint, address, received float value, and errors

## Realtime Law

This lane keeps the realtime callback clean:
- audio callback updates analyzer state only
- no OSC send from the audio callback
- no socket work from the audio callback
- no device discovery/opening from the audio callback
- no allocation, file IO, logging, or JSON work from the audio callback

This lane does not change normal app behavior:
- `MainComponent` still defaults to `dryRun`
- controlled OSC happens only inside the explicit proof run

## Evidence

Code:
- `source/app/LiveIOProofRunner.cpp`
- `tests/LiveIOProofRunnerTests.cpp`
- `docs/superpowers/specs/2026-05-25-p-live7-app-timer-osc-loopback.md`

Artifact:
- `debug/p-live1-live-io-proof/live_io_app_timer_osc_loopback_report.json`

RED evidence:
- `cmake --build build --target my_world_live_io_proof_runner_tests && ./build/my_world_live_io_proof_runner_tests` failed first on artifact count before `live_io_app_timer_osc_loopback_report.json` existed

Verification:
- `cmake --build build --target my_world_live_io_proof_runner_tests && ./build/my_world_live_io_proof_runner_tests`
- `cmake --build build --target my_world_live_io_proof_runner_tests my-world && ./build/my_world_live_io_proof_runner_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-live-io-proof-and-exit`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:
- `live io proof runner ok`
- app target `my-world` builds
- app proof writes `live_io_app_timer_osc_loopback_report.json`
- `70/70 tests passed`
- `git diff --check passed`

## Parked

- flipping normal app timer mode from UI or preferences
- external OSC/UDP targets
- always-on OSC receive nodes/server
- MIDI teach/learn mode
- realtime audio callback delivery
- full live UI indicator
