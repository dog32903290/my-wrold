# PV Analyzer Attack Detector

Date: 2026-05-24 17:53 Asia/Taipei

Status: active minimal spec. No implementation code has been written for this lane yet.

## Progress Gate Result

R runtime/render backbone is closed through R3. C6 already proved the analyzer family can load `compound.raw-energy` without turning analyzer work into one black box.

PV/analyzer detector expansion starts with `attack`, not `density` or `silence`.

Do not touch Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, or raw callback-buffer runtime in this slice.

## First Line

```text
fixtures/analyzer/attack_detector_cases.json
-> analyzer.attack runtime detector
-> debug/pv-attack-detector-proof/attack_detector_report.json
```

The first proof reads synthetic raw-energy analysis frames. It does not require live audio input, UI meter polish, MIDI mapping, shader uniforms, or browser promotion.

## Why Attack First

`attack` is the first detector because it turns raw energy into an event.

```text
raw-energy.rms / raw-energy.peak
-> analyzer.attack
-> onset_event + attack_value
```

`density` must consume `onset_event` later. It must not infer density from a UI-smoothed attack meter. `silence` stays parked until the attack detector has a clear relationship to floor/no-input behavior.

## Node Contract

```text
Node: analyzer.attack
Question: did a new trusted sound entry happen now, and how hard was it?
Conversion: raw energy facts -> detector event + attack state
Family: analyzer / detector-state

Inputs:
- rms: signal.float, required, raw mono RMS from compound.raw-energy or equivalent raw fact source
- peak: signal.float, optional, raw mono peak for clipping/uncertainty diagnostics
- sampleCount: signal.float, optional diagnostic, analysis window length for the source frame

Outputs:
- onset_event: event.trigger, one-frame event when a trusted rising edge passes detector rules
- attack_value: signal.float, bounded 0..1 rising strength for this onset or current attack frame
- attack_envelope: signal.float, detector state that decays after an onset; not the source for density
- confidence: signal.float, 0..1 trust in the detector decision

Parameters:
- floor: analysis calibration, default 0.02, owner RuntimeOp, affects trusted input floor, safe_to_change_live yes, saved yes, log_when_changed yes
- riseThreshold: gesture detector, default 0.08, owner RuntimeOp, affects onset truth, safe_to_change_live yes, saved yes, log_when_changed yes
- debounceMs: gesture detector/state policy, default 80 ms, owner RuntimeOp, affects retrigger truth, safe_to_change_live yes, saved yes, log_when_changed yes
- releaseMs: state policy, default 120 ms, owner RuntimeOp, affects attack_envelope decay only, safe_to_change_live yes, saved yes, log_when_changed yes
- outputSmoothMs: output shaping, default 0 ms in this proof, owner UI/output mapper later, affects display feel only, safe_to_change_live yes, saved yes, log_when_changed no

State:
- previousRms
- lastOnsetFrame or lastOnsetTimeMs
- attackEnvelope

Failure:
- missing rms input: no onset_event, attack_value 0, confidence 0, visible/runtime diagnostic `missing_input:rms`
- sampleCount <= 0 when provided: no onset_event, confidence 0, diagnostic `invalid_sample_count`
- peak > 1.0: detector may still run, but diagnostics warn `clipped_peak` and confidence is capped
- debounce window active: no second onset_event; attack_envelope may continue/release

Diagnostics:
- threshold used, floor used, debounceMs, releaseMs
- currentRms, previousRms, delta, gatedDelta
- onsetEventCount
- lastOnsetFrame
- confidence
- warning list

Evidence:
- fixture cases in `fixtures/analyzer/attack_detector_cases.json`
- focused detector test target, proposed name `my_world_analyzer_detector_tests`
- app proof dump `debug/pv-attack-detector-proof/attack_detector_report.json`

Split / compound decision:
- `analyzer.attack` is a detector/state RuntimeOp, not a full analyzer family black box.
- A later `compound.attack` may wrap raw-energy children plus `analyzer.attack`, but this proof first locks the detector contract and event output.
```

## Analyzer Patch Contract

```text
Patch: attack
Question: did a new breath/entry happen, and how strong was the entry?

Vertical pipeline:
raw-energy.rms / raw-energy.peak
  owner: C++ native/runtime proof
  Chinese: C6 已證明的 raw fact，不帶 loudness/gate/smooth 語意。

rising_delta
  owner: C++ native/runtime proof
  Chinese: current rms 與 previous rms / floor 的差，不是 UI meter。

detector_gate
  owner: C++ native/runtime proof
  Chinese: floor + riseThreshold + debounce 判斷是否成為新入口事件。

onset_event
  owner: C++ native/runtime proof
  Chinese: 給 density 吃的事件線；只能是一幀/一次事件，不是平滑值。

attack_envelope
  owner: C++ native/runtime proof
  Chinese: 給可視化或 later mapper 的狀態線，不能回頭改 detector truth。

output shaping
  owner: TS/JS/UI or later mapper
  Chinese: 可改顯示手感，但不能餵 density，也不能寫進 raw detector evidence。

Parallel influence:
onset_event -> future density -> count trusted entries in a time window
attack_value -> future visual/MIDI mapping -> drive response strength without becoming detector truth
silence_state -> future attack gate -> parked until silence has its own delayed/tested contract

Variables to carve:
floor: analysis calibration
riseThreshold: gesture detector
debounceMs: gesture detector/state policy
releaseMs: state policy
outputSmoothMs: output shaping only

Failure modes:
- soft breath is erased if floor/riseThreshold are too high.
- steady loud sound retriggers if debounce and delta rules are too weak.
- room noise wiggle becomes fake attack if floor is too low.
- density becomes unstable if it consumes attack_envelope instead of onset_event.
- sample-rate or buffer-size drift lies if debounce/window units are not explicit.

Evidence:
- quiet/no-input fixture: no onset, confidence 0.
- clean intended gesture fixture: exactly one onset_event, positive attack_value.
- steady-loud fixture: no retrigger after initial state.
- edge/noise fixture: wiggle below threshold produces no onset.
- one failure/warning path: missing rms or invalid sampleCount.

Open decision:
- Exact numeric defaults may be adjusted only by changing the fixture expectations and recording the reason in this spec.
```

## Fixture Contract

Proposed future fixture:

```text
fixtures/analyzer/attack_detector_cases.json
```

Minimum cases:

```text
quiet_no_input:
  raw rms frames: [0.0, 0.0, 0.0, 0.0]
  expected onsetEventCount: 0
  expected confidenceLast: 0

single_clean_entry:
  raw rms frames: [0.01, 0.02, 0.18, 0.22, 0.20]
  expected onsetEventCount: 1
  expected firstOnsetFrame: 2
  expected attackValueMin: 0.10

steady_loud_no_retrigger:
  raw rms frames: [0.20, 0.21, 0.205, 0.20]
  expected onsetEventCount: 0
  expected reason: delta below riseThreshold or debounce policy

micro_wiggle_below_threshold:
  raw rms frames: [0.015, 0.025, 0.018, 0.028]
  expected onsetEventCount: 0
  expected reason: below floor/riseThreshold

missing_rms_failure:
  raw rms frames: missing
  expected ok: false
  expected diagnostic: missing_input:rms
```

The fixture owns detector truth. UI smoothing, MIDI ranges, shader uniform mapping, and density windows must not be hidden inside it.

## Runtime / Debug Proof

Proposed proof artifacts:

```text
debug/pv-attack-detector-proof/attack_detector_report.json
debug/pv-attack-detector-proof/cook_order.json
debug/pv-attack-detector-proof/node_stats.json
debug/pv-attack-detector-proof/errors.json
```

Report minimum fields:

```text
ok
operation: pv_attack_detector
selectedDetector: attack
fixturePath
caseCount
passedCaseCount
onsetEventCounts
firstOnsetFrames
diagnostics
warnings
usesRawEnergyFacts: true
usesOutputSmoothingForDetector: false
densityImplemented: false
silenceImplemented: false
```

Target verification when implementation begins:

```text
cmake --build build --target my_world_analyzer_detector_tests my-world
./build/my_world_analyzer_detector_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-attack-detector-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```
