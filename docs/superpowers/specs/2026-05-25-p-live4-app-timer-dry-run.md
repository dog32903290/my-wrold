# P-LIVE4 App Timer Dry-Run Wiring Closure

Date: 2026-05-25 13:02 Asia/Taipei.

Status: closed.

## Question

Can the app `juce::Timer` drive a live IO control body from the current analyzer snapshot while the new live IO path remains dry-run only?

## Closed Line

```text
MainComponent::timerCallback()
-> updateAudioMeters()
-> AudioInputAnalyzer snapshot through loudness runtime bridge
-> LiveIOControlTimerDryRunConfig app bindings
-> tickLiveIOControlTimerDryRun()
-> LiveIOControlPump with injected dry-run MIDI/OSC senders
-> LiveIOControlTimerState and live IO status text
```

## Contract

Trigger:
- app timer calls `MainComponent::timerCallback()` at 30 Hz
- focused `LiveIOControlTimer` test drives synthetic timestamps

Input:
- current `AudioAnalyzerSnapshot`
- timer timestamp in milliseconds
- live IO bindings for loudness to MIDI CC, OSC float, and shader uniform
- timer tick interval in milliseconds
- dispatcher minimum interval in milliseconds

Success:
- enabled active ticks outside the timer interval pump one dry-run live IO control frame
- fast active ticks are marked `tick_rate_limited`
- inactive ticks are marked `inactive`
- disabled ticks are marked `disabled`
- state records tick, pump, inactive, rate-limited, MIDI dry-run, OSC dry-run, shader skipped, last loudness, last sample counter, and last status
- `MainComponent` shows the dry-run status alongside the existing MIDI status row

Failure:
- pump failures return `ok: false`
- state records readable status, message, and errors
- failures do not throw or hide diagnostics

Observability:
- `makeLiveIOControlTimerStateJson()` exposes the dry-run timer state for tests and future proof dumps
- app status row shows `live io dry <status> m<count> o<count>`

## Realtime Law

This lane keeps the realtime callback clean:
- audio callback updates analyzer state only
- no new live IO MIDI send from the audio callback
- no new live IO OSC send from the audio callback
- no device discovery/opening from the audio callback
- no allocation, file IO, logging, or JSON work from the audio callback

This lane also keeps the new app timer path dry-run:
- MIDI sends use an injected fake sender inside `LiveIOControlTimer`
- OSC sends use an injected fake sender inside `LiveIOControlTimer`
- shader uniform targets are counted as skipped, not applied through a renderer binding

Existing preference MIDI streaming remains separate and unchanged.

## Evidence

Code:
- `source/core/LiveIOControlTimer.h`
- `source/core/LiveIOControlTimer.cpp`
- `tests/LiveIOControlTimerTests.cpp`
- `source/app/MainComponent.h`
- `source/app/MainComponent.cpp`
- `CMakeLists.txt`

RED evidence:
- `cmake -S . -B build && cmake --build build --target my_world_live_io_control_timer_tests` failed first because `source/core/LiveIOControlTimer.cpp` did not exist

Verification:
- `cmake -S . -B build && cmake --build build --target my_world_live_io_control_timer_tests` failed RED first on missing `LiveIOControlTimer.cpp`
- `cmake -S . -B build && cmake --build build --target my_world_live_io_control_timer_tests && ./build/my_world_live_io_control_timer_tests`
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

- real MIDI/OSC dispatch from the app timer path
- realtime audio callback delivery
- live UI indicator beyond compact status text
- MIDI teach/learn mode
- note, pitchbend, sysex, trigger, and scheduler output operators
- external OSC/UDP targets
- always-on OSC receive nodes/server
