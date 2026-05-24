# C6 Analyzer Compound Family and Repair Loop

Date: 2026-05-24 12:48 Asia/Taipei

## Progress Gate Result

```text
C5 is closed and pushed:
- 73ce3f0 C5.1 module publish proof
- 9b48941 C5.2 AI worker publish_module
- 0754301 C5.3 visible Publish Module hand
```

C6 starts a new lane. It does not reopen C5 publishing, C4 AI commands, or C1 loudness closure.

## C6.1 Target

```text
analyzer family module library fixture
-> add one new analyzer compound package: compound.raw-energy
-> load through loadCompoundModuleNodeSpecsFromLibrary()
-> load through loadRuntimeRegistryFromModuleLibrary()
-> RuntimeOp coverage says raw-energy is create-enabled
-> synthetic runtime execution publishes rms / peak / sampleCount public outputs
-> create compound.raw-energy through InteractionContract
-> app proof dump writes debug/c6-analyzer-family-proof/analyzer_family_report.json
```

This first slice proves the analyzer compound family can contain more than the existing loudness compound without turning the analyzer into a black box.

## C6.1 Closed Slice

```text
fixtures/module-libraries/analyzer-family.module-library.json
-> module.loudness + module.raw-energy
-> compound.raw-energy visible NodeSpec
-> RuntimeRegistry entry for compound.raw-energy
-> synthetic.analyzer.raw_energy_out RuntimeOp coverage
-> synthetic stereo execution publishes rms / peak / sampleCount
-> createNode(visibleRegistry, compound.raw-energy)
-> debug/c6-analyzer-family-proof/analyzer_family_report.json
```

## C6.1 Evidence

- `fixtures/compounds/raw-energy.compound.json` defines the raw-energy compound as `AudioIn -> MonoMix -> RMS -> RawEnergyOut`.
- `fixtures/modules/raw-energy/module.json` packages `compound.raw-energy` with public `audio.in`, `rms`, `peak`, and `sampleCount` ports.
- `fixtures/module-libraries/analyzer-family.module-library.json` loads both `module.loudness` and `module.raw-energy`.
- `source/core/RuntimeRegistry.cpp` registers `synthetic.analyzer.raw_energy_out` and publishes raw fact outputs without detector shaping.
- `tests/AnalyzerCompoundFamilyTests.cpp` proves visible registry load, runtime registry load, RuntimeOp diagnostics, synthetic execution, and `InteractionContract::createNode()`.
- `--dump-c6-analyzer-family-proof-and-exit` writes `debug/c6-analyzer-family-proof/analyzer_family_report.json`.

## C6.1 Node Contract

```text
Node: compound.raw-energy
Question: what raw RMS / peak / sample-count facts exist after mono mix, before detector or output shaping?
Conversion: audio.channels -> raw fact signal public ports
Family: analyzer / raw facts
Inputs:
- audio.in: audio.channels
Outputs:
- rms: signal.float, measured mono RMS
- peak: signal.float, measured mono peak
- sampleCount: signal.float, analyzed window length for this proof runtime
Parameters: none in C6.1
State: none in C6.1
Failure:
- missing audio input leaves runtime child blocked and no public raw-energy outputs
- missing RuntimeOp blocks creation through runtime diagnostics
Diagnostics:
- runtime-op-ready / create-enabled when all child RuntimeOps are registered
- runtime execution records child input sources and public output sources
Evidence:
- tests/AnalyzerCompoundFamilyTests.cpp
- debug/c6-analyzer-family-proof/analyzer_family_report.json
Split / compound decision:
- raw-energy is a compound because it exposes useful raw analyzer facts while keeping AudioIn, MonoMix, RMS, and output publication visible as children.
- It is not loudness, attack, density, gate, or smoothing.
```

## C6.1 Analyzer Patch Contract

```text
Patch: raw-energy
Question: what trusted raw energy facts did the current synthetic audio window measure?

Vertical pipeline:
audio.input          owner: runtime proof, accepts channels
audio.mono_mix       owner: runtime proof, folds channels into mono and sample count
analyzer.rms         owner: runtime proof, measures rms / peak raw facts
analyzer.raw_energy_out owner: runtime proof, publishes raw fact ports without detector shaping

Parallel influence:
raw-energy.rms -> future loudness / attack detectors -> parks shared intermediate signal vocabulary
raw-energy.peak -> future clipping / rupture detector -> parked
raw-energy.sampleCount -> future window diagnostics -> parked

Variables to carve:
none in C6.1. Threshold, sensitivity, window, debounce, smooth, outMin/outMax stay parked.

Failure modes:
- raw facts can be mistaken for semantic loudness; C6.1 labels them raw facts only.
- sampleCount is a proof diagnostic, not a musical time/window contract yet.
- same-frame detector feedback is not introduced.

Evidence:
- family library loads both compound.loudness and compound.raw-energy.
- runtime coverage has zero missing child RuntimeOps for raw-energy.
- synthetic stereo fixture publishes rms=sqrt(0.28125), peak=0.75, sampleCount=4.
```

## Moved From C6.1 Parking Into C6.2

```text
AI repair loop / retry policy
```

## Still Parked Outside C6

```text
attack / density / sustain / silence compound semantics
default visible browser promotion beyond the proof fixture
live callback-buffer changes
analysis calibration controls
threshold / hysteresis / debounce / smoothing variables
MIDI mapping and shader uniform mapping for raw-energy
publish/reuse UX changes
remote sync / shared module registry
```

## C6.2 Target

```text
AIWorkerRepairPlan
-> bounded repair attempts
-> each attempt calls executeAIWorkerCommand()
-> failed command attempt logs failure evidence
-> next attempt can repair the command payload
-> first successful attempt stops the loop
-> GraphSession commandLog + collaborationLog record loop start, failed attempt, repaired result
-> app proof dump writes debug/c6-ai-repair-loop-proof/ai_repair_loop_report.json
```

C6.2 closes the retry/repair policy skeleton. It does not add natural-language parsing, remote sync, or new graph mutation commands.

## C6.2 Contract Check

| Boundary | Trigger | Input | Success | Failure | Log / proof |
| --- | --- | --- | --- | --- | --- |
| Repair loop orchestration | `executeAIWorkerRepairLoop(session, plan)` | repair id, worker id, intent, `maxAttempts`, ordered `AIWorkerCommandRequest` attempts | runs attempts through `executeAIWorkerCommand()`, stops at first `ok`, reports attempts run and final proof evidence | empty attempts, max attempts zero, all attempts fail, or attempt operation rejected | `ai_worker_repair_loop:*` command log entries plus collaboration log `repair_loop` entries |
| Command attempt | existing `executeAIWorkerCommand()` | one existing AI worker command request | shared command path result and evidence | existing command failure or rejection | existing `ai_worker:<operation>:*` command log and proof evidence |
| App proof | `--dump-c6-ai-repair-loop-proof-and-exit` | C2 work fixture, first bad move_node, second repaired move_node | report says one failed attempt, one successful repaired attempt, final node moved | fixture load/write failure, repair loop mismatch | `debug/c6-ai-repair-loop-proof/ai_repair_loop_report.json` |

## C6.2 Closed Slice

```text
AIWorkerRepairPlan(
  attempt 1: move_node missing node
  attempt 2: move_node library_loud1
)
-> executeAIWorkerRepairLoop()
-> attempt 1 fails through InteractionContract path
-> attempt 2 succeeds through InteractionContract path
-> loop stops before any extra attempt
-> collaborationLog records repair_loop failed_attempt + repaired evidence
-> debug/c6-ai-repair-loop-proof/ai_repair_loop_report.json
```

## C6.2 Evidence

- `source/ai/AIWorkerCommand.*` defines `AIWorkerRepairPlan`, `AIWorkerRepairAttemptResult`, and `AIWorkerRepairLoopResult`.
- `executeAIWorkerRepairLoop()` bounds attempts by `maxAttempts`, calls `executeAIWorkerCommand()` for each attempt, records failed attempt evidence, stops at the first successful command, and writes `ai_worker_repair_loop:*` command log entries.
- The repair loop does not mutate graph JSON directly and does not add a new graph mutation command.
- `tests/AIWorkerCommandTests.cpp` proves a missing-node `move_node` attempt fails, a repaired `move_node` attempt succeeds, the third attempt is not executed, the target node moves, and collaboration proof records `successfulAttemptIndex=2`.
- `--dump-c6-ai-repair-loop-proof-and-exit` writes `debug/c6-ai-repair-loop-proof/ai_repair_loop_report.json`.

## C6.2 Proof Report Must Say

```text
ok: true
operation: ai_repair_loop
repairId: c6.2-ai-repair-loop
attemptsRun: 2
maxAttempts: 3
status: repaired
firstAttemptStatus: failed
successfulAttemptIndex: 2
finalOperation: move_node
finalCommandLogStatus: ai_worker_repair_loop:repaired
graphMutationApplied: true
collaborationLogEntries >= 6
usesInteractionState: false
```

Latest C6.2 proof report says:

```text
ok: true
operation: ai_repair_loop
repairId: c6.2-ai-repair-loop
status: repaired
attemptsRun: 2
maxAttempts: 3
firstAttemptStatus: failed
successfulAttemptIndex: 2
finalOperation: move_node
finalCommandLogStatus: ai_worker_repair_loop:repaired
graphMutationApplied: true
collaborationLogEntries: 7
usesInteractionState: false
```

## C6 Closed Target

```text
analyzer compound family seed
-> raw-energy compound package + family library
-> runtime/visible registry proof
-> synthetic raw fact execution proof

AI repair loop closure
-> bounded repair attempts
-> each attempt uses executeAIWorkerCommand()
-> failed attempt evidence
-> repaired attempt stops the loop
-> command/collaboration proof evidence
```

C6 closes the two planned C6 candidate lines without reopening C5 and without adding natural-language parsing, analyzer detector semantics, remote sync, or broad publish UX.

## C6.2 Verification Run

```text
cmake --build build --target my_world_ai_worker_command_tests my-world
./build/my_world_ai_worker_command_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-ai-repair-loop-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```

Latest accepted targeted result:

```text
AI worker command contract ok
debug/c6-ai-repair-loop-proof/ai_repair_loop_report.json ok: true
30/30 tests passed
git diff --check passed
```

## Verification Gate

```text
cmake --build build --target my_world_analyzer_compound_family_tests my_world_runtime_registry_tests my_world_compound_module_tests my-world
./build/my_world_analyzer_compound_family_tests
./build/my_world_runtime_registry_tests
./build/my_world_compound_module_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-analyzer-family-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```

## C6 Proof Report Must Say

```text
ok: true
operation: analyzer_compound_family_seed
libraryPath: fixtures/module-libraries/analyzer-family.module-library.json
familyEntryCount: 2
visibleRegistryContainsRawEnergy: true
runtimeRegistryContainsRawEnergy: true
runtimeCoverageStatus: ready
createdRawEnergyNode: true
graphCommandLogStatus: create_node
rawEnergyPublicOutputs include rms, peak, sampleCount
loudnessStillPresent: true
usesInteractionState: false
```

Latest proof report says:

```text
ok: true
operation: analyzer_compound_family_seed
libraryPath: fixtures/module-libraries/analyzer-family.module-library.json
familyEntryCount: 2
visibleRegistryContainsRawEnergy: true
runtimeRegistryContainsRawEnergy: true
runtimeCoverageStatus: ready
createdRawEnergyNode: true
graphCommandLogStatus: create_node
rawEnergyPublicOutputs.rms: 0.53033
rawEnergyPublicOutputs.peak: 0.75
rawEnergyPublicOutputs.sampleCount: 4
loudnessStillPresent: true
usesInteractionState: false
```

## C6.1 Verification Run

```text
cmake --build build --target my_world_analyzer_compound_family_tests
./build/my_world_analyzer_compound_family_tests
cmake --build build --target my_world_runtime_registry_tests my_world_compound_module_tests
./build/my_world_runtime_registry_tests
./build/my_world_compound_module_tests
cmake --build build --target my-world
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-analyzer-family-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```

Latest accepted targeted result:

```text
analyzer compound family ok
runtime registry ok
compound module fixture ok
debug/c6-analyzer-family-proof/analyzer_family_report.json ok: true
30/30 tests passed
git diff --check passed
```
