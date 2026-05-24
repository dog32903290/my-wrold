# PV Analyzer Residue Detector Closure

Date: 2026-05-24 19:55 Asia/Taipei

Status: closed for the `residue` detector proof.

## Closure Line

```text
raw RMS + sustain_envelope, optionally cleared by silence_state
-> analyzer.residue
-> residue_state + residue_envelope + residue_timer_ms + confidence
-> focused tests + app debug proof artifacts
```

This slice does not touch Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, TiXL, MIDI mapping, shader uniform mapping, live callback-buffer runtime, browser/UI promotion, aggregate pressure, or flow-runner files.

Later update: aggregate pressure was selected and closed separately in `docs/superpowers/specs/2026-05-24-pv-analyzer-aggregate-pressure-closure.md`.

## Closed Detector

### Residue

```text
fixtures/analyzer/residue_detector_cases.json
-> analyzer.residue runtime detector
-> debug/pv-residue-detector-proof/residue_detector_report.json
```

Contract:

```text
raw RMS below floor after armed sustain_envelope
+ optional silence_state clear
-> residue_state + residue_envelope + residue_timer_ms + confidence
```

Proof says:

```text
ok: true
caseCount: 6
passedCaseCount: 6
usesRawRms: true
usesSustainEnvelopeForDetector: true
usesSilenceStateForClear: true
usesOutputSmoothingForDetector: false
```

The detector does not implement aggregate pressure and does not consume UI-smoothed loudness as detector truth.

## Runtime / Fixture Evidence

New fixture families:

```text
fixtures/analyzer/residue_detector_cases.json
fixtures/module-libraries/pv-residue-detector.module-library.json
fixtures/compounds/residue.compound.json
fixtures/modules/residue/module.json
```

New node docs:

```text
docs/nodes/analyzer.residue.md
```

New app proof command:

```text
--dump-pv-residue-detector-proof-and-exit
```

The proof writes:

```text
residue_detector_report.json
cook_order.json
node_stats.json
errors.json
```

## Verification Run

```text
cmake --build build --target my_world_analyzer_residue_detector_tests my_world_runtime_registry_tests my-world
./build/my_world_analyzer_residue_detector_tests
./build/my_world_runtime_registry_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-residue-detector-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```

Latest accepted result:

```text
analyzer residue detector ok
runtime registry ok
debug/pv-residue-detector-proof/residue_detector_report.json has ok: true
38/38 tests passed
git diff --check passed
```

## Still Parked

```text
output shaping beyond proof fields
browser/UI promotion
MIDI mapping
shader uniform mapping
live callback-buffer runtime changes
multi-node group-to-compound extraction
additional aggregate families such as breathiness, instability, source_legibility, rupture, or memory_distance
```
