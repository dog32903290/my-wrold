# P-LIVE1.2 Live IO Send Boundary Closure

Date: 2026-05-25 11:34 Asia/Taipei.

Status: closed.

## Question

Can mapped `LiveIOBus` events cross into a MIDI/OSC send boundary as controlled dry-run actions, without opening devices, sending UDP packets, touching realtime callbacks, or adding UI?

## Closed Line

```text
loaded compound.loudness public outputs
-> LiveIOBus target events
-> LiveIOSendAdapter dry-run route
-> MIDI CC dry-run action and OSC float dry-run action
-> live_io_send_report.json
```

## Contract

Trigger:
- existing app proof flag `--dump-live-io-proof-and-exit`
- focused unit tests for `LiveIOSendAdapter`

Input:
- `LiveIOBusReport` with `midi.cc`, `osc.float`, and `shader.uniform` events
- dry-run route: MIDI output label, OSC host, OSC port

Success:
- MIDI CC events become dry-run MIDI actions with output name, channel, CC, and value
- OSC float events become dry-run OSC actions with host, port, address, and float value
- shader uniform events are reported as skipped because they are not external device sends
- no real send occurs; report actions have `sent: false`

Failure:
- blocked bus reports block the send boundary with a readable error
- missing MIDI output blocks MIDI send actions
- missing or invalid OSC endpoint blocks OSC send actions

Observability:
- `debug/p-live1-live-io-proof/live_io_send_report.json`
- report fields include `kind`, `ok`, `status`, `message`, `actions`, `skipped`, and `errors`

## Evidence

Code:
- `source/core/LiveIOSendAdapter.h`
- `source/core/LiveIOSendAdapter.cpp`
- `source/app/LiveIOProofRunner.cpp`
- `tests/LiveIOSendAdapterTests.cpp`
- `tests/LiveIOProofRunnerTests.cpp`
- `CMakeLists.txt`

Artifacts:
- `debug/p-live1-live-io-proof/live_io_report.json`
- `debug/p-live1-live-io-proof/live_io_send_report.json`
- `debug/p-live1-live-io-proof/live_io_runtime_execution.json`

RED evidence:
- `cmake -S . -B build && cmake --build build --target my_world_live_io_send_adapter_tests` failed first on missing `LiveIOSendAdapter.h`
- `./build/my_world_live_io_proof_runner_tests` failed first on missing third artifact

Verification:
- `cmake --build build --target my_world_live_io_send_adapter_tests my_world_live_io_bus_tests my_world_live_io_proof_runner_tests my_world_startup_proof_tests my-world`
- `./build/my_world_live_io_send_adapter_tests`
- `./build/my_world_live_io_bus_tests`
- `./build/my_world_live_io_proof_runner_tests`
- `./build/my_world_startup_proof_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-live-io-proof-and-exit`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `live io send adapter ok`
- `live io bus ok`
- `live io proof runner ok`
- `debug/p-live1-live-io-proof/live_io_send_report.json` has `ok: true`, `status: "dry_run"`, MIDI CC value `64`, OSC endpoint `127.0.0.1:9000`, `sent: false`, and skipped `shader.uniform: uniform.loudness`
- `65/65 tests passed`

## Parked

- real MIDI device opening and send
- real OSC UDP send/receive
- MIDI teach/learn mode
- realtime audio callback wiring
- live UI mapping and indicators
