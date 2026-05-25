# P-LIVE11 MIDI Input Selector Preference Closure

Date: 2026-05-25 14:20 Asia/Taipei.

## Question

Can MIDI teach listen to one selected MIDI input when the user chooses it, while preserving the existing all-input fallback and keeping MIDI callback work realtime-safe?

## Contract

Input:

- `PerformancePreferences.midi.inputIdentifier`
- available MIDI input identifiers from JUCE
- MIDI teach target and incoming MIDI CC messages

Output:

- selected teach input identifiers
- PreferencesPanel MIDI input selection
- MainComponent MIDI teach listener enablement

Rules:

- Empty input selection means listen to all currently available MIDI inputs.
- A selected available input narrows MIDI teach listening to that device only.
- A selected missing input listens to no inputs and reports the existing no-input teach status.
- MIDI input callbacks still do no UI work; callbacks only post parsed CC data to the message thread.
- No arbitrary binding teach, OSC target preference, disk persistence, or realtime callback delivery changes.

## Proven Flow

```text
PerformancePreferences MIDI input selection
-> PreferencesPanel device choice
-> MainComponent::startMidiTeachListening()
-> selected MIDI input enablement
-> MIDI teach callback still learns CC through LiveIOAppController
```

## Evidence

- `source/preferences/PerformancePreferences.h`
- `source/preferences/PerformancePreferences.cpp`
- `source/app/PreferencesPanel.h`
- `source/app/PreferencesPanel.cpp`
- `source/app/MainComponent.cpp`
- `tests/PerformancePreferencesTests.cpp`

## Verification

RED:

- `cmake --build build --target my_world_performance_preferences_tests` failed first because `MidiPreferences` had no input fields and no `midiTeachInputIdentifiers()` policy.

GREEN:

- `cmake --build build --target my_world_performance_preferences_tests && ./build/my_world_performance_preferences_tests`
- `cmake --build build --target my_world_performance_preferences_tests my-world`
- `./build/my_world_performance_preferences_tests`
- `ctest --test-dir build --output-on-failure -R "performance_preferences|live_io_app_controller"`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:

- `performance preferences ok`
- app target `my-world` builds.
- focused `performance_preferences|live_io_app_controller` tests passed 2/2.
- full `ctest` passed 73/73.
- `git diff --check` passed.

## Parked

- Arbitrary binding teach.
- Preference disk persistence.
- OSC target preferences.
- External OSC targets and always-on OSC receive nodes/server.
- Realtime callback delivery.
- Broader MIDI output operators.
