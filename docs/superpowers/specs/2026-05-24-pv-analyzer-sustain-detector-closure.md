# PV Analyzer Sustain Detector Closure

Date: 2026-05-24 19:09 Asia/Taipei

Status: closed for the `sustain` detector proof.

Later update:

```text
2026-05-24 19:55 Asia/Taipei:
residue detector proof is also closed in
docs/superpowers/specs/2026-05-24-pv-analyzer-residue-detector-closure.md
```

## Closure Line

```text
raw RMS held above floor for holdMs
-> analyzer.sustain
-> sustain_state + sustain_timer_ms + sustain_envelope + confidence
-> focused tests + app debug proof artifacts
```

This slice does not touch Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, TiXL, MIDI mapping, shader uniform mapping, live callback-buffer runtime, browser/UI promotion, or flow-runner files.

## Closed Detector

### Sustain

```text
fixtures/analyzer/sustain_detector_cases.json
-> analyzer.sustain runtime detector
-> debug/pv-sustain-detector-proof/sustain_detector_report.json
```

Contract:

```text
raw RMS above floor for holdMs
-> sustain_state + sustain_timer_ms + sustain_envelope + confidence
```

Proof says:

```text
ok: true
caseCount: 5
passedCaseCount: 5
usesRawRms: true
usesAttackOnsetForDetector: false
usesSilenceStateForDetector: false
```

The detector does not consume UI-smoothed loudness, `attack_envelope`, `density_value`, or `silence_state` as detector truth.

## Runtime / Fixture Evidence

New fixture families:

```text
fixtures/analyzer/sustain_detector_cases.json
fixtures/module-libraries/pv-sustain-detector.module-library.json
fixtures/compounds/sustain.compound.json
fixtures/modules/sustain/module.json
```

New node docs:

```text
docs/nodes/analyzer.sustain.md
```

New app proof command:

```text
--dump-pv-sustain-detector-proof-and-exit
```

The proof writes:

```text
sustain_detector_report.json
cook_order.json
node_stats.json
errors.json
```

## Verification Run

```text
cmake --build build --target my_world_analyzer_sustain_detector_tests my_world_runtime_registry_tests my-world
./build/my_world_analyzer_sustain_detector_tests
./build/my_world_runtime_registry_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-sustain-detector-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```

Latest accepted result:

```text
analyzer sustain detector ok
runtime registry ok
debug/pv-sustain-detector-proof/sustain_detector_report.json has ok: true
37/37 tests passed
git diff --check passed
```

## Still Parked

```text
aggregate pressure detector semantics
output shaping beyond proof fields
browser/UI promotion
MIDI mapping
shader uniform mapping
live callback-buffer runtime changes
multi-node group-to-compound extraction
```
