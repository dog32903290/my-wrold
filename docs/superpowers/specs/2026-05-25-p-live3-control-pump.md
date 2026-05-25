# P-LIVE3 Control-Rate Analyzer Snapshot Pump Closure

Date: 2026-05-25 12:51 Asia/Taipei.

Status: closed.

## Question

Can analyzer snapshots be converted into control-rate live IO frames, tick-rate-limited, and handed to the live IO dispatcher without moving device IO into the realtime audio callback or wiring the app timer yet?

## Closed Line

```text
AudioAnalyzerSnapshot ticks
-> LiveIOControlPump tick interval gate
-> LiveIOValueFrame from snapshot.loudness
-> LiveIOControlDispatcher
-> injected MIDI output sender and OSC proof sink
-> live_io_control_pump_report.json
```

## Contract

Trigger:
- existing app proof flag `--dump-live-io-proof-and-exit`
- focused `LiveIOControlPump` and `LiveIOProofRunner` tests

Input:
- ordered control pump ticks with `timestampMs` and `AudioAnalyzerSnapshot`
- live IO bindings
- tick interval in milliseconds
- dispatcher minimum interval in milliseconds
- MIDI output inventory and selected identifier
- injected MIDI sender
- injected OSC float sender

Success:
- active snapshot ticks outside the tick interval become `LiveIOValueFrame` entries with public `out`
- fast ticks inside the tick interval are marked `tick_rate_limited`
- inactive snapshots are marked `inactive` and do not produce frames
- framed ticks are handed to `LiveIOControlDispatcher`
- report records tick counts, frame counts, inactive count, tick-rate-limited count, last active loudness, last sample counter, nested dispatch report, and errors

Failure:
- missing ticks reports `failed`
- no active frames reports `failed`
- dispatcher failures propagate as pump failures with readable errors
- failures return `ok: false`; they do not throw or hide diagnostics

Observability:
- `debug/p-live1-live-io-proof/live_io_control_pump_report.json`
- report fields include `kind`, `ok`, `status`, `message`, `tickCount`, `frameCount`, `inactiveTickCount`, `tickRateLimitedCount`, `lastLoudness`, `lastSampleCounter`, per-tick statuses, nested `dispatch`, and `errors`

## Realtime Law

This lane keeps the realtime callback clean:
- audio callback updates analyzer state only
- no MIDI send from the callback
- no OSC send from the callback
- no device discovery/opening from the callback
- no allocation, file IO, logging, or JSON work from the callback

## Evidence

Code:
- `source/core/LiveIOControlPump.h`
- `source/core/LiveIOControlPump.cpp`
- `source/core/LiveIOControlDispatcher.h`
- `source/core/LiveIOControlDispatcher.cpp`
- `source/app/LiveIOProofRunner.h`
- `source/app/LiveIOProofRunner.cpp`
- `tests/LiveIOControlPumpTests.cpp`
- `tests/LiveIOProofRunnerTests.cpp`
- `CMakeLists.txt`

Artifacts:
- `debug/p-live1-live-io-proof/live_io_control_pump_report.json`
- `debug/p-live1-live-io-proof/live_io_control_dispatch_report.json`
- `debug/p-live1-live-io-proof/live_io_midi_send_report.json`
- `debug/p-live1-live-io-proof/live_io_midi_inventory_report.json`
- `debug/p-live1-live-io-proof/live_io_osc_loopback_report.json`
- `debug/p-live1-live-io-proof/live_io_report.json`
- `debug/p-live1-live-io-proof/live_io_send_report.json`
- `debug/p-live1-live-io-proof/live_io_runtime_execution.json`

RED evidence:
- `cmake -S . -B build && cmake --build build --target my_world_live_io_control_pump_tests` failed first on missing `LiveIOControlPump.h`
- `./build/my_world_live_io_proof_runner_tests` failed first on artifact count before `live_io_control_pump_report.json` existed

Verification:
- `cmake --build build --target my_world_live_io_control_pump_tests my_world_live_io_proof_runner_tests my-world`
- `./build/my_world_live_io_control_pump_tests`
- `./build/my_world_live_io_proof_runner_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-live-io-proof-and-exit`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted app artifact:
- `debug/p-live1-live-io-proof/live_io_control_pump_report.json` has `ok: true`, `status: "pumped"`, `tickCount: 5`, `frameCount: 3`, `inactiveTickCount: 1`, `tickRateLimitedCount: 1`, `lastSampleCounter: 320`, nested dispatch `midiSentCount: 3`, `oscSentCount: 3`, `shaderSkippedCount: 3`, and no errors
- `69/69 tests passed`
- `git diff --check passed`

## Parked

- actual `MainComponent::timerCallback()` wiring
- realtime audio callback delivery
- live UI mapping and indicators
- MIDI teach/learn mode
- note, pitchbend, sysex, trigger, and scheduler output operators
- external OSC/UDP targets
- always-on OSC receive nodes/server
