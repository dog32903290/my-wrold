# P-LIVE19 Realtime Delivery Status

## Status

Closed on 2026-05-25.

## Acceptance

- App/control side can distinguish realtime delivery states: empty, writing, repeated, delivered.
- Delivered snapshots report the completed sequence and single-slot overwrite count.
- The app audio status surface shows realtime delivery sequence/drop telemetry from the timer side.
- No MIDI/OSC send, allocation, locks, file IO, device scanning, JSON parsing, logging, sleeping, or UI work was added inside the audio callback.

## Closed Line

```text
AudioRealtimeDelivery.consumeLatest()
-> AudioRealtimeDeliveryResult status / sequence / droppedSnapshots
-> MainComponent app timer status text
-> existing audio status label
```

## Evidence

- `source/audio/AudioRealtimeDelivery.h`
- `source/audio/AudioRealtimeDelivery.cpp`
- `source/app/MainComponent.cpp`
- `tests/AudioRealtimeDeliveryTests.cpp`

## Realtime Safety Notes

`publishFromRealtime()` still performs only atomic stores and the sequence latch. The new status text is built on the app/control side after `consumeLatest()`.

Single-slot overwrite telemetry is intentionally coarse: it reports how many completed snapshots were replaced before the app side consumed the latest one.

## Verification

- RED: `cmake --build build --target my_world_audio_realtime_delivery_tests` failed first because `AudioRealtimeDeliveryResult` did not expose `status` / `droppedSnapshots`, and `makeAudioRealtimeDeliveryStatusText()` did not exist.
- GREEN: `cmake --build build --target my_world_audio_realtime_delivery_tests && ./build/my_world_audio_realtime_delivery_tests`
- App build: `cmake --build build --target my-world`
- Focused CTest: `ctest --test-dir build --output-on-failure -R "audio_realtime_delivery"`

## Parked

- Multi-slot realtime event queue/backpressure telemetry.
- Dedicated standalone realtime delivery UI widget.
- Direct realtime MIDI/OSC send from the callback remains forbidden.
