# P-LIVE1.3 Controlled OSC Loopback Proof Closure

Date: 2026-05-25 12:02 Asia/Taipei.

Status: closed.

## Question

Can a mapped OSC float event be sent through a controlled localhost UDP/OSC boundary and received back as proof, without opening MIDI devices, sending to external UDP targets, touching realtime callbacks, or adding UI?

## Closed Line

```text
loaded compound.loudness public outputs
-> LiveIOBus OSC float event
-> LiveIOSendAdapter controlled loopback route
-> localhost UDP/OSC packet
-> proof receiver parses address + float
-> live_io_osc_loopback_report.json
```

## Contract

Trigger:
- existing app proof flag `--dump-live-io-proof-and-exit`
- focused `LiveIOSendAdapter` and `LiveIOProofRunner` tests

Input:
- `LiveIOBusReport` with the proof `osc.loudness` event
- controlled loopback route: host `127.0.0.1`, ephemeral receiver port

Success:
- OSC event is encoded as an OSC float message
- UDP packet is sent only to the local loopback receiver
- receiver reads address `/my-world/loudness` and float value `0.5`
- report records `sendStatus: "controlled_send"`, `sent: true`, and `received: true`

Failure:
- non-loopback controlled send is blocked
- invalid endpoint, send failure, timeout, malformed OSC type tag, or missing float payload records a readable error
- MIDI events are skipped in controlled OSC route as `midi.disabled`

Observability:
- `debug/p-live1-live-io-proof/live_io_osc_loopback_report.json`
- report fields include `kind`, `ok`, `status`, `sendStatus`, `sent`, `received`, `oscHost`, `oscPort`, `oscAddress`, `receivedFloatValue`, and `errors`

## Evidence

Code:
- `source/core/LiveIOSendAdapter.h`
- `source/core/LiveIOSendAdapter.cpp`
- `source/app/LiveIOProofRunner.cpp`
- `tests/LiveIOSendAdapterTests.cpp`
- `tests/LiveIOProofRunnerTests.cpp`

Artifacts:
- `debug/p-live1-live-io-proof/live_io_osc_loopback_report.json`
- `debug/p-live1-live-io-proof/live_io_send_report.json`
- `debug/p-live1-live-io-proof/live_io_report.json`
- `debug/p-live1-live-io-proof/live_io_runtime_execution.json`

RED evidence:
- `cmake --build build --target my_world_live_io_send_adapter_tests && ./build/my_world_live_io_send_adapter_tests` failed first on missing `makeLiveIOControlledOscLoopbackRoute` and `executeLiveIOSendBoundary`
- after the first implementation, the same test failed on OSC float byte order
- `./build/my_world_live_io_proof_runner_tests` failed first on missing fourth artifact

Verification:
- `cmake --build build --target my_world_live_io_send_adapter_tests my_world_live_io_proof_runner_tests my-world`
- `./build/my_world_live_io_send_adapter_tests`
- `./build/my_world_live_io_proof_runner_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-live-io-proof-and-exit`

Latest accepted app artifact:
- `debug/p-live1-live-io-proof/live_io_osc_loopback_report.json` has `ok: true`, `status: "received"`, `sendStatus: "controlled_send"`, `sent: true`, `received: true`, address `/my-world/loudness`, and `receivedFloatValue: 0.500000`

## Parked

- real MIDI device opening and send
- external OSC/UDP targets
- OSC receive nodes or always-on UDP server
- MIDI teach/learn mode
- realtime audio callback wiring
- live UI mapping and indicators
