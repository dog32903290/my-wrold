# P-LIVE21 Realtime Delivery Backpressure

## Status

Closed on 2026-05-25.

## Acceptance

- `AudioRealtimeDelivery` uses a bounded multi-slot realtime delivery buffer instead of one overwritten slot.
- App/control side reports skipped unread snapshots and overwritten snapshots separately.
- Callback-side publish remains realtime-safe: fixed storage, atomic/plain writes only, no allocation, locks, file IO, device scanning, JSON parsing, logging, sleeping, UI, MIDI, or OSC.
- Existing app timer and live IO indicator continue to consume realtime telemetry through `AudioRealtimeDeliveryResult`.

## Target Line

```text
audio callback snapshots
-> bounded multi-slot delivery slots
-> app timer consume latest completed snapshot
-> skipped / overwritten telemetry
-> existing live IO status label
```

## Evidence

- `source/audio/AudioRealtimeDelivery.h`
- `source/audio/AudioRealtimeDelivery.cpp`
- `source/core/LiveIOStatusIndicator.h`
- `source/core/LiveIOStatusIndicator.cpp`
- `source/app/MainComponent.cpp`
- `tests/AudioRealtimeDeliveryTests.cpp`
- `tests/LiveIOStatusIndicatorTests.cpp`

## Realtime Safety Notes

`publishFromRealtime()` writes one fixed slot selected by sequence. The slots are statically owned by `AudioRealtimeDelivery`; no queue allocation, lock, file IO, device scanning, JSON parsing, logging, sleeping, UI, MIDI, or OSC work was added to the callback path.

`consumeLatest()` keeps the app side's latest-snapshot behavior, but now reports `skippedSnapshots` and `overwrittenSnapshots` separately. With the current four-slot buffer, missed snapshots inside the retained window are counted as skipped; snapshots beyond that retained window are counted as overwritten.

## Verification

- RED: `cmake --build build --target my_world_audio_realtime_delivery_tests` failed first because `AudioRealtimeDeliveryResult::skippedSnapshots` and `overwrittenSnapshots` did not exist.
- RED: `cmake --build build --target my_world_live_io_status_indicator_tests` failed first because `LiveIORealtimeIndicatorTelemetry::skippedSnapshots` / `overwrittenSnapshots` and matching state fields did not exist.
- GREEN: `cmake --build build --target my_world_audio_realtime_delivery_tests my_world_live_io_status_indicator_tests && ./build/my_world_audio_realtime_delivery_tests && ./build/my_world_live_io_status_indicator_tests`
- App build: `cmake --build build --target my-world`
- Full suite: `ctest --test-dir build --output-on-failure` passed 75/75.
- Diff check: `git diff --check` passed.

## Parked

- Sequential catch-up consume API.
- Dynamically sized queues.
- Direct realtime MIDI/OSC send from the callback remains forbidden.
