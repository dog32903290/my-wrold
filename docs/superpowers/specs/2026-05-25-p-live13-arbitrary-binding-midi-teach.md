# P-LIVE13 Arbitrary Binding MIDI Teach Closure

Date: 2026-05-25 14:44 Asia/Taipei.

## Question

Can MIDI teach learn a CC for an arbitrary `midi.cc` Live IO binding without disturbing fixed loudness/map CC teach behavior or touching OSC/realtime lanes?

## Contract

Input:

- `LiveIOBinding` list
- target binding id
- incoming MIDI CC message

Output:

- learned MIDI channel/CC
- updated target `midi.cc` binding

Rules:

- Binding teach requires a non-empty binding id.
- Only the matching `midi.cc` binding is updated.
- Non-target MIDI bindings and non-MIDI bindings are unchanged.
- Fixed `loudness_cc` and `map_cc` teach behavior stays compatible.
- No UI binding browser, OSC target preference, external OSC send/receive, or realtime callback delivery changes.

## Proven Flow

```text
LiveIOBinding list
-> armLiveIOMidiTeachForBinding("midi.attack")
-> incoming MIDI CC
-> applyLiveIOMidiTeachToBindings()
-> only "midi.attack" channel/CC updates
```

## Evidence

- `source/core/LiveIOMidiTeach.h`
- `source/core/LiveIOMidiTeach.cpp`
- `tests/LiveIOMidiTeachTests.cpp`
- `docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md`

## Verification

RED:

- `cmake --build build --target my_world_live_io_midi_teach_tests` failed first because `armLiveIOMidiTeachForBinding()`, `LiveIOMidiTeachResult::bindingId`, and `applyLiveIOMidiTeachToBindings()` did not exist.

GREEN:

- `cmake --build build --target my_world_live_io_midi_teach_tests`
- `./build/my_world_live_io_midi_teach_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "live_io_midi_teach|live_io_app_controller|performance_preferences"`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:

- `live io midi teach ok`
- app target `my-world` builds.
- focused `live_io_midi_teach|live_io_app_controller|performance_preferences` tests passed 3/3.
- full `ctest` passed 73/73.
- `git diff --check` passed.

## Parked

- UI surface for choosing arbitrary binding ids.
- OSC target preferences.
- External OSC targets and always-on OSC receive nodes/server.
- Realtime callback delivery.
- Broader MIDI output operators.
