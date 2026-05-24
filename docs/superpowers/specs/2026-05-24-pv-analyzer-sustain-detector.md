# PV Analyzer Sustain Detector

Date: 2026-05-24 19:00 Asia/Taipei

Status: closed by `docs/superpowers/specs/2026-05-24-pv-analyzer-sustain-detector-closure.md`.

## Progress Gate Result

PV/analyzer detector expansion is closed through attack, density, and silence. The next selected lane is `sustain`, not residue, aggregate pressure, UI promotion, MIDI mapping, shader uniform mapping, or live callback-buffer runtime.

Do not touch Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, TiXL, relocation note, flow-runner files, `scripts/`, `tests/test_myworld_flow.py`, or `AGENTS.md` in this slice.

## First Line

```text
fixtures/analyzer/sustain_detector_cases.json
-> analyzer.sustain runtime detector
-> debug/pv-sustain-detector-proof/sustain_detector_report.json
```

The proof reads synthetic raw RMS frames. It does not require live audio input, UI meter polish, MIDI mapping, shader uniforms, browser promotion, or raw callback-buffer changes.

## Node Contract

```text
Node: analyzer.sustain
Question: has trusted sound energy stayed active long enough to call it sustained?
Conversion: raw RMS facts -> detector state + held-energy envelope
Family: analyzer / detector-state

Inputs:
- rms: signal.float, required, raw mono RMS from compound.raw-energy or equivalent raw fact source
- peak: signal.float, optional diagnostic, not used to decide sustain in this proof

Outputs:
- sustain_state: signal.float, 1 when raw RMS has stayed above floor for holdMs
- sustain_timer_ms: signal.float, active-hold timer in milliseconds
- sustain_envelope: signal.float, bounded 0..1 state envelope for later visual/MIDI mapping
- confidence: signal.float, 0..1 trust in the detector decision

Parameters:
- floor: analysis calibration, default 0.02, owner RuntimeOp, affects active energy truth, safe_to_change_live yes, saved yes, log_when_changed yes
- holdMs: gesture detector/state policy, default 300 ms, owner RuntimeOp, affects sustain truth, safe_to_change_live yes, saved yes, log_when_changed yes
- releaseMs: state policy, default 200 ms, owner RuntimeOp, affects sustain_envelope release only, safe_to_change_live yes, saved yes, log_when_changed yes
- outputSmoothMs: output shaping, default 0 ms in this proof, owner UI/output mapper later, affects display feel only, safe_to_change_live yes, saved yes, log_when_changed no

State:
- sustainTimerMs
- sustainEnvelope
- lastFrameTimeMs

Failure:
- missing or non-finite rms input: sustain_state 0, sustain_timer_ms preserves previous state, confidence 0, diagnostic `missing_input:rms`
- rms below floor: sustain timer resets to 0; sustain_state becomes 0; sustain_envelope releases by releaseMs

Diagnostics:
- floor, holdMs, releaseMs
- currentRms, activeAboveFloor
- sustainTimerMs
- sustainEnvelope
- confidence
- diagnostic string

Evidence:
- fixture cases in `fixtures/analyzer/sustain_detector_cases.json`
- focused detector test target `my_world_analyzer_sustain_detector_tests`
- app proof dump `debug/pv-sustain-detector-proof/sustain_detector_report.json`

Split / compound decision:
- `analyzer.sustain` is a detector/state RuntimeOp, not a full analyzer family black box.
- `compound.sustain` may wrap raw RMS measurement plus `analyzer.sustain`, but the proof keeps the detector truth raw-RMS based.
- Attack, density, and silence may influence later patch routing, but this proof does not consume their final UI-shaped outputs.
```

## Analyzer Patch Contract

```text
Patch: sustain
Question: has the performance held trusted energy long enough to be treated as sustained?

Vertical pipeline:
raw-energy.rms
  owner: C++ native/runtime proof
  Chinese: C6/PV 已證明的 raw fact，不帶 loudness/gate/smooth 語意。

active_above_floor
  owner: C++ native/runtime proof
  Chinese: raw RMS 是否高過 sustain floor，這是判斷，不是顯示手感。

sustain_timer_ms
  owner: C++ native/runtime proof
  Chinese: active 狀態連續保持多久；低於 floor 立即歸零。

sustain_state
  owner: C++ native/runtime proof
  Chinese: timer 過 holdMs 才成立的 detector truth。

sustain_envelope
  owner: C++ native/runtime proof
  Chinese: 給 later visual/MIDI mapper 的狀態線；releaseMs 只影響這條線，不回頭改 detector truth。

output shaping
  owner: TS/JS/UI or later mapper
  Chinese: 可改顯示手感，但不能寫進 detector evidence。

Parallel influence:
raw-energy.rms -> sustain -> held-energy state
attack.onset_event -> future sustain entry policy -> parked until a separate delayed edge is tested
silence_state -> future sustain reset/gate -> parked until a separate delayed edge is tested

Variables to carve:
floor: analysis calibration
holdMs: gesture detector/state policy
releaseMs: state policy for sustain_envelope only
outputSmoothMs: output shaping only

Failure modes:
- short notes become fake sustain if holdMs is too low.
- real held tones are erased if floor is too high.
- room noise becomes fake sustain if floor is too low.
- sustain becomes unstable if it consumes UI-smoothed loudness or attack envelope as detector truth.
- changing releaseMs must not change sustain_state timing.

Evidence:
- held active fixture: raw RMS above floor reaches sustain after holdMs.
- short active fixture: active energy shorter than holdMs stays not sustained.
- reset fixture: a quiet gap resets sustain_timer_ms.
- below-floor fixture: room noise below floor never sustains.
- failure fixture: missing RMS reports `missing_input:rms`.

Open decision:
- Future "sustain after attack" behavior is parked; this lane only proves raw held-energy sustain.
```

## Fixture Contract

Target fixture:

```text
fixtures/analyzer/sustain_detector_cases.json
```

Minimum cases:

```text
held_active_reaches_sustain:
  raw rms frames: [0.03, 0.05, 0.06, 0.05]
  expected sustainStateLast: true
  expected sustainTimerMsLast: 300

short_active_not_sustain:
  raw rms frames: [0.04, 0.05]
  expected sustainStateLast: false
  expected sustainTimerMsLast: 100

quiet_gap_resets_timer:
  raw rms frames: [0.05, 0.06, 0.00, 0.05]
  expected sustainStateLast: false
  expected sustainTimerMsLast: 100

noise_below_floor_not_sustain:
  raw rms frames: [0.01, 0.015, 0.018, 0.019]
  expected sustainStateLast: false
  expected sustainTimerMsLast: 0

missing_rms_failure:
  raw rms frames: missing
  expected ok: false
  expected diagnostic: missing_input:rms
```

## Runtime / Debug Proof

Target proof artifacts:

```text
debug/pv-sustain-detector-proof/sustain_detector_report.json
debug/pv-sustain-detector-proof/cook_order.json
debug/pv-sustain-detector-proof/node_stats.json
debug/pv-sustain-detector-proof/errors.json
```

Report minimum fields:

```text
ok
operation: pv_sustain_detector
selectedDetector: sustain
fixturePath
caseCount
passedCaseCount
usesRawRms: true
usesAttackOnsetForDetector: false
usesSilenceStateForDetector: false
cases
error
```

Target verification:

```text
cmake --build build --target my_world_analyzer_sustain_detector_tests my_world_runtime_registry_tests my-world
./build/my_world_analyzer_sustain_detector_tests
./build/my_world_runtime_registry_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-sustain-detector-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```
