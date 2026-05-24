# PV Analyzer Aggregate Pressure

Date: 2026-05-24 20:54 Asia/Taipei

Status: closed by `docs/superpowers/specs/2026-05-24-pv-analyzer-aggregate-pressure-closure.md`.

Closure timestamp: 2026-05-24 21:05 Asia/Taipei.

## Progress Gate Result

PV analyzer work was closed through attack, density, silence, sustain, and residue when this lane opened. This spec selected `aggregate pressure`, not UI promotion, MIDI mapping, shader uniform mapping, browser promotion, or live callback-buffer runtime.

Do not touch Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, TiXL, relocation note, flow-runner files, `scripts/`, `tests/test_myworld_flow.py`, or `AGENTS.md` in this slice.

## First Line

```text
fixtures/analyzer/aggregate_pressure_cases.json
-> analyzer.aggregate_pressure runtime aggregate
-> debug/pv-aggregate-pressure-proof/aggregate_pressure_report.json
```

The proof reads synthetic raw RMS plus detector-state frames. It does not require live audio input, UI meter polish, MIDI mapping, shader uniforms, browser promotion, or raw callback-buffer changes.

## Node Contract

```text
Node: analyzer.aggregate_pressure
Question: how much current performance pressure exists after combining trusted raw and detector-state lanes?
Conversion: raw RMS + detector states -> bounded aggregate value + weighted component debug outputs
Family: analyzer / aggregate

Inputs:
- rms: signal.float, required, raw mono RMS from compound.raw-energy or equivalent raw fact source
- attack_value: signal.float, required, detector output from analyzer.attack
- density_value: signal.float, required, aggregate-ready detector output from analyzer.density
- sustain_envelope: signal.float, required, detector state from analyzer.sustain
- residue_envelope: signal.float, required, detector state from analyzer.residue
- silence_state: signal.float/bool, required, detector state from analyzer.silence; when true it clears final pressure

Outputs:
- pressure_value: signal.float, bounded 0..1 aggregate pressure
- energy_component: signal.float, weighted raw RMS contribution
- attack_component: signal.float, weighted attack contribution
- density_component: signal.float, weighted density contribution
- sustain_component: signal.float, weighted sustain contribution
- residue_component: signal.float, weighted residue contribution
- confidence: signal.float, 0..1 trust in the aggregate value

Parameters:
- energyScale: analysis calibration, default 4.0, owner RuntimeOp, maps raw RMS to 0..1 energy component input, safe_to_change_live yes, saved yes, log_when_changed yes
- energyWeight: aggregate policy, default 0.25, owner RuntimeOp, affects pressure_value, safe_to_change_live yes, saved yes, log_when_changed yes
- attackWeight: aggregate policy, default 0.25, owner RuntimeOp, affects pressure_value, safe_to_change_live yes, saved yes, log_when_changed yes
- densityWeight: aggregate policy, default 0.20, owner RuntimeOp, affects pressure_value, safe_to_change_live yes, saved yes, log_when_changed yes
- sustainWeight: aggregate policy, default 0.20, owner RuntimeOp, affects pressure_value, safe_to_change_live yes, saved yes, log_when_changed yes
- residueWeight: aggregate policy, default 0.10, owner RuntimeOp, affects pressure_value, safe_to_change_live yes, saved yes, log_when_changed yes
- silenceClears: aggregate policy, default true, owner RuntimeOp, clears final pressure and components when silence_state is true, safe_to_change_live yes, saved yes, log_when_changed yes
- outputSmoothMs: output shaping, default 0 ms in this proof, owner UI/output mapper later, affects display feel only, safe_to_change_live yes, saved yes, log_when_changed no

State:
- none for this proof. Aggregate pressure is a frame-local weighted aggregate.

Failure:
- missing or non-finite required input: pressure_value 0, confidence 0, diagnostic `missing_input:<port>`
- negative or over-range detector values are clamped before weighting, with no state mutation
- silence_state true: pressure_value and components are 0, confidence stays 1

Diagnostics:
- all weights and energyScale
- clamped source values
- weighted component values
- pressure_value
- confidence
- diagnostic string

Evidence:
- fixture cases in `fixtures/analyzer/aggregate_pressure_cases.json`
- focused aggregate test target `my_world_analyzer_aggregate_pressure_tests`
- app proof dump `debug/pv-aggregate-pressure-proof/aggregate_pressure_report.json`

Split / compound decision:
- `analyzer.aggregate_pressure` is an aggregate, not a detector.
- It may consume detector-state outputs, but must expose weighted component debug outputs so the mix is not a magic score.
- UI/MIDI/shader output shaping remains parked.
```

## Analyzer Patch Contract

```text
Patch: aggregate pressure
Question: how much current pressure exists after raw energy and detector-state lines are combined?

Vertical pipeline:
raw-energy.rms
  owner: C++ native/runtime proof
  Chinese: raw energy base; scaled by energyScale before weight.

attack_value
  owner: C++ native/runtime proof
  Chinese: entry force; event output is not counted directly here.

density_value
  owner: C++ native/runtime proof
  Chinese: recent event crowding; already a bounded detector value.

sustain_envelope
  owner: C++ native/runtime proof
  Chinese: held pressure state.

residue_envelope
  owner: C++ native/runtime proof
  Chinese: tail pressure state; residue is allowed because it now has its own proof.

silence_state
  owner: C++ native/runtime proof
  Chinese: stable silence clears pressure so old tail does not leak forward.

weighted components
  owner: C++ native/runtime proof
  Chinese: every input contribution is visible and testable.

pressure_value
  owner: C++ native/runtime proof
  Chinese: bounded 0..1 aggregate. Not MIDI/shader output shaping.

Parallel influence:
attack_value -> aggregate_pressure -> current entry pressure
density_value -> aggregate_pressure -> recent crowding pressure
sustain_envelope -> aggregate_pressure -> held pressure
residue_envelope -> aggregate_pressure -> tail pressure
silence_state -> aggregate_pressure -> clear stable no-input pressure

Variables to carve:
energyScale: analysis calibration
energyWeight / attackWeight / densityWeight / sustainWeight / residueWeight: aggregate policy
silenceClears: aggregate policy
outputSmoothMs: output shaping only

Failure modes:
- pressure becomes arbitrary if component weights are hidden.
- quiet room noise becomes pressure if energyScale is too high.
- old tail leaks into silence if silence_state cannot clear.
- UI/MIDI/shader mapping becomes tangled if output ranges are implemented in this node.
- negative or over-range upstream values must clamp rather than distort the aggregate.

Evidence:
- high pressure fixture with all components active.
- attack spike fixture with attack-dominant pressure.
- residue tail fixture with small tail pressure.
- silence clear fixture with pressure zero.
- clamp fixture for over-range detector values.
- missing required input fixtures.

Open decision:
- Future output shaping for MIDI/shader gets a separate lane; aggregate pressure only publishes bounded value and components.
```

## Fixture Contract

Target fixture:

```text
fixtures/analyzer/aggregate_pressure_cases.json
```

Minimum cases:

```text
high_pressure_all_components:
  rms: 0.20, attack_value: 0.90, density_value: 0.50, sustain_envelope: 0.80, residue_envelope: 0.20, silence_state: false
  expected pressureValueLast: 0.705

attack_spike_pressure:
  rms: 0.03, attack_value: 1.00, density_value: 0.25, sustain_envelope: 0.10, residue_envelope: 0.00, silence_state: false
  expected pressureValueLast: 0.35

residue_tail_pressure:
  rms: 0.01, attack_value: 0.00, density_value: 0.00, sustain_envelope: 0.20, residue_envelope: 0.60, silence_state: false
  expected pressureValueLast: 0.11

silence_clears_pressure:
  same strong components, silence_state: true
  expected pressureValueLast: 0

overrange_inputs_clamp:
  rms: 1.0, attack_value: 1.4, density_value: 2.0, sustain_envelope: 1.2, residue_envelope: 0.5
  expected pressureValueLast: 0.95

missing_rms_failure:
  expected diagnostic: missing_input:rms

missing_attack_value_failure:
  expected diagnostic: missing_input:attack_value
```

## Runtime / Debug Proof

Target proof artifacts:

```text
debug/pv-aggregate-pressure-proof/aggregate_pressure_report.json
debug/pv-aggregate-pressure-proof/cook_order.json
debug/pv-aggregate-pressure-proof/node_stats.json
debug/pv-aggregate-pressure-proof/errors.json
```

Report minimum fields:

```text
ok
operation: pv_aggregate_pressure
selectedAggregate: aggregate_pressure
fixturePath
caseCount
passedCaseCount
usesRawRms: true
usesDetectorStates: true
usesOutputSmoothingForAggregate: false
cases
error
```

Target verification:

```text
cmake --build build --target my_world_analyzer_aggregate_pressure_tests my_world_runtime_registry_tests my-world
./build/my_world_analyzer_aggregate_pressure_tests
./build/my_world_runtime_registry_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-aggregate-pressure-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```
