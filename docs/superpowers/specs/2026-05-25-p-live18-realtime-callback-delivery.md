# P-LIVE18 Realtime Callback Delivery

## Status

Closed on 2026-05-25.

## Acceptance

- Audio callback publishes analyzer snapshots into a bounded realtime-safe delivery slot.
- App/control side consumes only completed snapshots and advances a last-seen sequence.
- Existing `LiveIOControlTimer` remains outside the callback.
- No MIDI/OSC send, allocation, locks, file IO, device scanning, JSON parsing, logging, sleeping, or UI work was added inside the callback.

## Closed Line

```text
AudioInputAnalyzer callback
-> AudioAnalyzerState.processBlock()
-> AudioRealtimeDelivery.publishFromRealtime()
-> MainComponent app timer consumeRealtimeDelivery()
-> existing LiveIOControlTimer outside callback
```

## Evidence

- `source/audio/AudioRealtimeDelivery.h`
- `source/audio/AudioRealtimeDelivery.cpp`
- `source/audio/AudioInputAnalyzer.h`
- `source/audio/AudioInputAnalyzer.cpp`
- `source/app/MainComponent.h`
- `source/app/MainComponent.cpp`
- `tests/AudioRealtimeDeliveryTests.cpp`
- `CMakeLists.txt`

## Realtime Safety Notes

`publishFromRealtime()` uses only atomic stores and a sequence latch. It does not allocate, lock, parse, scan devices, send MIDI/OSC, touch files, log, sleep, or touch UI.

`consumeLatest()` runs on the control/app side. It drops empty/repeated/in-progress snapshots and only returns a completed sequence.

## Verification

- RED: `cmake -S . -B build && cmake --build build --target my_world_audio_realtime_delivery_tests` failed first because `source/audio/AudioRealtimeDelivery.cpp` did not exist.
- GREEN: `ctest --test-dir build --output-on-failure -R "audio_realtime_delivery"`
- App build: `cmake --build build --target my-world`
- Full suite: `ctest --test-dir build --output-on-failure` passed 75/75.

## Parked

- Direct realtime MIDI/OSC send from the callback remains forbidden.
- Multi-slot event queue/backpressure telemetry.
- Dedicated UI indicator for realtime delivery sequence/drop status.
