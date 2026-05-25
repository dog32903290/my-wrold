# P-LIVE1.4 MIDI Output Inventory / Route Report Closure

Date: 2026-05-25 12:13 Asia/Taipei.

Status: closed.

## Question

Can the live IO proof capture available MIDI output devices and report selected/unavailable routes, without opening a MIDI device, sending MIDI, touching realtime callbacks, or adding UI?

## Closed Line

```text
JUCE MidiOutput::getAvailableDevices()
-> LiveIOMidiOutputInventory
-> selected route report when a device exists
-> unavailable route report
-> live_io_midi_inventory_report.json
```

## Contract

Trigger:
- existing app proof flag `--dump-live-io-proof-and-exit`
- focused `LiveIOMidiOutputInventory` and `LiveIOProofRunner` tests

Input:
- MIDI output device list with `name` and `identifier`
- route request by identifier or name

Success:
- inventory report lists device count and device identifiers
- route by identifier/name returns `selected` with selected device fields
- missing route returns `unavailable` with a readable error
- app proof remains `ok` when no MIDI outputs exist; route selection is then reported unavailable instead of failing the proof

Failure:
- a requested output that is not present reports `status: "unavailable"`
- no MIDI send or device open is attempted in this lane

Observability:
- `debug/p-live1-live-io-proof/live_io_midi_inventory_report.json`
- report fields include `kind`, `ok`, `status`, `deviceCount`, `devices`, `selectedRoute`, and `unavailableRoute`

## Evidence

Code:
- `source/core/LiveIOMidiOutputInventory.h`
- `source/core/LiveIOMidiOutputInventory.cpp`
- `source/app/LiveIOProofRunner.h`
- `source/app/LiveIOProofRunner.cpp`
- `source/app/MainComponent.cpp`
- `tests/LiveIOMidiOutputInventoryTests.cpp`
- `tests/LiveIOProofRunnerTests.cpp`
- `CMakeLists.txt`

Artifacts:
- `debug/p-live1-live-io-proof/live_io_midi_inventory_report.json`
- `debug/p-live1-live-io-proof/live_io_report.json`
- `debug/p-live1-live-io-proof/live_io_send_report.json`
- `debug/p-live1-live-io-proof/live_io_osc_loopback_report.json`
- `debug/p-live1-live-io-proof/live_io_runtime_execution.json`

RED evidence:
- `cmake -S . -B build && cmake --build build --target my_world_live_io_midi_output_inventory_tests` failed first on missing `LiveIOMidiOutputInventory.h`
- `./build/my_world_live_io_proof_runner_tests` failed first because `LiveIOProofRunRequest` had no `midiOutputInventory`

Verification:
- `cmake --build build --target my_world_live_io_midi_output_inventory_tests my_world_live_io_proof_runner_tests my-world`
- `./build/my_world_live_io_midi_output_inventory_tests`
- `./build/my_world_live_io_proof_runner_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-live-io-proof-and-exit`

Latest accepted app artifact:
- `debug/p-live1-live-io-proof/live_io_midi_inventory_report.json` has `ok: true`, `status: "inventoried"`, `deviceCount: 1`, selected route for `IAC驅動程式 匯流排1`, and unavailable route for `__missing_live_io_midi_output__`

## Parked

- real MIDI device open
- real MIDI send
- MIDI teach/learn mode
- realtime audio callback wiring
- live UI mapping and indicators
