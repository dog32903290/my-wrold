# P-LIVE10 MIDI Teach Closure

Date: 2026-05-25 14:04 Asia/Taipei.

## Question

Can MIDI teach arm a specific live IO MIDI CC target, learn the next incoming control-change message, update the app MIDI preferences, and then disarm without doing UI work in the high-priority MIDI callback?

## Contract

Input:

- `LiveIOMidiTeachTarget`
- `LiveIOMidiTeachIncomingMessage`
- app PreferencesPanel teach buttons
- JUCE MIDI input callback while teach is armed

Output:

- `LiveIOMidiTeachState`
- learned channel and CC
- updated loudness/map CC preferences
- readable teach status text

Rules:

- Teach starts idle.
- Arming requires a learnable target: `loudness_cc` or `map_cc`.
- Non-CC MIDI messages do not learn and keep teach armed.
- The next CC clamps channel to `1..16`, clamps CC to `0..127`, records the target, and disarms.
- App MIDI input listening is temporary and only starts after the user presses a teach button.
- The MIDI callback only parses a CC and posts to the message thread.
- UI and preference changes happen on the message thread.
- Teach removes its MIDI callback and disables only inputs it temporarily enabled.

## Proven Flow

```text
PreferencesPanel Learn Loudness / Learn Map
-> MainComponent::armMidiTeach()
-> temporary MIDI input callback
-> incoming MIDI CC parsed only
-> MessageManager::callAsync
-> handleLiveIOMidiTeachMessage()
-> PreferencesPanel::applyLearnedMidiCc()
-> emitMidiPreferences()
-> MIDI teach listener removed
```

## Evidence

- `source/core/LiveIOMidiTeach.h`
- `source/core/LiveIOMidiTeach.cpp`
- `tests/LiveIOMidiTeachTests.cpp`
- `source/app/PreferencesPanel.h`
- `source/app/PreferencesPanel.cpp`
- `source/app/MainComponent.h`
- `source/app/MainComponent.cpp`
- `CMakeLists.txt`

## Verification

RED:

- `cmake -S . -B build && cmake --build build --target my_world_live_io_midi_teach_tests` failed because `LiveIOMidiTeach.h` did not exist.

GREEN:

- `cmake --build build --target my_world_live_io_midi_teach_tests my-world && ./build/my_world_live_io_midi_teach_tests`
- `cmake --build build --target my_world_live_io_midi_teach_tests my-world && ./build/my_world_live_io_midi_teach_tests && ctest --test-dir build --output-on-failure && git diff --check`

Latest accepted result:

- `live io midi teach ok`
- app target `my-world` builds.
- `72/72 tests passed`
- `git diff --check passed`

## Parked

- MIDI input device selector/preference.
- Teach for arbitrary live IO bindings beyond loudness/map CC preferences.
- Persistence of preferences across app launches.
- OSC target preferences.
- External OSC targets and always-on OSC receive nodes/server.
- Realtime callback delivery.
