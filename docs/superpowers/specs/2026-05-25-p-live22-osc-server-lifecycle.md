# P-LIVE22 OSC Server Lifecycle

## Status

Closed on 2026-05-25.

## Acceptance

- `LiveIOAppController` owns an app/control-side OSC receiver lifecycle.
- App timer tick opens the receiver from sanitized live IO preferences, polls once, and exposes receiver status.
- A matching OSC float datagram becomes a `LiveIOValueFrame` value with receiver evidence.
- Receiver closes/reopens when preferences change.
- No OSC receive work is added to the audio callback.

## Target Line

```text
LiveIOPreferences OSC receive config
-> LiveIOAppController receiver lifecycle
-> app timer poll
-> LiveIOValueFrame from incoming OSC float
-> status evidence
```

## Evidence

- `source/app/LiveIOAppController.h`
- `source/app/LiveIOAppController.cpp`
- `tests/LiveIOAppControllerTests.cpp`

## Lifecycle Notes

`LiveIOAppController` owns `LiveIOOscReceiver`. `applyLiveIOPreferences()` closes an open receiver when host, port, address, or value id changes; the next `tick()` opens the receiver again from sanitized live IO preferences.

`tick()` polls once with timeout `0` on the app/control side. A matching OSC float datagram becomes `LiveIOValueFrame` via the existing `makeLiveIOValueFrameFromOscReceive()` helper. Non-matching or missing datagrams leave the frame empty and report poll status.

No audio callback code changed.

## Verification

- RED: `cmake --build build --target my_world_live_io_app_controller_tests` failed first because `LiveIOAppTimerResult` did not expose OSC receiver lifecycle fields.
- GREEN: `cmake --build build --target my_world_live_io_app_controller_tests && ./build/my_world_live_io_app_controller_tests`
- Focused CTest: `ctest --test-dir build --output-on-failure -R "live_io_app_controller|live_io_osc_receiver"`
- App build: `cmake --build build --target my-world`
- Full suite: `ctest --test-dir build --output-on-failure` passed 75/75.
- Diff check: `git diff --check` passed.

## Parked

- Graph OSC input node.
- UI fields for separate receive config.
- Dynamic OSC address routing/scanning.
- Direct realtime MIDI/OSC send from the audio callback.
