# PV Analyzer Aggregate Pressure Closure

Date: 2026-05-24 21:05 Asia/Taipei

Status: closed for the `aggregate_pressure` proof.

## Closure Line

```text
raw-energy.rms
+ attack_value
+ density_value
+ sustain_envelope
+ residue_envelope
+ silence_state
-> analyzer.aggregate_pressure
-> pressure_value + weighted component debug outputs + confidence
-> focused tests + app debug proof artifacts
```

This slice does not touch Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, TiXL, browser/UI promotion, MIDI mapping, shader uniform mapping, live callback-buffer runtime, flow-runner files, `scripts/`, `tests/test_myworld_flow.py`, relocation note, or `AGENTS.md`.

## Closed Aggregate

```text
fixtures/analyzer/aggregate_pressure_cases.json
-> analyzer.aggregate_pressure runtime aggregate
-> debug/pv-aggregate-pressure-proof/aggregate_pressure_report.json
```

Contract:

```text
raw-energy.rms + attack_value + density_value + sustain_envelope + residue_envelope + silence_state
-> analyzer.aggregate_pressure
-> pressure_value + energy_component + attack_component + density_component + sustain_component + residue_component + confidence
```

Proof says:

```text
ok: true
caseCount: 7
passedCaseCount: 7
usesRawRms: true
usesDetectorStates: true
usesOutputSmoothingForAggregate: false
```

The aggregate is not a detector and does not implement output shaping. It exposes weighted components so `pressure_value` is inspectable rather than a hidden score.

## Runtime / Fixture Evidence

New fixture families:

```text
fixtures/analyzer/aggregate_pressure_cases.json
fixtures/module-libraries/pv-aggregate-pressure.module-library.json
fixtures/compounds/aggregate-pressure.compound.json
fixtures/modules/aggregate-pressure/module.json
```

New node docs:

```text
docs/nodes/analyzer.aggregate-pressure.md
```

New app proof command:

```text
--dump-pv-aggregate-pressure-proof-and-exit
```

The proof writes:

```text
aggregate_pressure_report.json
cook_order.json
node_stats.json
errors.json
```

Observed proof artifacts:

```text
debug/pv-aggregate-pressure-proof/aggregate_pressure_report.json has ok: true
debug/pv-aggregate-pressure-proof/cook_order.json orders raw facts, detector states, aggregate, aggregate out
debug/pv-aggregate-pressure-proof/node_stats.json records 7/7 passing cases
debug/pv-aggregate-pressure-proof/errors.json has ok: true and empty errors
```

## Verification Run

```text
cmake --build build --target my_world_analyzer_aggregate_pressure_tests my_world_runtime_registry_tests my-world
./build/my_world_analyzer_aggregate_pressure_tests
./build/my_world_runtime_registry_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-aggregate-pressure-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```

Latest accepted result:

```text
analyzer aggregate pressure ok
runtime registry ok
debug/pv-aggregate-pressure-proof/aggregate_pressure_report.json has ok: true
39/39 tests passed
git diff --check passed
```

## Still Parked

```text
browser/UI promotion
MIDI mapping
shader uniform mapping
live callback-buffer runtime changes
multi-node group-to-compound extraction
additional aggregate families such as breathiness, instability, source_legibility, rupture, or memory_distance
```
