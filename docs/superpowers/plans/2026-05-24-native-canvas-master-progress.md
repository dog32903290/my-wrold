# Native Canvas Master Progress Plan

> **For agentic workers:** Open this file first when checking progress. This is the single progress entrypoint; linked implementation plans and specs are evidence/detail, not competing dashboards.

**Goal:** Keep one current progress surface for `我的世界` so C/R/visual/TiXL work can proceed without contradicting active sessions.

**Architecture:** Specs define contracts and closure evidence. This master plan defines current lane ownership, sequence, blockers, and conflict rules. Old implementation plans remain historical construction records unless this file marks them active.

**Tech Stack:** Markdown progress plan, linked specs/plans, C++20/CMake verification gates, app proof dump commands.

---

## Operating Rule

```text
追進度只先開這份：
docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md
```

Then follow links from this file only when a lane needs detail.

Do not use old implementation plans as current status. Many old plans contain "parked" notes that were true at that slice but are no longer current after C2/C3.

## Source Hierarchy

| Rank | Document type | Role |
| --- | --- | --- |
| 1 | This master progress plan | Current status, active lane, sequencing, conflicts |
| 2 | Closure specs | Proven evidence and parked scope for a finished lane |
| 3 | Active slice spec | Contract for the lane currently being built |
| 4 | Active implementation plan | Step-by-step construction for the current slice |
| 5 | Old implementation plans | Historical records only |

## Current Snapshot

Date: 2026-05-24 22:12 Asia/Taipei.

Branch:

```text
codex/tooll3-interaction-t0-t7
```

Local repo relocation note:

```text
2026-05-24 17:21 Asia/Taipei:
Current working repo is /Users/chenbaiwei/Projects/my-world.
Historical docs also mention /Users/chenbaiwei/Desktop/我的世界 and /Users/chenbaiwei/Projects/我的世界.
Treat those as path breadcrumbs, not current working-directory truth.
For future path/build/debug-output bugs, check:
docs/superpowers/handoffs/2026-05-24-repo-relocation-note.md
```

Latest known commits:

```text
5cc6399 Close R runtime render backbone
ebeab9f Add R2 headless render runtime
d884f2d Close R1 OpenGL render backend
94fc456 Add render backend contract
fd1e972 Plan R segment implementation
90a4b40 Route R runtime render backbone
a6390b6 Split storage contract serialization
cff975e Split runtime registry responsibilities
2c53c05 Split ImGui smoke overlay helpers
bbadbe8 Schedule post-C three-layer cleanup
311b64e Tidy C proof request fixtures
2c49f1a Extract C proof reports
0f81a42 Tidy C proof directory setup
5fb9d66 Tidy C6 proof fixtures
cdb35da Tidy C6 repair loop closure
6939992 Close C6 analyzer family and repair loop
0754301 Close C5.3 visible module publish
9b48941 Add C5.2 AI worker module publish
73ce3f0 Close C5.1 module publish proof
ec3efe2 Close C4 AI worker command proof
d73b893 Add C4.2 AI worker move_node command
835ce85 Add C4.1 AI worker save_work contract
09edd86 Add C3.5 save commit failure proof
57ba0c5 Add C3.4 background save commit worker
```

Current C4 note:

```text
C4.1 AI worker save_work command contract is closed.
C4.2 AI worker move_node command contract is closed.
C4.3 AI worker move_node -> save_work app proof closure is closed in the current slice.
Current C4 evidence is in:
- docs/superpowers/specs/2026-05-24-c4-ai-worker-command-contract.md
- debug/c4-ai-worker-save-work-proof/ai_worker_save_work_report.json

Verification run:
- `cmake --build build --target my_world_ai_worker_command_tests`
- `./build/my_world_ai_worker_command_tests`
- `cmake --build build --target my-world`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c4-ai-worker-save-work-proof-and-exit`
- `git diff --check`
```

Do not reopen C4.1-C4.3 to add natural language parsing, repair loop, remote sync, or additional graph mutation commands.

Current C5 note:

```text
C5.1 module publish/reuse command + storage/runtime proof is closed and pushed in 73ce3f0.
C5.2 AI worker publish_module command path is closed and pushed in 9b48941.
C5.3 visible publish hand is closed and pushed in 0754301.
C5 is closed. Do not reopen C5 for analyzer-family or repair-loop work.
C5 closure spec:
- docs/superpowers/specs/2026-05-24-c5-module-publish-reuse-path.md
Current C5 evidence is in:
- tests/ModulePublishTests.cpp
- tests/AIWorkerCommandTests.cpp
- debug/c5-module-publish-proof/module_publish_report.json
- debug/c5-ai-worker-module-publish-proof/ai_worker_module_publish_report.json
- debug/c5-visible-module-publish-proof/visible_module_publish_report.json

Verification run:
- `cmake --build build --target my_world_module_publish_tests my_world_save_work_command_tests my_world_compound_module_tests my_world_runtime_registry_tests my-world`
- `./build/my_world_module_publish_tests`
- `./build/my_world_save_work_command_tests`
- `./build/my_world_compound_module_tests`
- `./build/my_world_runtime_registry_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-module-publish-proof-and-exit`
- `git diff --check`
- `ctest --test-dir build --output-on-failure`

Current C5.2 verification:
- `cmake --build build --target my_world_ai_worker_command_tests my-world`
- `./build/my_world_ai_worker_command_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-ai-worker-module-publish-proof-and-exit`
- `git diff --check`
- `ctest --test-dir build --output-on-failure`

Current C5.3 verification:
- `cmake --build build --target my-world`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-visible-module-publish-proof-and-exit`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`
```

Current C6 note:

```text
C6.1 analyzer compound family seed is closed in this slice.
C6.2 AI repair loop closure is closed in this slice.
C6 is closed. It did not add natural-language parsing and did not reopen C5.
C6 post-close hygiene stays contract-neutral:
- cdb35da extracts repair-loop terminal logging/proof closure and adds rejected-branch tests.
- 5fb9d66 keeps C6 app proof fixtures local, with no node/runtime/schema change.
- current C-segment hygiene passes share C2-C6 proof directory setup and extract C2-C6 proof report builders into `source/app/ProofReports.*`, with no proof report schema change.
- current C-segment hygiene also extracts C4/C5 proof request fixture builders in `source/app/MainComponent.cpp`, with no command payload or proof flow change.
C6 spec:
- docs/superpowers/specs/2026-05-24-c6-analyzer-compound-family.md
C6.1 evidence is in:
- tests/AnalyzerCompoundFamilyTests.cpp
- fixtures/compounds/raw-energy.compound.json
- fixtures/modules/raw-energy/module.json
- fixtures/module-libraries/analyzer-family.module-library.json
- debug/c6-analyzer-family-proof/analyzer_family_report.json
C6.1 proof:
analyzer family library fixture
-> compound.raw-energy package
-> visible/runtime registry load
-> runtime coverage create-enabled
-> synthetic execution publishes raw rms / peak / sampleCount
-> InteractionContract createNode proof

Current C6.1 targeted verification:
- `cmake --build build --target my_world_analyzer_compound_family_tests`
- `./build/my_world_analyzer_compound_family_tests`
- `cmake --build build --target my_world_runtime_registry_tests my_world_compound_module_tests`
- `./build/my_world_runtime_registry_tests`
- `./build/my_world_compound_module_tests`
- `cmake --build build --target my-world`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-analyzer-family-proof-and-exit`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:
- `analyzer compound family ok`
- `runtime registry ok`
- `compound module fixture ok`
- `debug/c6-analyzer-family-proof/analyzer_family_report.json` has `ok: true`
- `30/30 tests passed`
- `git diff --check passed`

C6.2 proof:
AIWorkerRepairPlan
-> bounded attempts
-> every attempt goes through executeAIWorkerCommand()
-> failed move_node attempt records evidence
-> repaired move_node attempt succeeds and stops the loop
-> debug/c6-ai-repair-loop-proof/ai_repair_loop_report.json

Current C6.2 targeted verification:
- `cmake --build build --target my_world_ai_worker_command_tests my-world`
- `./build/my_world_ai_worker_command_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-ai-repair-loop-proof-and-exit`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted targeted result:
- `AI worker command contract ok`
- `debug/c6-ai-repair-loop-proof/ai_repair_loop_report.json` has `ok: true`
- `30/30 tests passed`
- `git diff --check passed`
```

## Main Spine

| Lane | Status | Current law / evidence | Next |
| --- | --- | --- | --- |
| V1 visual proof | proven | `--dump-proof-and-exit` | R lane absorbed runtime/render ownership through R3 |
| A1 audio proof | proven | `--dump-audio-proof-and-exit` | Raw callback-buffer runtime remains parked |
| C1 compound core | closed | `docs/superpowers/specs/2026-05-24-c1-24-compound-proof-closure.md` | Do not reopen C1 |
| C2 PatchDocument work persistence | closed | `docs/superpowers/specs/2026-05-24-c2-compound-work-closure.md` | Do not reopen C2 |
| C3 storage command path | closed | `docs/superpowers/specs/2026-05-24-c3-storage-command-path.md` | Do not reopen C3 |
| C4 AI worker command path | closed | `docs/superpowers/specs/2026-05-24-c4-ai-worker-command-contract.md` | Do not reopen C4 |
| C5 module publish/reuse path | closed | `docs/superpowers/specs/2026-05-24-c5-module-publish-reuse-path.md` | Do not reopen C5 for C6 work |
| C6.1 analyzer compound family seed | closed | `docs/superpowers/specs/2026-05-24-c6-analyzer-compound-family.md` | do not reopen for detector semantics |
| C6.2 AI repair loop closure | closed | `docs/superpowers/specs/2026-05-24-c6-analyzer-compound-family.md` | do not reopen for natural-language parsing |
| H1 post-C three-layer cleanup | closed | `docs/superpowers/specs/2026-05-24-post-c-three-layer-cleanup.md` | Do not reopen for R/PV/TiXL work |
| R runtime/render backbone | closed | R1 closed: OpenGL proof now runs behind `RenderBackend`; R2 closed: headless `image.constant -> output.texture_summary` writes debug artifacts; R3 closed: V1/R2 proof JSON fields are compatible; `ctest` 33/33 | Do not reopen for PV work |
| PV/analyzer detector expansion | closed | `docs/superpowers/specs/2026-05-24-pv-analyzer-detector-expansion-closure.md`; attack/density/silence focused tests and app proof dumps passed; `ctest` 36/36 | Do not reopen for sustain/aggregate/UI mapping without a new selected lane |
| PV/sustain detector | closed | `docs/superpowers/specs/2026-05-24-pv-analyzer-sustain-detector-closure.md`; focused tests and app proof dump passed; `ctest` 37/37 | Do not reopen for residue/aggregate/UI mapping without a new selected lane |
| PV/residue detector | closed | `docs/superpowers/specs/2026-05-24-pv-analyzer-residue-detector-closure.md`; focused tests and app proof dump passed; `ctest` 38/38 | Do not reopen for aggregate/UI mapping without a new selected lane |
| PV/aggregate pressure | closed | `docs/superpowers/specs/2026-05-24-pv-analyzer-aggregate-pressure-closure.md`; focused tests and app proof dump passed; `ctest` 39/39 | Do not reopen for UI/MIDI/shader mapping without a new selected lane |
| PV-B1 analyzer environment promotion | closed | `docs/superpowers/specs/2026-05-24-pv-b1-analyzer-environment-promotion-closure.md`; focused test and app proof dump passed; V1 runtime UI diagnostics include PV compounds; `ctest` 40/40 | Do not reopen for MIDI/shader/live callback work without a new selected lane |
| PV-B1.1 node surface geometry | closed | `docs/superpowers/specs/2026-05-24-pv-b1-node-surface-geometry.md`; TiXL-style `NodeSpec -> CanvasNodeSurfaceGeometry -> draw/hit-test`; focused tests, app build, PV-B1 proof, full `ctest` 40/40, `git diff --check` passed | Do not reopen for broader skin, thumbnails, MIDI/shader mapping, or TiXL runtime work without a new selected lane |
| PV proof harness extraction | closed | `docs/superpowers/specs/2026-05-24-pv-proof-harness-extraction.md`; `PVDetectorProofRunner` owns detector proof runtime/fixture/artifact orchestration; focused tests, app build, and attack CLI proof passed | Do not reopen detector semantics; C2-C6/PV-B1/V1 proof harness cleanup remain separate selected lanes |
| PV-B1 proof harness extraction | closed | `docs/superpowers/specs/2026-05-24-pv-b1-proof-harness-extraction.md`; `PVB1AnalyzerEnvironmentProofRunner` owns visible-catalog proof artifact orchestration; focused test, app build, and PV-B1 CLI proof passed | Do not reopen PV-B1 semantics; C2-C6/V1 proof harness cleanup remain separate selected lanes |
| C6 proof harness extraction | closed | `docs/superpowers/specs/2026-05-24-c6-proof-harness-extraction.md`; `C6AnalyzerFamilyProofRunner` owns raw-energy visible/runtime/synthetic-audio proof orchestration; focused test, app build, and C6 CLI proof passed | Do not reopen C6 analyzer semantics; C2-C5/C6 repair/V1 proof harness cleanup remain separate selected lanes |
| TiXL parity | ledgered, not main spine | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md` | P-TAX1 only after next active lane is selected |

## Active Lane Protocol

Only one lane should be marked `in progress` in this file unless the files are disjoint.

Current active lane:

```text
None after C6 proof harness extraction as of 2026-05-24 23:59 Asia/Taipei.

C6 proof harness extraction closed as of 2026-05-24 23:59 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-24-c6-proof-harness-extraction.md

Closed proof line:
C6AnalyzerFamilyProofRunRequest
-> runC6AnalyzerFamilyProof()
-> analyzer family visible/runtime/synthetic-audio proof
-> analyzer_family_report.json
-> MainComponent status facade

Latest verification:
- `cmake -S . -B build` red before runner source existed, then green after implementation
- `cmake --build build --target my_world_c6_analyzer_family_proof_runner_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "c6_analyzer_family_proof_runner|analyzer_compound_family"`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-analyzer-family-proof-and-exit`

Latest accepted result:
- `analyzer_compound_family` passed
- `c6_analyzer_family_proof_runner` passed
- app build passed
- CLI C6 proof exited 0 and wrote `debug/c6-analyzer-family-proof/analyzer_family_report.json` with `ok: true`, `familyEntryCount: 2`, `runtimeCoverageStatus: "ready"`, and `loudnessStillPresent: true`

Parked:
- C2-C5 proof orchestration remains in `MainComponent`
- C6 AI repair-loop proof remains in `MainComponent`
- V1 shader proof remains tied to `OpenGLShaderPreview`

Previous closure:
PV-B1 proof harness extraction closed as of 2026-05-24 23:52 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-24-pv-b1-proof-harness-extraction.md

Closed proof line:
PVB1AnalyzerEnvironmentProofRunRequest
-> runPVB1AnalyzerEnvironmentProof()
-> analyzer visible module library + visible catalog proof
-> analyzer_environment_report.json
-> MainComponent status facade

Latest verification:
- `cmake -S . -B build` red before runner source existed, then green after implementation
- `cmake --build build --target my_world_pv_b1_analyzer_environment_proof_runner_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "pv_b1_analyzer_environment_proof_runner|analyzer_visible_catalog"`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-b1-analyzer-environment-proof-and-exit`

Latest accepted result:
- `analyzer_visible_catalog` passed
- `pv_b1_analyzer_environment_proof_runner` passed
- app build passed
- CLI PV-B1 proof exited 0 and wrote `debug/pv-b1-analyzer-environment-proof/analyzer_environment_report.json` with `ok: true`, `loadedModuleNodeCount: 8`, and `createdNodeCount: 8`

Parked:
- C2-C5 proof orchestration remains in `MainComponent`
- C6 AI repair-loop proof remains in `MainComponent`
- V1 shader proof remains tied to `OpenGLShaderPreview`

Previous closure:
PV proof harness extraction closed as of 2026-05-24 23:17 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-24-pv-proof-harness-extraction.md

Closed proof line:
PVDetectorProofRunRequest
-> runPVDetectorProof()
-> detector runtime registry + fixture proof
-> report/cook_order/node_stats/errors artifacts
-> MainComponent status facade

Latest verification:
- `cmake --build build --target my_world_pv_detector_proof_runner_tests`
- `ctest --test-dir build --output-on-failure -R pv_detector_proof_runner`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "analyzer_detector|analyzer_density_detector|analyzer_silence_detector|analyzer_sustain_detector|analyzer_residue_detector|analyzer_aggregate_pressure|pv_detector_proof_runner"`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-attack-detector-proof-and-exit`

Latest accepted result:
- `pv_detector_proof_runner` passed
- focused analyzer/PV tests: 7/7 passed
- app build passed
- CLI attack proof exited 0 and wrote `debug/pv-attack-detector-proof/attack_detector_report.json` with `ok: true`

Parked:
- C2-C6 proof orchestration remains in `MainComponent`
- V1 shader proof remains tied to `OpenGLShaderPreview`

Previous closure:
PV-B1.1 node surface geometry closed as of 2026-05-24 22:33 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-24-pv-b1-node-surface-geometry.md

Closed proof line:
GraphNode + NodeSpec
-> CanvasNodeSurfaceGeometry
-> ImGui node drawing
-> InteractionContract hit-test / portCenter

Latest accepted verification:
- focused geometry + hit-test tests passed
- app target built
- PV-B1 analyzer environment proof exited 0 and wrote `ok: true`
- full ctest passed 40/40
- git diff --check passed

Closed boundary:
This lane only fixes node surface geometry for visible PV/analyzer compounds. It does not add analyzer DSP, MIDI mapping, shader uniform mapping, browser polish, live callback-buffer runtime, Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, TiXL runtime code, relocation-note behavior, flow-runner behavior, or `AGENTS.md` changes.

PV-B1 analyzer environment promotion closed as of 2026-05-24 22:12 Asia/Taipei.
Closure spec:
- docs/superpowers/specs/2026-05-24-pv-b1-analyzer-environment-promotion-closure.md

Initial PV-B1 spec:
- docs/superpowers/specs/2026-05-24-pv-b1-analyzer-environment-promotion.md

Closed proof line:
fixtures/module-libraries/pv-analyzer-visible.module-library.json
-> visible node browser/create registry
-> debug/pv-b1-analyzer-environment-proof/analyzer_environment_report.json

Contract:
closed analyzer module packages
-> visible NodeSpec catalog + runtime-op diagnostics
-> createNode command path for compound.loudness, compound.raw-energy, compound.attack, compound.density, compound.silence, compound.sustain, compound.residue, compound.aggregate-pressure

This lane did not add analyzer DSP, MIDI mapping, shader uniform mapping, browser polish, live callback-buffer runtime, Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, TiXL, or flow-runner behavior.

Latest accepted verification:
- focused analyzer visible catalog test passed
- app proof dump for PV-B1 analyzer environment exited 0 and wrote `ok: true`
- V1 proof dump still exited 0 and runtime UI diagnostics include PV compounds as runtime ready
- ctest passed 40/40

PV/aggregate pressure closed as of 2026-05-24 21:05 Asia/Taipei.
Closure spec:
- docs/superpowers/specs/2026-05-24-pv-analyzer-aggregate-pressure-closure.md

Initial aggregate spec:
- docs/superpowers/specs/2026-05-24-pv-analyzer-aggregate-pressure.md

Closed proof line:
fixtures/analyzer/aggregate_pressure_cases.json
-> analyzer.aggregate_pressure runtime aggregate
-> debug/pv-aggregate-pressure-proof/aggregate_pressure_report.json + cook_order.json + node_stats.json + errors.json

Contract:
raw-energy.rms + attack_value + density_value + sustain_envelope + residue_envelope + silence_state
-> analyzer.aggregate_pressure
-> pressure_value + energy_component + attack_component + density_component + sustain_component + residue_component + confidence

This lane did not implement UI promotion, MIDI mapping, shader uniform mapping, browser promotion, or live callback-buffer runtime.

Latest accepted verification:
- focused aggregate test passed
- runtime registry test passed
- app proof dump for aggregate pressure exited 0 and wrote `ok: true`
- ctest passed 39/39

PV/residue detector closed as of 2026-05-24 19:55 Asia/Taipei.
Closure spec:
- docs/superpowers/specs/2026-05-24-pv-analyzer-residue-detector-closure.md

Initial residue spec:
- docs/superpowers/specs/2026-05-24-pv-analyzer-residue-detector.md

Closed proof line:
fixtures/analyzer/residue_detector_cases.json
-> analyzer.residue runtime detector
-> debug/pv-residue-detector-proof/residue_detector_report.json + cook_order.json + node_stats.json + errors.json

Contract:
raw-energy.rms + sustain_envelope, optionally cleared by silence_state
-> analyzer.residue
-> residue_state + residue_envelope + residue_timer_ms + confidence

This lane did not implement aggregate pressure, UI smoothing, MIDI mapping, shader uniform mapping, or live callback-buffer runtime.

Latest accepted verification:
- focused residue test passed
- runtime registry test passed
- app proof dump for residue exited 0 and wrote `ok: true`
- ctest passed 38/38

PV/sustain detector closed as of 2026-05-24 19:09 Asia/Taipei.
Closure spec:
- docs/superpowers/specs/2026-05-24-pv-analyzer-sustain-detector-closure.md

Initial sustain spec:
- docs/superpowers/specs/2026-05-24-pv-analyzer-sustain-detector.md

Closed proof line:
fixtures/analyzer/sustain_detector_cases.json
-> analyzer.sustain runtime detector
-> debug/pv-sustain-detector-proof/sustain_detector_report.json + cook_order.json + node_stats.json + errors.json

Contract:
raw-energy.rms held above floor for holdMs
-> analyzer.sustain
-> sustain_state + sustain_timer_ms + sustain_envelope + confidence

This lane must not consume UI-smoothed loudness, attack envelope, density value, or silence output as detector truth.

Latest accepted verification:
- focused sustain test passed
- runtime registry test passed
- app proof dump for sustain exited 0 and wrote `ok: true`
- ctest passed 37/37

PV/analyzer detector expansion closed as of 2026-05-24 18:28 Asia/Taipei.
Closure spec:
- docs/superpowers/specs/2026-05-24-pv-analyzer-detector-expansion-closure.md

Initial attack spec:
- docs/superpowers/specs/2026-05-24-pv-analyzer-attack-detector.md

Closed proof lines:
fixtures/analyzer/attack_detector_cases.json
-> analyzer.attack runtime detector
-> debug/pv-attack-detector-proof/attack_detector_report.json + cook_order.json + node_stats.json + errors.json

fixtures/analyzer/density_detector_cases.json
-> analyzer.density runtime detector
-> debug/pv-density-detector-proof/density_detector_report.json + cook_order.json + node_stats.json + errors.json

fixtures/analyzer/silence_detector_cases.json
-> analyzer.silence runtime detector
-> debug/pv-silence-detector-proof/silence_detector_report.json + cook_order.json + node_stats.json + errors.json

Latest accepted verification:
- focused attack/density/silence tests passed
- app proof dumps for attack/density/silence exited 0 and wrote `ok: true`
- ctest passed 36/36
- git diff --check passed

H1.1 UI overlay split is closed.
H1.2 RuntimeRegistry split is closed.
H1.3 StorageContract split is closed in the current slice.
R0 runtime/render backbone roadmap spec is closed in the current slice.
R1 OpenGL proof backend extraction is closed in d884f2d.
R2 headless image.constant runtime proof is closed in ebeab9f.
R3 runtime/render proof compatibility closure is closed in 5cc6399.
```

Current PV note:

```text
Current PV analyzer line has closed detector, aggregate, and environment-promotion contracts:

attack:
raw-energy.rms / raw-energy.peak
-> analyzer.attack
-> onset_event + attack_value + attack_envelope + confidence

density:
attack.onset_event
-> analyzer.density
-> density_value + event_count + density_envelope + confidence

silence:
raw-energy.rms below floor for holdMs
-> analyzer.silence
-> silence_state + silence_timer_ms + confidence

sustain:
raw-energy.rms above floor for holdMs
-> analyzer.sustain
-> sustain_state + sustain_timer_ms + sustain_envelope + confidence

residue:
raw-energy.rms + sustain_envelope, optionally cleared by silence_state
-> analyzer.residue
-> residue_state + residue_envelope + residue_timer_ms + confidence

aggregate_pressure:
raw-energy.rms + attack_value + density_value + sustain_envelope + residue_envelope + silence_state
-> analyzer.aggregate_pressure
-> pressure_value + energy_component + attack_component + density_component + sustain_component + residue_component + confidence

Density proof explicitly says `usesAttackEnvelopeForDetector: false`.
Attack proof explicitly says `usesOutputSmoothingForDetector: false`.
Silence proof explicitly says it uses raw RMS and does not implement attack/density.
Sustain proof explicitly says it uses raw RMS and does not use attack onset or silence state for detector truth.
Residue proof explicitly says it uses raw RMS + sustain envelope, uses silence state only to clear, and does not use output smoothing.
Aggregate pressure proof explicitly says it uses raw RMS + detector states and does not use output smoothing for the aggregate.

PV-B1:
fixtures/module-libraries/pv-analyzer-visible.module-library.json
-> visible node browser/create registry
-> compound.loudness + compound.raw-energy + compound.attack + compound.density + compound.silence + compound.sustain + compound.residue + compound.aggregate-pressure are runtime ready and create-enabled

PV-B1 proof explicitly says it adds no analyzer DSP, MIDI mapping, shader uniform mapping, or live callback runtime.
```

Current R note:

```text
R0 roadmap spec:
- docs/superpowers/specs/2026-05-24-r-runtime-render-backbone-roadmap.md

R segment implementation plan:
- docs/superpowers/plans/2026-05-24-r-segment-implementation.md

R1 first line:
makeDefaultShaderOutputGraph()
-> OpenGL proof backend behind RenderBackend
-> debug/v1-shader-proof/frame.png + cook_order.json + node_stats.json still pass

R2 first new runtime-node line:
fixtures/runtime/top_constant_to_output.graph.json
-> headless image.constant runtime
-> debug/r2-top-constant/texture_summary.json + cook_order.json + node_stats.json + errors.json
```

R closed without Metal, image.blur, node thumbnails, SOP/MAT/POINT, or render export.

R1 closed evidence:

```text
my_world_opengl_render_backend_tests printed `opengl render backend ok`.
--dump-proof-and-exit wrote debug/v1-shader-proof/{frame.png,cook_order.json,node_stats.json}.
node_stats.json kept `"renderer": "OpenGL"`.
ctest passed 32/32 before d884f2d.
```

R2 closed evidence:

```text
my_world_headless_render_runtime_tests printed `headless render runtime ok`.
debug/r2-top-constant/texture_summary.json reports 1280x720 rgba8.
debug/r2-top-constant/cook_order.json orders const1 before out1.
debug/r2-top-constant/node_stats.json records render-domain image.constant and output.texture_summary nodes.
debug/r2-top-constant/errors.json has `"ok": true`.
Invalid resolution and unsupported node type fixtures fail with errors evidence.
ctest passed 33/33.
```

R3 closed evidence:

```text
V1 and R2 cook_order.json both expose `"version": 1` and `cookOrder`.
V1 node_stats.json reports `"renderer": "OpenGL"`.
R2 node_stats.json reports `"renderer": "headless"`.
Both proofs expose render-domain node status evidence.
R3 verification: focused R tests passed, --dump-proof-and-exit exited 0, ctest passed 33/33, git diff --check passed.
No Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, storage schema, or reopened C4/C5/C6 behavior entered R.
```

Current H1.1 note:

```text
H1.1 splits source/ui/ImGuiSmokeOverlay.cpp into:
- source/ui/ImGuiSmokeOverlayHelpers.*
- source/ui/ImGuiSmokeOverlayBrowser.cpp
- source/ui/ImGuiSmokeOverlayInspector.cpp

Public ImGuiSmokeOverlay API stays stable.
Command callbacks and graph mutation paths are unchanged.

Verification run:
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-visible-module-publish-proof-and-exit`
- `git diff --check`
```

Current H1.2 note:

```text
H1.2 splits source/core/RuntimeRegistry.cpp into:
- source/core/RuntimeRegistry.cpp
- source/core/RuntimeRegistryDiagnostics.cpp
- source/core/RuntimeRegistryJson.cpp
- source/core/RuntimeRegistrySynthetic.cpp
- source/core/RuntimeRegistryInternals.h

RuntimeRegistry value structs and public function signatures stay stable.
Registry schema and RuntimeOp semantics are unchanged.

Verification run:
- `cmake --build build --target my_world_runtime_registry_tests my_world_analyzer_compound_family_tests my-world`
- `./build/my_world_runtime_registry_tests`
- `./build/my_world_analyzer_compound_family_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-analyzer-family-proof-and-exit`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-ai-repair-loop-proof-and-exit`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`
```

Current H1.3 note:

```text
H1.3 splits source/storage/StorageContract.cpp into:
- source/storage/StorageContract.cpp
- source/storage/StorageContractJson.*
- source/storage/StorageContractWorkProject.cpp
- source/storage/StorageContractPatchDocument.cpp
- source/storage/StorageContractModules.cpp

Storage JSON field names and parse behavior stay stable.
save_work behavior is unchanged.

Verification run:
- `cmake --build build --target my_world_storage_tests my_world_patch_document_tests my_world_save_work_command_tests my_world_module_publish_tests my-world`
- `./build/my_world_storage_tests`
- `./build/my_world_patch_document_tests`
- `./build/my_world_save_work_command_tests`
- `./build/my_world_module_publish_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c2-storage-proof-and-exit`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c3-save-work-proof-and-exit`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-module-publish-proof-and-exit`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`
```

C4 proved:

```text
AI worker request
-> allowed operation boundary
-> move_node through InteractionContract
-> save_work through StorageCommand
-> command/collaboration proof evidence
-> moved node persists through PatchDocument reload
```

Park during C4:

```text
RenderBackend extraction
visual vocabulary
TiXL browser/search implementation
remote sync
raw callback-buffer runtime
```

Scheduled before those parked lanes:

```text
H1 post-C three-layer cleanup
-> source/ui/ImGuiSmokeOverlay.cpp responsibility split (done in H1.1)
-> source/core/RuntimeRegistry.cpp responsibility split (done in H1.2)
-> source/storage/StorageContract.cpp serialization split (done in H1.3)
```

Parked after C6:

```text
MIDI mapping and shader uniform mapping
multi-node group-to-compound extraction
browser polish beyond analyzer admission/create proof
remote sync / shared module registry
TiXL browser/search implementation
raw callback-buffer runtime
```

## Conflict Register

| Conflict | Where it appears | Decision |
| --- | --- | --- |
| Old S0 plan says production `Command+S` / local git worker parked, but C3 spec says C3.1-C3.5 are closed | `2026-05-22-s0-storage-proof.md` vs `2026-05-24-c3-storage-command-path.md` | Treat S0 plan as historical. Current status comes from C3 spec and this master plan. |
| C2 plan parks local git auto-commit, but C3 implements opt-in local git worker | `2026-05-24-c2-compound-work-closure.md` vs C3 spec | Not a contradiction if scoped by lane. C2 did not claim git; C3 later implemented it. |
| `P` prefix is overloaded | Tooll3 skin `P0-P7`, TiXL `P-TAX/P-SEARCH`, future "P segment" talk | Future performance/patch-vocabulary lane must not use bare `P1/P2`. Use `PV-*` for patch vocabulary or rename the lane before writing specs. |
| TiXL ledger is a plan but also tracks progress | `2026-05-24-tixl-parity-construction-ledger.md` | It is a sub-ledger. Start progress from this master plan, then open TiXL ledger only for TiXL-visible parity work. |
| Skeleton spec can become too long and look like the dashboard | `2026-05-22-native-canvas-skeleton-design.md` | Skeleton is architecture/status evidence. This master plan is the current dashboard. |
| C4 closes save_work and move_node only, not the full AI worker loop | C4 spec vs future AI worker expectations | Repair-loop orchestration moved to C6.2. Natural language parsing, additional graph mutation commands, and remote sync remain parked outside C4/C6. |
| C5 can be mistaken for new analyzer/module vocabulary work | C5 title vs C6 analyzer family | C5.1 only proves publishing and reusing one selected compound source. C6.1 owns the analyzer family seed; detector semantics remain future PV work. |
| TiXL node-function spec names `top_constant_to_output` as the first runtime slice, while skeleton spec blocks production visual work on RenderBackend extraction | `2026-05-23-node-function-spec-from-tixl.md` vs `2026-05-22-native-canvas-skeleton-design.md` | R roadmap resolves this by making R1 the backend ownership extraction for existing V1 proof, then R2 the first new headless runtime-node slice. |
| PV work was parked until explicitly selected | master plan parked PV before R closed | Resolved and closed through attack/density/silence. Sustain, residue, aggregate pressure, UI promotion, live callback-buffer work, and shader/MIDI mapping remain parked. |
| Sustain was selected after detector expansion closure | PV closure parked sustain until a new selected lane | Resolved and closed by `docs/superpowers/specs/2026-05-24-pv-analyzer-sustain-detector-closure.md`. Residue, aggregate pressure, UI promotion, shader/MIDI mapping, and live callback-buffer work remain parked. |
| Residue was selected after sustain closure | Sustain closure parked residue until a new selected lane | Resolved and closed by `docs/superpowers/specs/2026-05-24-pv-analyzer-residue-detector-closure.md`. Aggregate pressure, UI promotion, shader/MIDI mapping, and live callback-buffer work remain parked. |
| Aggregate pressure was selected after residue closure | Residue closure parked aggregate pressure until a new selected lane | Resolved and closed by `docs/superpowers/specs/2026-05-24-pv-analyzer-aggregate-pressure-closure.md`. UI promotion, shader/MIDI mapping, live callback-buffer work, and additional aggregate families remain parked. |
| Analyzer environment promotion was selected after aggregate pressure closure | Aggregate closure parked browser/UI promotion until a new selected lane | Resolved and closed by `docs/superpowers/specs/2026-05-24-pv-b1-analyzer-environment-promotion-closure.md`. Browser polish, MIDI mapping, shader mapping, live callback-buffer work, and additional aggregate families remain parked. |

## Plan Inventory

| Plan | Current role | Open directly for progress? |
| --- | --- | --- |
| `2026-05-22-a1-audio-proof.md` | historical A0/A1 implementation plan | no |
| `2026-05-22-codex-canvas-hands-v0.md` | historical/utility implementation plan | no |
| `2026-05-22-g0-graph-language-contract.md` | historical graph-language implementation plan | no |
| `2026-05-22-s0-storage-proof.md` | historical storage implementation plan, superseded by C2/C3 for save status | no |
| `2026-05-22-tooll3-interaction-t0-t7.md` | historical interaction implementation plan | no |
| `2026-05-24-c2-1-patch-document-boundary.md` | historical C2.1 implementation plan | no |
| `2026-05-24-c2-compound-work-closure.md` | historical C2 implementation/closure plan | no |
| `2026-05-24-c5-module-publish-reuse-path.md` | C5 closure evidence | no, unless auditing C5 evidence |
| `2026-05-24-c6-analyzer-compound-family.md` | C6 closure evidence | no, unless auditing C6 evidence |
| `2026-05-24-pv-analyzer-attack-detector.md` | initial PV attack detector contract | no, unless auditing attack contract |
| `2026-05-24-pv-analyzer-detector-expansion-closure.md` | PV attack/density/silence closure evidence | no, unless auditing PV closure |
| `2026-05-24-pv-analyzer-sustain-detector.md` | PV sustain detector contract | no, unless auditing sustain |
| `2026-05-24-pv-analyzer-sustain-detector-closure.md` | PV sustain closure evidence | no, unless auditing sustain closure |
| `2026-05-24-pv-analyzer-residue-detector.md` | PV residue detector contract | no, unless auditing residue |
| `2026-05-24-pv-analyzer-residue-detector-closure.md` | PV residue closure evidence | no, unless auditing residue closure |
| `2026-05-24-pv-analyzer-aggregate-pressure.md` | initial PV aggregate pressure contract | no, unless auditing aggregate pressure |
| `2026-05-24-pv-analyzer-aggregate-pressure-closure.md` | PV aggregate pressure closure evidence | no, unless auditing aggregate pressure closure |
| `2026-05-24-pv-b1-analyzer-environment-promotion.md` | initial PV-B1 analyzer environment promotion contract | no, unless auditing PV-B1 |
| `2026-05-24-pv-b1-analyzer-environment-promotion-closure.md` | PV-B1 analyzer environment promotion closure evidence | no, unless auditing PV-B1 closure |
| `2026-05-24-post-c-three-layer-cleanup.md` | H1 closure evidence | no, unless auditing H1 evidence |
| `2026-05-24-r-runtime-render-backbone-roadmap.md` | R closure evidence and historical routing | no, unless auditing R evidence |
| `2026-05-24-r-segment-implementation.md` | R implementation/closure history | no, unless auditing R evidence |
| `2026-05-24-flow-runner-automation.md` | untracked separate flow-runner lane owned outside C6 | no |
| `2026-05-24-tixl-parity-construction-ledger.md` | TiXL-visible parity sub-ledger, not the current active lane | only from this master plan |

## Session Safety

Files outside PV detector expansion ownership that this pass must not edit or include unless explicitly requested:

```text
AGENTS.md
docs/superpowers/handoffs/2026-05-24-repo-relocation-note.md
docs/superpowers/plans/2026-05-24-flow-runner-automation.md
scripts/
tests/test_myworld_flow.py
```

PV analyzer owned files:

```text
docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md
docs/superpowers/specs/2026-05-24-pv-analyzer-attack-detector.md
docs/superpowers/specs/2026-05-24-pv-analyzer-detector-expansion-closure.md
docs/superpowers/specs/2026-05-24-pv-b1-analyzer-environment-promotion.md
docs/superpowers/specs/2026-05-24-pv-b1-analyzer-environment-promotion-closure.md
docs/nodes/analyzer.attack.md
docs/nodes/analyzer.density.md
docs/nodes/analyzer.silence.md
fixtures/analyzer/
fixtures/compounds/attack.compound.json
fixtures/compounds/density.compound.json
fixtures/compounds/silence.compound.json
fixtures/compounds/sustain.compound.json
fixtures/compounds/residue.compound.json
fixtures/compounds/aggregate-pressure.compound.json
fixtures/module-libraries/pv-analyzer-visible.module-library.json
fixtures/module-libraries/pv-*-detector.module-library.json
fixtures/module-libraries/pv-aggregate-pressure.module-library.json
fixtures/modules/attack/
fixtures/modules/density/
fixtures/modules/silence/
fixtures/modules/sustain/
fixtures/modules/residue/
fixtures/modules/aggregate-pressure/
source/audio/Analyzer*Detector.*
source/audio/AnalyzerAggregatePressure.*
source/core/Analyzer*DetectorFixture.*
source/core/AnalyzerAggregatePressureFixture.*
source/core/AnalyzerVisibleCatalog.*
source/core/RuntimeRegistrySynthetic.cpp
source/app/Main.*
source/render/OpenGLShaderPreview.cpp
CMakeLists.txt
tests/Analyzer*DetectorTests.cpp
tests/AnalyzerVisibleCatalogTests.cpp
tests/RuntimeRegistryTests.cpp
```

Closed H cleanup files, do not edit for PV:

```text
docs/superpowers/specs/2026-05-24-post-c-three-layer-cleanup.md
source/core/RuntimeRegistry.*
source/storage/StorageContract.*
source/ui/ImGuiSmokeOverlay.*
```

Closed R routing files, do not edit for PV:

```text
docs/superpowers/specs/2026-05-24-r-runtime-render-backbone-roadmap.md
docs/superpowers/plans/2026-05-24-r-segment-implementation.md
```

## Next Handoff Sentence

Open this master plan first. Active lane is `None` after PV-B1.1 node surface geometry closure. Next selectable lanes include MIDI/shader mapping, live callback-buffer runtime, browser polish, or a new analyzer aggregate family, but each needs a fresh active spec before code. Do not add analyzer DSP, MIDI mapping, shader uniform mapping, browser polish, live callback-buffer runtime, Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, TiXL runtime work, relocation note, flow-runner files, `scripts/`, `tests/test_myworld_flow.py`, or `AGENTS.md` without selecting that lane first.

## Next Master-Plan Maintenance

After the next lane finishes:

1. Re-check `git status -sb`.
2. Read the lane closure spec or create it if missing.
3. Update this file:
   - move the active lane from `in progress` to `closed` or `blocked`;
   - record proof commands;
   - set the next active lane explicitly.
4. Do not start MIDI/shader mapping, live callback-buffer runtime, TiXL, browser polish, or another analyzer family until a new active lane row is updated.
