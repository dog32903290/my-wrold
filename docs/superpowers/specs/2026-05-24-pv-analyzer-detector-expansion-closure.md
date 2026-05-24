# PV Analyzer Detector Expansion Closure

Date: 2026-05-24 18:28 Asia/Taipei

Status: closed through attack, density, and silence detector proofs.

Later update:

```text
2026-05-24 19:09 Asia/Taipei:
sustain detector proof is also closed in
docs/superpowers/specs/2026-05-24-pv-analyzer-sustain-detector-closure.md
```

## Closure Line

```text
raw-energy facts
-> analyzer.attack onset_event
-> analyzer.density event window
and raw RMS -> analyzer.silence quiet hold
-> focused tests + app debug proof artifacts
```

This slice does not touch Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, TiXL, MIDI mapping, shader uniform mapping, live callback-buffer runtime, or browser/UI promotion.

## Closed Detectors

### Attack

```text
fixtures/analyzer/attack_detector_cases.json
-> analyzer.attack runtime detector
-> debug/pv-attack-detector-proof/attack_detector_report.json
```

Contract:

```text
raw-energy.rms / raw-energy.peak
-> onset_event + attack_value + attack_envelope + confidence
```

Proof says:

```text
ok: true
caseCount: 5
passedCaseCount: 5
usesRawEnergyFacts: true
usesOutputSmoothingForDetector: false
```

### Density

```text
fixtures/analyzer/density_detector_cases.json
-> analyzer.density runtime detector
-> debug/pv-density-detector-proof/density_detector_report.json
```

Contract:

```text
attack.onset_event
-> density_value + event_count + density_envelope + confidence
```

Proof says:

```text
ok: true
caseCount: 5
passedCaseCount: 5
usesOnsetEvents: true
usesAttackEnvelopeForDetector: false
```

### Silence

```text
fixtures/analyzer/silence_detector_cases.json
-> analyzer.silence runtime detector
-> debug/pv-silence-detector-proof/silence_detector_report.json
```

Contract:

```text
raw RMS below floor for holdMs
-> silence_state + silence_timer_ms + confidence
```

Proof says:

```text
ok: true
caseCount: 5
passedCaseCount: 5
usesRawRms: true
attackImplemented: false
densityImplemented: false
```

## Runtime / Fixture Evidence

New fixture families:

```text
fixtures/analyzer/attack_detector_cases.json
fixtures/analyzer/density_detector_cases.json
fixtures/analyzer/silence_detector_cases.json

fixtures/module-libraries/pv-attack-detector.module-library.json
fixtures/module-libraries/pv-density-detector.module-library.json
fixtures/module-libraries/pv-silence-detector.module-library.json
```

New node docs:

```text
docs/nodes/analyzer.attack.md
docs/nodes/analyzer.density.md
docs/nodes/analyzer.silence.md
```

New app proof commands:

```text
--dump-pv-attack-detector-proof-and-exit
--dump-pv-density-detector-proof-and-exit
--dump-pv-silence-detector-proof-and-exit
```

Each proof writes:

```text
*_detector_report.json
cook_order.json
node_stats.json
errors.json
```

## Verification Run

```text
cmake --build build --target my_world_analyzer_detector_tests my_world_analyzer_density_detector_tests my_world_analyzer_silence_detector_tests my_world_runtime_registry_tests my_world_analyzer_compound_family_tests my-world
./build/my_world_analyzer_detector_tests
./build/my_world_analyzer_density_detector_tests
./build/my_world_analyzer_silence_detector_tests
./build/my_world_runtime_registry_tests
./build/my_world_analyzer_compound_family_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-attack-detector-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-density-detector-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-silence-detector-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```

Latest accepted result:

```text
analyzer detector ok
analyzer density detector ok
analyzer silence detector ok
runtime registry ok
analyzer compound family ok
36/36 tests passed
git diff --check passed
```

## Still Parked

```text
residue / aggregate pressure detector semantics
output shaping beyond proof fields
browser/UI promotion
MIDI mapping
shader uniform mapping
live callback-buffer runtime changes
multi-node group-to-compound extraction
```
