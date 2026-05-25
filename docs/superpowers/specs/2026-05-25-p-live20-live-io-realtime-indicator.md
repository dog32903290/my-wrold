# P-LIVE20 Live IO Realtime Indicator

## Status

Closed on 2026-05-25.

## Acceptance

- Existing live IO status indicator can carry app-side realtime delivery telemetry.
- Indicator text and JSON expose realtime delivery status, sequence, and dropped snapshot count.
- `MainComponent` feeds the latest `AudioRealtimeDeliveryResult` into the live IO indicator on the app timer side.
- No MIDI/OSC send, allocation, locks, file IO, device scanning, JSON parsing, logging, sleeping, or UI work is added inside the audio callback.

## Target Line

```text
AudioRealtimeDeliveryResult
-> MainComponent app timer
-> LiveIOStatusIndicatorState realtime fields
-> existing live IO status label
```

## Evidence

- `source/core/LiveIOStatusIndicator.h`
- `source/core/LiveIOStatusIndicator.cpp`
- `source/app/MainComponent.h`
- `source/app/MainComponent.cpp`
- `tests/LiveIOStatusIndicatorTests.cpp`

## Realtime Safety Notes

The new indicator telemetry is applied after `AudioRealtimeDeliveryResult` is consumed on the app timer side. `publishFromRealtime()` is untouched.

## Verification

- RED: `cmake --build build --target my_world_live_io_status_indicator_tests` failed first because `LiveIORealtimeIndicatorTelemetry` and `withLiveIORealtimeTelemetry()` did not exist.
- GREEN: `cmake --build build --target my_world_live_io_status_indicator_tests && ./build/my_world_live_io_status_indicator_tests`
- App build: `cmake --build build --target my-world`
- Focused CTest: `ctest --test-dir build --output-on-failure -R "live_io_status_indicator"`
- Full suite: `ctest --test-dir build --output-on-failure` passed 75/75.
- Diff check: `git diff --check` passed.

## Parked

- Multi-slot realtime event queue/backpressure telemetry.
- New standalone UI widget.
- Direct realtime MIDI/OSC send from the callback remains forbidden.
