# PV Analyzer Residue Detector

Date: 2026-05-24 19:45 Asia/Taipei

Status: closed by `docs/superpowers/specs/2026-05-24-pv-analyzer-residue-detector-closure.md`.

## Progress Gate Result

PV/analyzer detector expansion is closed through attack, density, silence, and sustain. The next selected lane is `residue`, not aggregate pressure, UI promotion, MIDI mapping, shader uniform mapping, or live callback-buffer runtime.

Do not touch Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, TiXL, relocation note, flow-runner files, `scripts/`, `tests/test_myworld_flow.py`, or `AGENTS.md` in this slice.

## First Line

```text
fixtures/analyzer/residue_detector_cases.json
-> analyzer.residue runtime detector
-> debug/pv-residue-detector-proof/residue_detector_report.json
```

The proof reads synthetic raw RMS frames plus explicit detector-state inputs. It does not require live audio input, UI meter polish, MIDI mapping, shader uniforms, browser promotion, aggregate pressure, or raw callback-buffer changes.

## Node Contract

```text
Node: analyzer.residue
Question: after trusted held energy falls below the active floor, how much tail remains?
Conversion: raw RMS + sustain state -> residue state/envelope, optionally cleared by silence state
Family: analyzer / detector-state

Inputs:
- rms: signal.float, required, raw mono RMS from compound.raw-energy or equivalent raw fact source
- sustain_envelope: signal.float, required, detector state from analyzer.sustain; not UI-smoothed loudness
- silence_state: signal.float/bool, optional, detector state from analyzer.silence; when true it clears residue

Outputs:
- residue_state: signal.float, 1 while a previously armed sustain tail remains above residueThreshold
- residue_envelope: signal.float, bounded 0..1 tail envelope for later aggregate/visual/MIDI mapping
- residue_timer_ms: signal.float, time since the armed sound fell below the active floor
- confidence: signal.float, 0..1 trust in the residue decision

Parameters:
- floor: analysis calibration, default 0.02, owner RuntimeOp, affects active vs tail truth, safe_to_change_live yes, saved yes, log_when_changed yes
- armThreshold: gesture detector/state policy, default 0.6, owner RuntimeOp, affects whether sustain_envelope arms residue, safe_to_change_live yes, saved yes, log_when_changed yes
- residueThreshold: gesture detector/state policy, default 0.1, owner RuntimeOp, affects residue_state truth, safe_to_change_live yes, saved yes, log_when_changed yes
- decayMs: state policy, default 200 ms in this proof, owner RuntimeOp, affects residue_envelope release, safe_to_change_live yes, saved yes, log_when_changed yes
- silenceClears: state policy, default true, owner RuntimeOp, lets silence_state clear tail memory, safe_to_change_live yes, saved yes, log_when_changed yes
- outputSmoothMs: output shaping, default 0 ms in this proof, owner UI/output mapper later, affects display feel only, safe_to_change_live yes, saved yes, log_when_changed no

State:
- armedEnvelope
- residueEnvelope
- residueTimerMs
- lastFrameTimeMs

Failure:
- missing or non-finite rms input: residue_state 0, confidence 0, diagnostic `missing_input:rms`
- missing or non-finite sustain_envelope input: residue_state 0, confidence 0, diagnostic `missing_input:sustain_envelope`
- missing silence_state is allowed; the detector runs without the explicit silence clear

Diagnostics:
- floor, armThreshold, residueThreshold, decayMs
- currentRms, sustainEnvelope, silenceState
- armedEnvelope, residueTimerMs, residueEnvelope
- confidence
- diagnostic string

Evidence:
- fixture cases in `fixtures/analyzer/residue_detector_cases.json`
- focused detector test target `my_world_analyzer_residue_detector_tests`
- app proof dump `debug/pv-residue-detector-proof/residue_detector_report.json`

Split / compound decision:
- `analyzer.residue` is a detector/state RuntimeOp, not aggregate pressure.
- It may consume `sustain_envelope` and optional `silence_state` because those are named detector-state signals, not UI meters.
- Aggregate pressure remains parked until residue has its own proof.
```

## Analyzer Patch Contract

```text
Patch: residue
Question: what tail remains after a held sound stops being active?

Vertical pipeline:
raw-energy.rms
  owner: C++ native/runtime proof
  Chinese: 判斷 active vs tail 的 raw fact，不用 loudness/smooth。

sustain_envelope
  owner: C++ native/runtime proof
  Chinese: 只用來 arm residue；短促 attack 沒有 sustain，就不產生殘留。

silence_state
  owner: C++ native/runtime proof
  Chinese: 穩定 silence 可以清掉尾巴；缺這條線時 detector 仍可跑，但不做 silence clear。

tail transition
  owner: C++ native/runtime proof
  Chinese: raw RMS 從 active 掉到 floor 以下，且前面 sustain_envelope 足夠，才進 residue。

residue_envelope
  owner: C++ native/runtime proof
  Chinese: tail 的可映射狀態線，之後 aggregate pressure 可以吃它。

output shaping
  owner: TS/JS/UI or later mapper
  Chinese: 可改顯示手感，但不能回頭改 residue truth。

Parallel influence:
sustain_envelope -> residue -> tail/memory state after active sound
silence_state -> residue -> clear stable no-input tail
residue_envelope -> future aggregate pressure -> parked until aggregate gets its own spec

Variables to carve:
floor: analysis calibration
armThreshold: detector state policy
residueThreshold: detector truth
decayMs: state policy
silenceClears: state policy
outputSmoothMs: output shaping only

Failure modes:
- room noise becomes residue if floor is too low.
- short clicks become residue if armThreshold is too low.
- real tail disappears too fast if decayMs is too short.
- aggregate pressure becomes arbitrary if residue is skipped and folded directly into weights.
- residue lies if it consumes UI-smoothed loudness instead of raw RMS + detector-state signals.

Evidence:
- sustained drop fixture: held sound falls below floor and creates residue.
- decay fixture: residue envelope eventually falls below residueThreshold.
- silence clear fixture: silence_state clears the residue tail.
- short unsustained fixture: attack-like energy with low sustain_envelope creates no residue.
- failure fixtures: missing RMS and missing sustain_envelope diagnostics.

Open decision:
- Aggregate pressure weighting is explicitly parked; residue only publishes a tail state.
```

## Fixture Contract

Target fixture:

```text
fixtures/analyzer/residue_detector_cases.json
```

Minimum cases:

```text
sustained_drop_creates_residue:
  rms frames: active, active, below floor
  sustain_envelope frames: armed above threshold before drop
  expected residueStateLast: true
  expected residueTimerMsLast: 100

residue_decays_below_threshold:
  rms frames: active, repeated below floor
  sustain_envelope frames: armed before drop
  expected residueStateLast: false after decay

silence_state_clears_residue:
  rms frames: active, below floor, below floor
  silence_state last frame: true
  expected residueEnvelopeLast: 0

short_unsustained_drop_no_residue:
  sustain_envelope below armThreshold
  expected residueStateLast: false

missing_rms_failure:
  expected diagnostic: missing_input:rms

missing_sustain_envelope_failure:
  expected diagnostic: missing_input:sustain_envelope
```

## Runtime / Debug Proof

Target proof artifacts:

```text
debug/pv-residue-detector-proof/residue_detector_report.json
debug/pv-residue-detector-proof/cook_order.json
debug/pv-residue-detector-proof/node_stats.json
debug/pv-residue-detector-proof/errors.json
```

Report minimum fields:

```text
ok
operation: pv_residue_detector
selectedDetector: residue
fixturePath
caseCount
passedCaseCount
usesRawRms: true
usesSustainEnvelopeForDetector: true
usesSilenceStateForClear: true
usesOutputSmoothingForDetector: false
cases
error
```

Target verification:

```text
cmake --build build --target my_world_analyzer_residue_detector_tests my_world_runtime_registry_tests my-world
./build/my_world_analyzer_residue_detector_tests
./build/my_world_runtime_registry_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-residue-detector-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```
