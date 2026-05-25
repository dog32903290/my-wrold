# P-LIVE1.5 Controlled MIDI Open/Send Proof Closure

Date: 2026-05-25 12:24 Asia/Taipei.

Status: closed.

## Question

Can the live IO proof open the selected MIDI output and send one mapped loudness CC message, without moving work into the realtime callback, adding teach mode, or building live UI mapping?

## Closed Line

```text
loaded compound.loudness public out
-> LiveIOBus midi.loudness event
-> LiveIOMidiSendProof selected output route
-> controlled device sender boundary
-> JUCE MidiOutput::openDevice()
-> MidiMessage::controllerEvent(ch1, cc20, value64)
-> live_io_midi_send_report.json
```

## Contract

Trigger:
- existing app proof flag `--dump-live-io-proof-and-exit`
- focused `LiveIOMidiSendProof` and `LiveIOProofRunner` tests

Input:
- a mapped `LiveIOBus` MIDI CC event
- MIDI output inventory from P-LIVE1.4
- route request by selected output identifier
- injectable sender boundary for tests and app JUCE send

Success:
- mapped event becomes a sanitized MIDI CC message with channel `1..16`, CC `0..127`, and value `0..127`
- selected route opens the requested output
- sender reports `opened: true` and `sent: true`
- proof report records selected output, binding id, channel, CC, value, status byte, data1, data2, and errors

Failure:
- missing route reports `status: "unavailable"` and does not call the sender
- missing sender reports `status: "sender_unavailable"`
- open failure reports `status: "open_failed"`
- send failure reports `status: "send_failed"`

Observability:
- `debug/p-live1-live-io-proof/live_io_midi_send_report.json`
- report fields include `kind`, `ok`, `status`, `message`, `selectedName`, `selectedIdentifier`, `opened`, `sent`, `bindingId`, `channel`, `cc`, `value`, `statusByte`, `data1`, `data2`, and `errors`

## Evidence

Code:
- `source/core/LiveIOMidiSendProof.h`
- `source/core/LiveIOMidiSendProof.cpp`
- `source/app/LiveIOProofRunner.h`
- `source/app/LiveIOProofRunner.cpp`
- `source/app/MainComponent.cpp`
- `tests/LiveIOMidiSendProofTests.cpp`
- `tests/LiveIOProofRunnerTests.cpp`
- `CMakeLists.txt`

Artifacts:
- `debug/p-live1-live-io-proof/live_io_midi_send_report.json`
- `debug/p-live1-live-io-proof/live_io_midi_inventory_report.json`
- `debug/p-live1-live-io-proof/live_io_report.json`
- `debug/p-live1-live-io-proof/live_io_send_report.json`
- `debug/p-live1-live-io-proof/live_io_osc_loopback_report.json`
- `debug/p-live1-live-io-proof/live_io_runtime_execution.json`

RED evidence:
- `cmake -S . -B build && cmake --build build --target my_world_live_io_midi_send_proof_tests` failed first on missing `LiveIOMidiSendProof.h`
- `cmake --build build --target my_world_live_io_proof_runner_tests` failed first because `LiveIOProofRunRequest` had no `midiOutputSender`

Verification:
- `cmake --build build --target my_world_live_io_midi_send_proof_tests my_world_live_io_proof_runner_tests my-world`
- `./build/my_world_live_io_midi_send_proof_tests`
- `./build/my_world_live_io_proof_runner_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-live-io-proof-and-exit`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted app artifact:
- `debug/p-live1-live-io-proof/live_io_midi_send_report.json` has `ok: true`, `status: "sent"`, `opened: true`, `sent: true`, selected route for `IAC驅動程式 匯流排1`, channel `1`, CC `20`, value `64`, status byte `176`, data1 `20`, and data2 `64`
- `67/67 tests passed`
- `git diff --check passed`

## Parked

- MIDI teach/learn mode
- realtime audio callback delivery
- live UI mapping and indicators
- note, pitchbend, sysex, trigger, and scheduler output operators
- persistent user-selected routing preferences for the proof path
- external OSC/UDP targets and always-on OSC receive nodes/server
