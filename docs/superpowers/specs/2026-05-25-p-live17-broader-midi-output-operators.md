# P-LIVE17 Broader MIDI Output Operators

## Status

Closed on 2026-05-25.

## Acceptance

- `LiveIOBus` can emit MIDI CC and MIDI note-on events from the same normalized live IO value frame.
- MIDI send proof reports byte-level output for note-on:
  - status byte `0x90 + channel - 1`
  - data1 = note number
  - data2 = velocity
- Control-rate dispatch can send note-on through the existing injected MIDI sender.
- No UI, device scanning, file IO, JSON parsing, or realtime callback delivery was added.

## Closed Line

```text
LiveIO normalized value
-> midi.cc or midi.note_on binding
-> LiveIOMidiMessage
-> injected control-rate sender
-> byte-level proof report
```

## Evidence

- `source/core/LiveIOBus.h`
- `source/core/LiveIOBus.cpp`
- `source/core/LiveIOMidiSendProof.h`
- `source/core/LiveIOMidiSendProof.cpp`
- `source/core/LiveIOControlDispatcher.cpp`
- `tests/LiveIOBusTests.cpp`
- `tests/LiveIOMidiSendProofTests.cpp`
- `tests/LiveIOControlDispatcherTests.cpp`

## Verification

- RED: `cmake --build build --target my_world_live_io_bus_tests my_world_live_io_midi_send_proof_tests my_world_live_io_control_dispatcher_tests` failed first on missing `makeLiveIOMidiNoteOnBinding`, `midiNoteOn`, `midiNote`, and `midiVelocity`.
- GREEN: `ctest --test-dir build --output-on-failure -R "live_io_bus|live_io_midi_send_proof|live_io_control_dispatcher"`
- App build: `cmake --build build --target my-world`

## Parked

- UI selection for MIDI operators.
- MIDI note-off / program-change / pitch-bend operators.
- Realtime callback delivery.
