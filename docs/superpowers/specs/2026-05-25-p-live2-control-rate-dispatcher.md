# P-LIVE2 Control-Rate Live IO Dispatcher Closure

Date: 2026-05-25 12:38 Asia/Taipei.

Status: closed.

## Question

Can the live IO proof process multiple loudness frames through a control-rate dispatcher, rate-limit dispatches, call MIDI/OSC sinks, and report sent/skipped counts without moving work into the realtime audio callback or adding UI?

## Closed Line

```text
synthetic loudness control frames
-> LiveIOBus MIDI/OSC/uniform mapping
-> LiveIOControlDispatcher min-interval gate
-> injected MIDI output sender and OSC proof sink
-> rate-limited report with sent/skipped counts
-> live_io_control_dispatch_report.json
```

## Contract

Trigger:
- existing app proof flag `--dump-live-io-proof-and-exit`
- focused `LiveIOControlDispatcher` and `LiveIOProofRunner` tests

Input:
- ordered control frames with `timestampMs` and `LiveIOValueFrame`
- live IO bindings from P-LIVE1
- `minIntervalMs`
- MIDI output inventory and selected identifier
- injected MIDI sender
- injected OSC float sender

Success:
- first eligible frame dispatches immediately
- a frame inside `minIntervalMs` is marked `rate_limited`
- later eligible frames dispatch
- MIDI CC events call the MIDI sender and count sent messages
- OSC float events call the OSC sender and count sent messages
- shader uniform events stay outside device IO and are counted as skipped
- app proof writes a control dispatch report artifact

Failure:
- missing frame input reports `failed`
- unmapped bus frame records the bus error
- missing MIDI sender, unavailable MIDI output, open failure, or send failure records a readable error
- missing OSC sender or failed OSC send records a readable error
- failures return `ok: false`; they do not throw, block the realtime callback, or hide diagnostics

Observability:
- `debug/p-live1-live-io-proof/live_io_control_dispatch_report.json`
- report fields include `kind`, `ok`, `status`, `message`, `frameCount`, `dispatchedFrameCount`, `rateLimitedFrameCount`, `midiSentCount`, `oscSentCount`, `shaderSkippedCount`, per-frame statuses, and `errors`

## Evidence

Code:
- `source/core/LiveIOControlDispatcher.h`
- `source/core/LiveIOControlDispatcher.cpp`
- `source/app/LiveIOProofRunner.h`
- `source/app/LiveIOProofRunner.cpp`
- `source/app/MainComponent.cpp`
- `tests/LiveIOControlDispatcherTests.cpp`
- `tests/LiveIOProofRunnerTests.cpp`
- `CMakeLists.txt`

Artifacts:
- `debug/p-live1-live-io-proof/live_io_control_dispatch_report.json`
- `debug/p-live1-live-io-proof/live_io_midi_send_report.json`
- `debug/p-live1-live-io-proof/live_io_midi_inventory_report.json`
- `debug/p-live1-live-io-proof/live_io_osc_loopback_report.json`
- `debug/p-live1-live-io-proof/live_io_report.json`
- `debug/p-live1-live-io-proof/live_io_send_report.json`
- `debug/p-live1-live-io-proof/live_io_runtime_execution.json`

RED evidence:
- `cmake -S . -B build && cmake --build build --target my_world_live_io_control_dispatcher_tests` failed first on missing `LiveIOControlDispatcher.h`
- `cmake --build build --target my_world_live_io_proof_runner_tests` failed first because `LiveIOProofRunRequest` had no `controlOscSender`

Verification:
- `cmake --build build --target my_world_live_io_control_dispatcher_tests my_world_live_io_proof_runner_tests my-world`
- `./build/my_world_live_io_control_dispatcher_tests`
- `./build/my_world_live_io_proof_runner_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-live-io-proof-and-exit`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted app artifact:
- `debug/p-live1-live-io-proof/live_io_control_dispatch_report.json` has `ok: true`, `status: "dispatched"`, `frameCount: 4`, `dispatchedFrameCount: 3`, `rateLimitedFrameCount: 1`, `midiSentCount: 3`, `oscSentCount: 3`, `shaderSkippedCount: 3`, and no errors
- `68/68 tests passed`
- `git diff --check passed`

## Parked

- realtime audio callback delivery
- live UI mapping and indicators
- MIDI teach/learn mode
- note, pitchbend, sysex, trigger, and scheduler output operators
- external OSC/UDP targets
- always-on OSC receive nodes/server
