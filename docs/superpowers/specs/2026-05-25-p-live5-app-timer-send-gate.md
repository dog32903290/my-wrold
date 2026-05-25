# P-LIVE5 App Timer Send Mode Gate Closure

Date: 2026-05-25 13:29 Asia/Taipei.

Status: closed.

## Question

Can the app timer live IO body carry an explicit send-mode gate so dry-run remains the default and controlled send only happens when a caller deliberately selects it and injects senders?

## Closed Line

```text
MainComponent::timerCallback()
-> updateAudioMeters()
-> LiveIOControlTimerConfig sendMode
-> dryRun gate uses fake MIDI/OSC senders
-> controlledSend gate uses injected MIDI/OSC senders
-> LiveIOControlTimerState separates dry-run counts from controlled-send counts
```

## Contract

Trigger:
- app timer calls `MainComponent::timerCallback()` at 30 Hz
- focused `LiveIOControlTimer` test drives synthetic timestamps and send modes

Input:
- current `AudioAnalyzerSnapshot`
- timer timestamp in milliseconds
- live IO bindings for loudness to MIDI CC, OSC float, and shader uniform
- `LiveIOControlTimerSendMode`
- optional MIDI inventory, selected identifier, MIDI sender, and OSC sender

Success:
- `dryRun` mode never calls provided MIDI/OSC senders
- `dryRun` mode records `midiDryRunCount` and `oscDryRunCount`
- `controlledSend` mode calls injected MIDI/OSC senders
- `controlledSend` mode records `midiControlledSendCount` and `oscControlledSendCount`
- state records `lastSendMode`
- app timer path carries the send-mode gate and still defaults to `dryRun`

Failure:
- `controlledSend` without required sender/inventory fails with readable status and errors
- failure records state errors and returns `ok: false`
- failures do not throw or hide diagnostics

Observability:
- `makeLiveIOControlTimerStateJson()` includes dry-run counts, controlled-send counts, `lastStatus`, `lastSendMode`, and errors
- app status row keeps showing compact live IO status and count pairs

## Realtime Law

This lane keeps the realtime callback clean:
- audio callback updates analyzer state only
- no live IO send from the audio callback
- no device discovery/opening from the audio callback
- no allocation, file IO, logging, or JSON work from the audio callback

This lane does not enable real app timer MIDI/OSC by default:
- `MainComponent` keeps `liveIOSendMode = dryRun`
- controlled send exists as an explicit core gate with injected senders only

Existing preference MIDI streaming remains separate and unchanged.

## Evidence

Code:
- `source/core/LiveIOControlTimer.h`
- `source/core/LiveIOControlTimer.cpp`
- `tests/LiveIOControlTimerTests.cpp`
- `source/app/MainComponent.h`
- `source/app/MainComponent.cpp`

RED evidence:
- `cmake --build build --target my_world_live_io_control_timer_tests` failed first on missing `LiveIOControlTimerConfig`, `LiveIOControlTimerSendMode`, `tickLiveIOControlTimer`, `midiControlledSendCount`, `oscControlledSendCount`, and `lastSendMode`

Verification:
- `cmake --build build --target my_world_live_io_control_timer_tests && ./build/my_world_live_io_control_timer_tests`
- `cmake --build build --target my_world_live_io_control_timer_tests my-world && ./build/my_world_live_io_control_timer_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-live-io-proof-and-exit`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:
- `live io control timer ok`
- app target `my-world` builds
- live IO CLI proof exits successfully
- `70/70 tests passed`
- `git diff --check passed`

## Parked

- flipping app `liveIOSendMode` from UI or preferences
- real MIDI/OSC dispatch from the default app timer path
- MIDI teach/learn mode
- realtime audio callback delivery
- full live UI indicator
- note, pitchbend, sysex, trigger, and scheduler output operators
- external OSC/UDP targets
- always-on OSC receive nodes/server
