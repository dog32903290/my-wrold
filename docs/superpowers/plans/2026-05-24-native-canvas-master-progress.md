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

## Lane Selection Gate

Before any new implementation lane starts, check this gate. If one item fails, do not start the lane; update the plan or close the previous lane first.

```text
1. `git status -sb` has been checked for active or uncommitted work.
2. This master plan marks exactly one active lane, or explicitly marks `None`.
3. Any dirty files are either owned by the selected lane or listed in Session Safety.
4. The selected lane has a one-line proof before code starts.
5. The verification commands are named before code starts.
6. The previous lane has closure evidence or an explicit parked reason.
7. Tempting adjacent work is written into parked scope instead of entering the lane silently.
```

Use this gate to prevent visual, Metal, live IO, TiXL parity, cleanup, and AI-worker work from being opened just because a gap is visible.

## Source Hierarchy

| Rank | Document type | Role |
| --- | --- | --- |
| 1 | This master progress plan | Current status, active lane, sequencing, conflicts |
| 2 | Closure specs | Proven evidence and parked scope for a finished lane |
| 3 | Active slice spec | Contract for the lane currently being built |
| 4 | Active implementation plan | Step-by-step construction for the current slice |
| 5 | Old implementation plans | Historical records only |

## Current Snapshot

Date: 2026-05-25 23:53 Asia/Taipei.

Branch:

```text
codex/work5-work-segment-closure
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
WORK5 work segment closure (current local lane)
ae04c7b Add WORK4 app status wording
f8a36f0 Add WORK3 work diagnostics report
8b334ac Add WORK2 active work resolver
52fb50a Add WORK1 active work lifecycle status
5cb4880 Close APP workbench session spine
6ffe589 Add APP6 stable workbench proof runner
e22713b Add APP5 stable workbench proof alias
7e5b6ca Add APP4 active work source truth
e479cc9 Correct APP3 master plan status
1c2323e Add APP3 workbench app controller spine
7b4fb0c Add APP2 workbench open status spine
7a56cc4 Add APP1 workbench session spine
fa6a762 Add G1 graph IO mapping proof artifact
5d11ae3 Add graph IO mapping foundation
3d8653c Add portable JUCE build bootstrap
08da80b Harden OSC parsing and proof output
0b0cfac Close audio visual proof segment
09f53a5 Record app visual reaction proof readback
9bfbf58 Capture visual reaction frames in shader proof
380e071 Add visual reaction proof artifact
318b100 Bridge live IO uniforms into shader preview
2bd8e8a Add shader preview smoke read
1bd20d5 Add shader preview input bridge contract
6f3f995 Add shader uniform proof artifact
aa6c6cf Shape shader uniform evidence JSON
ea6a883 Add shader uniform control evidence
b174f2e Add live IO operator status evidence
7db3233 Add live IO operator picker foundation
888f26c Add OSC receiver lifecycle to live IO controller
3999703 Add realtime delivery backpressure telemetry
dca3f31 Show realtime delivery in live IO status
9113fee Add realtime delivery status telemetry
0d31a2e Add realtime live IO snapshot delivery
5895944 Add broader live IO MIDI operators
f52a3a1 Add live IO OSC receive spine
5b35fcc Add external live IO OSC target proof
5d57a75 Add live IO OSC target preferences
afd7b18 Add arbitrary live IO MIDI teach
e71d69c Add live IO preference persistence
85bc645 Extract live IO app controller
ba7d142 Add live IO MIDI teach
663e440 Add live IO send mode preferences
96075a0 Add live IO status indicator
329f878 Add live IO app timer OSC loopback proof
b4da97f Add live IO app timer MIDI proof
2b892f5 Add control-rate live IO dispatcher proof
d28e83c Add controlled MIDI send proof
6eefbd4 Add MIDI output inventory proof
93b1f8f Add OSC loopback live IO proof
2728868 Add live IO send boundary proof
32bf75b Add live IO bus proof
e1197e5 Close variation blend and thumbnail proof
f11bff3 Add variation CRUD commands
6872983 Add parameter metadata contract
e7037f5 Add presets and snapshots foundation
8660bd3 Reuse proof support in PV runners
4f86e70 Reuse proof support in A1 and C2
30845a7 Extract proof run support helpers
3c0e471 Extract V1 shader proof artifacts
caa9372 Extract A1 audio proof runner
562860d Extract C2 storage proof runner
ca90a0e Extract C3 save work proof runner
a4991ff Extract C4 AI worker proof runner
adaeb53 Extract C6 repair proof runner
248e41f Extract C5 module publish proof runner
8006397 Extract C6 analyzer proof runner
47703f9 Extract PV-B1 analyzer proof runner
04f33d5 Add PV analyzer detector proofs
5cc6399 Close R runtime render backbone
ebeab9f Add R2 headless render runtime
```

## Session Safety

As of 2026-05-25 23:53 Asia/Taipei, APP1-APP7 and WORK1-WORK4 are closed locally and not pushed. WORK5 work segment closure is closed locally on `codex/work5-work-segment-closure`; do not push unless explicitly requested.

```text
WORK5 owned files:
- docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md
- docs/superpowers/specs/2026-05-25-work5-work-segment-closure.md
```
Do not add active-work preparation, workbench UI panels, canvas gestures, mapping editor, graph UI node surface, runtime cook loop, save mutation, direct realtime MIDI/OSC send, shader preview live binding expansion, OpenGL backend expansion, Metal, or analyzer DSP in WORK5.

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
| C5 proof harness extraction | closed | `docs/superpowers/specs/2026-05-25-c5-proof-harness-extraction.md`; `C5ModulePublishProofRunner` owns direct, AI worker, and visible module publish proof orchestration; focused test, app build, and all three C5 CLI proofs passed | Do not reopen C5 publish semantics; C2-C4/C6 repair/V1 proof harness cleanup remain separate selected lanes |
| C6 AI repair-loop proof harness extraction | closed | `docs/superpowers/specs/2026-05-25-c6-ai-repair-loop-proof-harness-extraction.md`; `C6AIRepairLoopProofRunner` owns repair plan, fixture lookup, repair execution, and report writing; focused test, app build, and C6 repair CLI proof passed | Do not reopen AI repair semantics; C2-C4/V1 proof harness cleanup remain separate selected lanes |
| C4 AI worker save-work proof harness extraction | closed | `docs/superpowers/specs/2026-05-25-c4-ai-worker-save-work-proof-harness-extraction.md`; `C4AIWorkerSaveWorkProofRunner` owns fixture copy, move/save AI command proof, storage reload, layout evidence, and report writing; focused test, app build, and C4 CLI proof passed | Do not reopen C4 AI command semantics; C2-C3/V1 proof harness cleanup remain separate selected lanes |
| C3 save-work proof harness extraction | closed | `docs/superpowers/specs/2026-05-25-c3-save-work-proof-harness-extraction.md`; `C3SaveWorkProofRunner` owns fixture copy, move/save command proof, storage reload, layout evidence, and report writing; focused test, app build, and C3 CLI proof passed | Do not reopen C3 storage command semantics; C2/V1 proof harness cleanup remain separate selected lanes |
| C2 storage proof harness extraction | closed | `docs/superpowers/specs/2026-05-25-c2-storage-proof-harness-extraction.md`; `C2StorageProofRunner` owns fixture lookup, patch save/reload, layout evidence, and report writing; focused test, app build, and C2 CLI proof passed | Do not reopen C2 storage semantics; V1/A1 proof harness cleanup remain separate selected lanes |
| A1 audio proof harness extraction | closed | `docs/superpowers/specs/2026-05-25-a1-audio-proof-harness-extraction.md`; `A1AudioProofRunner` owns runtime registry lookup, synthetic runtime execution, audio stats, compound, runtime execution, and bridge artifact writing; focused test, app build, and A1 CLI proof passed | Do not reopen A1 analyzer/runtime semantics; V1 proof harness cleanup remains separate selected lane |
| V1 shader proof artifact extraction | closed | `docs/superpowers/specs/2026-05-25-v1-shader-proof-artifact-extraction.md`; `V1ShaderProofArtifacts` owns V1 proof artifact path/report/PNG writing while `OpenGLShaderPreview` keeps live frame capture; focused test, app build, and V1 CLI proof passed | Do not treat this as headless V1; RenderBackend/headless visual work remains a separate selected lane |
| ProofRunSupport C3/C4 cleanup | closed | `docs/superpowers/specs/2026-05-25-proof-run-support-c3-c4.md`; shared proof text/directory/candidate/copy primitives now serve C3/C4 runners; focused tests, app build, C3/C4 CLI proofs, and full `ctest` 51/51 passed | Do not centralize all proof runner logic; migrate remaining helpers only as selected small slices |
| ProofRunSupport A1/C2 cleanup | closed | `docs/superpowers/specs/2026-05-25-proof-run-support-a1-c2.md`; A1/C2 now share proof text/directory/candidate primitives with `ProofRunSupport`; focused tests, app build, and A1/C2 CLI proofs passed | Do not centralize all proof runner logic; C5/C6/PV cleanup remains separate selected slices |
| ProofRunSupport PV/PV-B1 cleanup | closed | `docs/superpowers/specs/2026-05-25-proof-run-support-pv-pvb1.md`; PV detector and PV-B1 runners now share proof text/directory/candidate primitives with `ProofRunSupport`; focused tests, app build, all PV detector CLI proofs, and PV-B1 CLI proof passed | Do not centralize all proof runner logic; C5/C6 helper cleanup remains separate selected slices |
| ProofRunSupport C5/C6 cleanup | closed | `docs/superpowers/specs/2026-05-25-proof-run-support-c5-c6.md`; C5 publish and C6 analyzer/repair runners now share proof text/directory/candidate primitives with `ProofRunSupport`; focused tests, app build, all C5/C6 CLI proofs, and full `ctest` 51/51 passed | Do not turn `ProofRunSupport` into a generic proof runner; future cleanup should target adapter duplication |
| MainComponent startup proof adapter | closed | `docs/superpowers/specs/2026-05-25-startup-proof-adapter-cleanup.md`; `StartupProofOptions` and task mapping now own CLI startup proof selection/order/delays; focused test, app build, and C2 CLI proof passed | Do not move proof runner semantics into startup adapter; next cleanup is proof status facade duplication |
| MainComponent proof status facade | closed | `docs/superpowers/specs/2026-05-25-proof-status-facade-cleanup.md`; `finishProofDump()` owns proof result status text and quit-after-dump handling for file-based proof runners; app build and PV attack CLI proof passed | Do not move request construction or proof semantics into the UI facade; next cleanup is path policy extraction |
| App path policy cleanup | closed | `docs/superpowers/specs/2026-05-25-app-path-policy-cleanup.md`; `AppPaths` owns project root/debug folder/active manifest/candidate roots policy; app build plus C2 and C5 visible CLI proofs passed | Do not add proof runner schemas to path policy; next cleanup is active work save/publish adapter |
| Active work service cleanup | closed | `docs/superpowers/specs/2026-05-25-active-work-service-cleanup.md`; `ActiveWorkService` owns default active work preparation plus visible save/publish request construction; focused service/storage tests and app build passed | Treat further MainComponent cleanup as a fresh selected lane |
| P-TAX1 TiXL taxonomy fixture | closed | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`; `fixtures/tixl-witness/operator-browser-taxonomy.json` plus `tixl_taxonomy_fixture` prove root paths, selected drilldown paths, source commit/counts, hidden exclusions, and alias-not-root rules | P-SEARCH1 is now closed; do not start UI browser polish without selecting a fresh lane |
| P-SEARCH1 TiXL browser/search/compatible create | closed | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`; `NodeSpecBrowser` plus `node_spec_browser` prove search, aliases, saved type preservation, and compatible-create candidate filtering | Next TiXL interaction lane is P-OPS1; do not start UI browser polish without a fresh selected lane |
| P-OPS1A richer graph operation trace | closed | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`; `InteractionContract` macro helpers plus `t3_t5_commands` and `interaction_traces` prove reconnect input/output end and split-edge-create-node as one undoable command | Remaining P-OPS1 work is hidden input/multi-input/drag-existing-node/snap/unsnap/shake, or select P-OUT1 |
| P-OPS1B richer graph operation continuation | closed | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`; command helpers plus `t3_t5_commands` and `interaction_traces` prove hidden input, ordered input insert, existing-node edge insert, snap/unsnap, and shake disconnect | Next requested lane is P-OUT1 output pinning |
| P-OUT1 output pinning | closed | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`; `OutputViewState` plus `output_view_state`, PatchDocument/saveWork roundtrip, and visible workspace state prove selection-following output and pin persistence | Next selectable parity lane is P-PARAM1 or P-TIME1; keep output slots/render toolbar parked |
| P-PARAM1A parameter row states | closed | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`; `ParameterRowState` plus `parameter_row_state`, command/storage roundtrip, and visible inspector state prove default/manual/connected/animated/reset rows | Next selectable parity lane is P-PARAM1B typed controls or P-TIME1; keep presets/snapshots parked |
| P-PARAM1B typed parameter controls | closed | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`; `ParameterControl` plus `parameter_controls`, PatchDocument roundtrip, and visible inspector typed controls prove typed edit normalization before `set_param` | Next requested lane is P-TIME1 bars-native timeline |
| P-TIME1 bars-native timeline model | closed | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`; `TimelineState` plus `timeline_state`, command undo/redo, PatchDocument roundtrip, and saveWork roundtrip prove bars as canonical and seconds/frames as derived views | No active parity lane selected after P-TIME1 |
| P-TIME2 transport playback controls | closed | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`; `TimelineState` transport fields plus `transport_controls`, bottom transport UI, PatchDocument roundtrip, and saveWork roundtrip prove play/pause/stop/step/reverse/loop controls | Next requested lane is P-VAR1 presets/snapshots foundation |
| P-VAR1 presets/snapshots foundation | closed | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`; `VariationState` plus `variation_state`, command create/apply, skip reasons, PatchDocument roundtrip, saveWork roundtrip, and left-rail state consumption prove presets/snapshots foundation | No active parity lane selected after P-VAR1 |
| P-PARAM007 parameter metadata | closed | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`; `ParamSpec` metadata plus `parameter_metadata` prove groups, descriptions, value-based relevance filtering, inspector grouping/tooltips, and NodeSpec-owned preset exclusion | Next selectable parity lane is VAR-004 variation canvas CRUD |
| P-VAR004 variation canvas CRUD foundation | closed | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`; `InteractionContract` variation CRUD verbs plus `variation_state` prove rename/delete/move for presets and snapshots with undo/redo and storage preservation | No active parity lane after P-VAR004 |
| P-VAR005 variation thumbnail selection hit-test | closed | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`; `VariationThumbnailLayout`, thumbnail hit-test, `selectVariation`, and left-rail thumbnail UI prove preset/snapshot thumbnails can be selected without applying/blending | VAR-005 hover preview / Alt blend is now closed separately |
| VAR-005 hover preview / Alt blend | closed | `VariationPreviewReport`, `previewVariationBlend`, `commitVariationBlend`, and left-rail hover/Alt-click prove deterministic non-mutating preview plus one undoable blend command | Child enable UI, symbol-browser preset creation, richer type-specific blend rules remain parked |
| R-TN1 real thumbnail headless proof | closed | `HeadlessRenderRuntime` writes `thumbnail.png` and `thumbnail_stats.json` for `image.constant -> output.texture_summary`; `headless_render_runtime`, app build, and full `ctest` passed | Full render/export window, render process states, render queue, and UI render settings remain parked |
| P-LIVE1 live IO bus foundation | closed | `LiveIOBus` plus `LiveIOProofRunner` map loaded `compound.loudness` public output to MIDI CC, OSC float, and shader uniform target events; app CLI `--dump-live-io-proof-and-exit` writes `live_io_report.json` | Real MIDI/OSC device IO, teach mode, UDP send/receive, realtime callback wiring, and live UI mapping remain parked |
| P-LIVE1.2 live IO send boundary | closed | `docs/superpowers/specs/2026-05-25-p-live1-2-live-io-send-boundary.md`; `LiveIOSendAdapter` turns `LiveIOBus` MIDI/OSC events into dry-run send actions and app proof writes `live_io_send_report.json`; `ctest` 65/65 | Real MIDI/UDP send, device scan, realtime callback wiring, teach mode, and live UI mapping remain parked |
| P-LIVE1.3 controlled OSC loopback proof | closed | `docs/superpowers/specs/2026-05-25-p-live1-3-osc-loopback-proof.md`; `LiveIOSendAdapter` sends one OSC float packet to a controlled localhost receiver and app proof writes `live_io_osc_loopback_report.json` | MIDI device send, external UDP target, always-on OSC server, realtime callback wiring, teach mode, and live UI mapping remain parked |
| P-LIVE1.4 MIDI output inventory / route report | closed | `docs/superpowers/specs/2026-05-25-p-live1-4-midi-output-inventory.md`; app proof writes `live_io_midi_inventory_report.json` with MIDI output device list plus selected/unavailable route reports | Controlled MIDI open/send is closed in P-LIVE1.5; realtime callback wiring, teach mode, and live UI mapping remain parked |
| P-LIVE1.5 controlled MIDI open/send proof | closed | `docs/superpowers/specs/2026-05-25-p-live1-5-controlled-midi-send.md`; `LiveIOMidiSendProof` plus `LiveIOProofRunner` open the selected MIDI output and send one CC proof message; app proof writes `live_io_midi_send_report.json` | MIDI teach mode, realtime callback wiring, live UI mapping, broader MIDI output operators, and external OSC/UDP targets remain parked |
| P-LIVE2 control-rate live IO dispatcher proof | closed | `docs/superpowers/specs/2026-05-25-p-live2-control-rate-dispatcher.md`; `LiveIOControlDispatcher` rate-limits frame dispatch, calls injected MIDI/OSC control sinks, skips shader uniforms, and app proof writes `live_io_control_dispatch_report.json` | Realtime callback delivery, live UI mapping, MIDI teach mode, external OSC/UDP targets, and broader MIDI output operators remain parked |
| P-LIVE3 control-rate analyzer snapshot pump | closed | `docs/superpowers/specs/2026-05-25-p-live3-control-pump.md`; `LiveIOControlPump` turns analyzer snapshots into control frames, tick-rate-limits inactive/fast ticks, feeds `LiveIOControlDispatcher`, and app proof writes `live_io_control_pump_report.json` | App timer dry-run wiring is closed in P-LIVE4; realtime callback delivery, live UI mapping, MIDI teach mode, and external OSC targets remain parked |
| P-LIVE4 app timer dry-run wiring | closed | `docs/superpowers/specs/2026-05-25-p-live4-app-timer-dry-run.md`; `LiveIOControlTimer` records app-timer-driven dry-run state from analyzer snapshots, `MainComponent::timerCallback()` reaches it through `updateAudioMeters()`, and status text exposes dry MIDI/OSC counts | Send mode gate is closed in P-LIVE5; live UI indicator is closed in P-LIVE8; MIDI teach is closed in P-LIVE10; realtime callback delivery and broader output operators remain parked |
| P-LIVE5 app timer send mode gate | closed | `docs/superpowers/specs/2026-05-25-p-live5-app-timer-send-gate.md`; `LiveIOControlTimerConfig` adds `dryRun` / `controlledSend`, dry-run never calls provided senders, controlled-send calls injected MIDI/OSC senders, and app timer carries the gate while defaulting to dry-run | Controlled MIDI app timer proof is closed in P-LIVE6; OSC app timer loopback is closed in P-LIVE7; live UI indicator is closed in P-LIVE8; UI/preferences mode switch is closed in P-LIVE9; MIDI teach is closed in P-LIVE10; realtime callback delivery remains parked |
| P-LIVE6 controlled app timer MIDI proof | closed | `docs/superpowers/specs/2026-05-25-p-live6-app-timer-midi-proof.md`; `LiveIOProofRunner` opt-in proof drives `LiveIOControlTimerConfig(sendMode: controlledSend)` with MIDI enabled/OSC disabled and writes `live_io_app_timer_midi_report.json` showing one controlled MIDI send | OSC app timer loopback is closed in P-LIVE7; live UI indicator is closed in P-LIVE8; normal app mode switching is closed in P-LIVE9; MIDI teach is closed in P-LIVE10; realtime callback delivery remains parked |
| P-LIVE7 controlled app timer OSC loopback | closed | `docs/superpowers/specs/2026-05-25-p-live7-app-timer-osc-loopback.md`; `LiveIOProofRunner` opt-in proof drives the same `LiveIOControlTimer` body with MIDI disabled/OSC enabled, sends through `LiveIOSendAdapter`, receives localhost OSC, and writes `live_io_app_timer_osc_loopback_report.json` | Live UI indicator is closed in P-LIVE8; normal app mode switching is closed in P-LIVE9; MIDI teach is closed in P-LIVE10; external OSC targets/receive nodes and realtime callback delivery remain parked |
| P-LIVE8 live UI indicator | closed | `docs/superpowers/specs/2026-05-25-p-live8-live-ui-indicator.md`; `LiveIOStatusIndicator` turns real `LiveIOControlTimerState` plus send mode into compact text/tone/counts, and `MainComponent` shows it in a dedicated status label; `ctest` 71/71 | Send mode preferences are closed in P-LIVE9; MIDI teach is closed in P-LIVE10; external OSC targets/receive nodes, broader MIDI output operators, and realtime callback delivery remain parked |
| P-LIVE9 send mode preferences | closed | `docs/superpowers/specs/2026-05-25-p-live9-send-mode-preferences.md`; `PerformancePreferences.liveIO` defaults to dry-run, sanitizes invalid send modes back to dry-run, `PreferencesPanel` exposes dry-run/controlled-send, and `MainComponent` maps the sanitized preference into `LiveIOControlTimerConfig.sendMode`; `ctest` 71/71 | MIDI teach is closed in P-LIVE10; OSC target preferences, external OSC receive nodes/server, broader MIDI output operators, and realtime callback delivery remain parked |
| P-LIVE10 MIDI teach | closed | `docs/superpowers/specs/2026-05-25-p-live10-midi-teach.md`; `LiveIOMidiTeach` arms loudness/map CC targets, learns the next incoming MIDI CC, disarms, and `MainComponent` temporarily routes MIDI input callbacks to PreferencesPanel learned channel/CC updates without UI work in the callback; `ctest` 72/72 | MIDI input selector/preference, arbitrary binding teach, preference persistence, OSC target preferences, external OSC receive nodes/server, and realtime callback delivery remain parked |
| P-LIVE-H1 live IO app controller cleanup | closed | `docs/superpowers/specs/2026-05-25-p-live-h1-app-controller-cleanup.md`; `LiveIOAppController` owns app-timer live IO state and MIDI teach state, while `MainComponent` keeps JUCE UI/device registration; focused controller test, app build, and `ctest` 73/73 passed | MIDI input selector/preference, arbitrary binding teach, preference persistence, OSC target preferences, external OSC receive nodes/server, and realtime callback delivery remain parked |
| P-LIVE11 MIDI input selector/preference | closed | `docs/superpowers/specs/2026-05-25-p-live11-midi-input-selector-preference.md`; `PerformancePreferences` stores selected MIDI input, `PreferencesPanel` exposes input selection, and `MainComponent` narrows MIDI teach listening to the selected input while preserving all-input fallback; focused tests, app build, and `ctest` 73/73 passed | Arbitrary binding teach, preference disk persistence, OSC target preferences, external OSC receive nodes/server, and realtime callback delivery remain parked |
| P-LIVE12 preference disk persistence | closed | `docs/superpowers/specs/2026-05-25-p-live12-preference-disk-persistence.md`; `savePerformancePreferences()` / `loadPerformancePreferences()` roundtrip the full app preference snapshot, `MainComponent` loads it on startup and saves UI preference changes; focused tests, app build, and `ctest` 73/73 passed | Arbitrary binding teach, OSC target preferences, external OSC receive nodes/server, and realtime callback delivery remain parked |
| P-LIVE13 arbitrary binding MIDI teach | closed | `docs/superpowers/specs/2026-05-25-p-live13-arbitrary-binding-midi-teach.md`; `armLiveIOMidiTeachForBinding()` learns a CC for an arbitrary binding id and `applyLiveIOMidiTeachToBindings()` updates only the matching `midi.cc` binding; focused tests, app build, and `ctest` 73/73 passed | UI binding chooser, OSC target preferences, external OSC receive nodes/server, broader MIDI output operators, and realtime callback delivery remain parked |
| P-LIVE14 OSC target preferences | closed | `docs/superpowers/specs/2026-05-25-p-live14-osc-target-preferences.md`; `LiveIOPreferences` stores OSC host/port/loudness address, save/load roundtrips them, and app OSC binding uses the configured address while external send remains parked; focused tests, app build, and `ctest` 73/73 passed | UI fields, external OSC send proof, external OSC receive nodes/server, broader MIDI output operators, and realtime callback delivery remain parked |
| P-LIVE15 external OSC target send proof | closed | `docs/superpowers/specs/2026-05-25-p-live15-external-osc-target-send-proof.md`; controlled app timer carries OSC host/port/address into an injected sender, proving external target routing without adding a server or realtime callback delivery; focused tests, app build, and `ctest` 73/73 passed | Always-on OSC receive nodes/server, broader MIDI output operators, and realtime callback delivery remain parked |
| P-LIVE16 OSC receive spine | closed | `docs/superpowers/specs/2026-05-25-p-live16-osc-receive-spine.md`; `LiveIOOscReceiver` opens/polls/closes a controlled UDP receiver, decodes OSC float datagrams, and emits matching `LiveIOValueFrame` values; focused tests, app build, and `ctest` 74/74 passed | UI/server lifecycle, broader MIDI output operators, and realtime callback delivery remain parked |
| P-LIVE17 broader MIDI output operators | closed | `docs/superpowers/specs/2026-05-25-p-live17-broader-midi-output-operators.md`; `LiveIOBus` can emit `midi.cc` and `midi.note_on`, `LiveIOMidiMessage` reports byte-level CC/note-on output, and the control-rate dispatcher sends both through the existing injected MIDI sender; focused tests and app build passed | UI operator picker, note-off/program-change/pitch-bend, and realtime callback delivery remain parked |
| P-LIVE18 realtime callback delivery | closed | `docs/superpowers/specs/2026-05-25-p-live18-realtime-callback-delivery.md`; `AudioInputAnalyzer` publishes callback snapshots into `AudioRealtimeDelivery`, `MainComponent` consumes completed sequences on the app timer, and `LiveIOControlTimer` remains outside the callback; focused test and app build passed | Direct realtime MIDI/OSC send, multi-slot backpressure telemetry, and delivery UI remain parked |
| P-LIVE19 realtime delivery status | closed | `docs/superpowers/specs/2026-05-25-p-live19-realtime-delivery-status.md`; `AudioRealtimeDeliveryResult` reports empty/writing/repeated/delivered state, completed sequence, and coarse single-slot dropped snapshot count, and the app timer shows it in the audio status label; focused test and app build passed | Multi-slot queue/backpressure telemetry, standalone delivery UI widget, and direct realtime MIDI/OSC send remain parked |
| P-LIVE20 live IO realtime indicator | closed | `docs/superpowers/specs/2026-05-25-p-live20-live-io-realtime-indicator.md`; app-side `AudioRealtimeDeliveryResult` telemetry flows into existing live IO status indicator text/json and the live IO label; focused test and app build passed | Multi-slot queue/backpressure telemetry, standalone delivery UI widget, and direct realtime MIDI/OSC send remain parked |
| P-LIVE21 realtime delivery backpressure | closed | `docs/superpowers/specs/2026-05-25-p-live21-realtime-delivery-backpressure.md`; `AudioRealtimeDelivery` now uses four fixed slots and reports skipped vs overwritten snapshots separately into the existing live IO indicator; focused tests and app build passed | Sequential catch-up API, dynamic queues, standalone delivery UI widget, and direct realtime MIDI/OSC send remain parked |
| P-LIVE22 OSC server lifecycle | closed | `docs/superpowers/specs/2026-05-25-p-live22-osc-server-lifecycle.md`; `LiveIOAppController` owns the app/control-side OSC receiver lifecycle, polls once on app timer tick, and exposes matching incoming OSC float values as `LiveIOValueFrame` evidence; focused tests and app build passed | Graph OSC input node, separate receive UI fields, dynamic OSC address routing, and direct realtime callback send/receive remain parked |
| P-LIVE23 UI operator picker foundation | closed | `docs/superpowers/specs/2026-05-25-p-live23-ui-operator-picker-foundation.md`; live IO preferences now store one selected output operator, PreferencesPanel exposes it, and `LiveIOAppController` builds the primary loudness binding for `midi.cc`, `midi.note_on`, `osc.float`, or `shader.uniform`; focused tests and app build passed | Full mapping editor, multiple simultaneous user-selected bindings, graph IO node, dynamic OSC scanning, and new MIDI operator kinds remain parked |
| P-LIVE24 live IO operator status evidence | closed | `docs/superpowers/specs/2026-05-25-p-live24-live-io-operator-status-evidence.md`; `LiveIOStatusIndicatorState` carries the selected output operator, app controller status text exposes `op <kind>`, and app-timer proof reports record `outputOperator`; focused tests passed | Full mapping editor, multiple simultaneous user-selected bindings, graph IO node, dynamic OSC scanning, and new MIDI operator kinds remain parked |
| P-LIVE25 shader uniform control evidence | closed | `docs/superpowers/specs/2026-05-25-p-live25-shader-uniform-control-evidence.md`; `shader.uniform` control dispatch now records `u_loudness` evidence, timer/status state expose the latest uniform value/sample counter, and app-timer proof JSON writes it back | Shader preview live binding, standalone uniform artifact, graph IO node, and full mapping editor remain parked |
| P-LIVE26 shader uniform evidence JSON shape | closed | `docs/superpowers/specs/2026-05-25-p-live26-shader-uniform-evidence-json-shape.md`; status JSON and app-timer proof JSON now include a nested `shaderUniformEvidence` object with source, binding, uniform, value, and sample counter | Standalone uniform artifact, shader preview input bridge, shader preview smoke read, and live binding remain parked |
| P-LIVE27 shader uniform proof artifact | closed | `docs/superpowers/specs/2026-05-25-p-live27-shader-uniform-proof-artifact.md`; live IO proof runner now writes `live_io_shader_uniform_report.json` as an independent artifact containing `u_loudness` evidence | Shader preview input bridge, shader preview smoke read, and live binding remain parked |
| P-LIVE28 shader preview input bridge contract | closed | `docs/superpowers/specs/2026-05-25-p-live28-shader-preview-input-bridge-contract.md`; `ShaderPreviewInputBridge` defines a JUCE-free uniform snapshot contract and JSON shape for preview input without wiring OpenGL live binding | Shader preview smoke-read, OpenGL backend wiring, and live binding remain parked |
| P-LIVE29 shader preview smoke-read | closed | `docs/superpowers/specs/2026-05-25-p-live29-shader-preview-smoke-read.md`; `ShaderPreviewInputBridge` can convert a `u_loudness` snapshot into `RenderFrameInput`, and a fake render backend reads the value in a smoke test | OpenGL backend wiring, live binding, and visual reaction remain parked |
| P-LIVE30 A1 to V1 handoff closure | closed | `docs/superpowers/specs/2026-05-25-p-live30-a1-to-v1-handoff-closure.md`; closes the P-LIVE lane at app/control-rate handoff: loudness now has live IO evidence, standalone uniform artifact, preview input bridge, and render-side smoke-read | Next work should leave P-LIVE naming and open an `AV1 audio-to-visual bridge` lane for real preview reaction |
| AV1 audio-to-visual bridge | closed | `docs/superpowers/specs/2026-05-25-av1-audio-to-visual-bridge.md`; `MainComponent::tickLiveIOControl()` feeds `shader.uniform` evidence into `OpenGLShaderPreview::setInputSnapshot()`, and preview converts the snapshot through `ShaderPreviewInputBridge` before updating its render loudness input | Pixel-diff visual proof, graph node mapping, OpenGL uniform list expansion, and Metal remain parked |
| AV2 visual reaction proof | closed | `docs/superpowers/specs/2026-05-25-av2-visual-reaction-proof.md`; V1 proof artifacts can now write `visual_reaction.json` comparing quiet/loud frames with changed pixel count and mean absolute pixel delta | Real OpenGL offscreen capture pair, shader preview live binding, graph node mapping, and Metal remain parked |
| AV3 real OpenGL capture pair | closed | `docs/superpowers/specs/2026-05-25-av3-real-opengl-capture-pair.md`; shader preview proof dump now renders quiet/loud frame pairs through the active OpenGL preview backend and passes them to `V1ShaderProofArtifacts` for `visual_reaction.json` | Headless/offscreen GL harness, graph node mapping, full mapping editor, direct realtime callback send, and Metal remain parked |
| AV4 app dump visual reaction readback | closed | `docs/superpowers/specs/2026-05-25-av4-app-dump-visual-reaction-readback.md`; `--dump-proof-and-exit` now produces and reads back `debug/v1-shader-proof/visual_reaction.json` with `ok: true`, `status: changed`, `changedPixels: 4147429`, and `meanAbsDelta: 0.071055` | Headless/offscreen GL harness, graph node mapping, full mapping editor, direct realtime callback send, and Metal remain parked |
| AV5 audio visual segment closure | closed | `docs/superpowers/specs/2026-05-25-av5-audio-visual-segment-closure.md`; closes the AV segment from live IO shader uniform evidence through preview loudness input, quiet/loud OpenGL capture, V1 artifact writing, and app dump readback | Headless/offscreen GL harness, user-facing graph IO mapping, full mapping editor, direct realtime callback send, Metal, and visual polish remain parked |
| Gemini risk triage hardening | closed | `docs/superpowers/specs/2026-05-25-gemini-risk-triage-hardening.md`; OSC proof readback now uses bounded decoder, malformed OSC tests cover missing null/truncated payloads, UDP receive buffers no longer truncate at 1024/256 bytes, OpenGL proof artifact writes run off the render thread, and source/CMake no longer contain user-specific hardcoded paths | Full OSC bundle parsing, JSON report writer replacement, non-loopback send UI/lifecycle, dedicated proof job system, and CI JUCE bootstrap remain parked |
| BUILD1 portable JUCE bootstrap | closed | `docs/superpowers/specs/2026-05-25-build1-portable-juce-bootstrap.md`; CMake can use explicit/local JUCE or fetch pinned JUCE 8.0.12, GitHub Actions has a macOS CMake workflow, and build docs explain both paths | CI cache tuning, Linux/Windows matrix, package manager integration, and automatic JUCE version bumps remain parked |
| G1 graph IO mapping foundation | closed | `docs/superpowers/specs/2026-05-25-g1-graph-io-mapping-foundation.md`; saved fixture graph loads one `signal.float` source-to-`shader.uniform` target mapping, validates it, generates an existing `LiveIOBus` binding, emits graph IO mapping JSON evidence, and `G1GraphIOMappingProofRunner` writes `graph_io_mapping_report.json`; `ctest` 79/79 passed | Full mapping editor, graph UI node surface, multiple user-authored mappings, broader MIDI/OSC operators, direct realtime send, shader preview live binding expansion, Metal, and analyzer DSP remain parked |
| APP1 workbench session spine | closed | `docs/superpowers/specs/2026-05-25-app1-workbench-session-spine.md`; work fixture + G1 mapping fixture builds a `WorkbenchSession` snapshot with document, graph counts, dirty/save/proof/preview, output/timeline, and mapping validity evidence; `APP1WorkbenchSessionProofRunner` writes `workbench_session_report.json`; app startup proof command writes `debug/app1-workbench-session-proof/workbench_session_report.json`; `ctest` 81/81 passed | Workbench UI panels, canvas gestures, mapping editor, runtime cook loop, save command integration, AI worker visible task surface, shader preview live-binding expansion, Metal, and visual polish remain parked |
| APP2 workbench open/status spine | closed locally | `docs/superpowers/specs/2026-05-25-app2-workbench-open-status-spine.md`; app opens and holds `currentWorkbenchSession` from active work if present, otherwise C2 fixture + G1 mapping; `MainComponent` reads snapshot status; app proof writes `debug/app2-workbench-open-status-proof/workbench_open_status_report.json` with `ok: true`; local commit `7b4fb0c` | Node functionality, canvas UI, mapping editor, runtime cook, save mutation, OpenGL backend expansion, Metal, and visual polish remain parked |
| APP3 workbench app controller spine | closed locally | `docs/superpowers/specs/2026-05-25-app3-workbench-app-controller-spine.md`; `WorkbenchAppController` owns current session snapshot, status text, and APP2 proof request creation; `MainComponent` reads controller state instead of owning the snapshot directly; local commit `1c2323e` | Node functionality, canvas UI, mapping editor, runtime cook, save mutation, OpenGL backend expansion, Metal, and visual polish remain parked |
| APP4 active work source truth | closed locally | `docs/superpowers/specs/2026-05-25-app4-active-work-source-truth.md`; workbench session reports `active-work-opened`, `fixture-fallback-active-missing`, or `active-work-blocked`; app dump proof writes `workSourceStatus` and `activeWorkManifestPath`; local commit `7e5b6ca` | Active-work preparation, save mutation, node functionality, canvas UI, mapping editor, runtime cook, OpenGL backend expansion, Metal, and visual polish remain parked |
| APP5 stable workbench proof alias | closed locally | `docs/superpowers/specs/2026-05-25-app5-stable-workbench-proof-alias.md`; stable `--dump-app-workbench-session-proof-and-exit` writes `debug/app-workbench-session-proof/workbench_open_status_report.json`; legacy APP2 flag remains accepted; local commit `e22713b` | Internal legacy APP2 proof runner names, node functionality, canvas UI, mapping editor, runtime cook, save mutation, OpenGL backend expansion, Metal, and visual polish remain parked |
| APP6 stable workbench proof runner | closed locally | `docs/superpowers/specs/2026-05-25-app6-stable-workbench-proof-runner.md`; `AppWorkbenchSessionProofRunner` owns stable request/result/run API; APP2 proof runner remains as wrapper compatibility; local commit `6ffe589` | Removing legacy APP2 compatibility target/test, node functionality, canvas UI, mapping editor, runtime cook, save mutation, OpenGL backend expansion, Metal, and visual polish remain parked |
| APP7 workbench session spine closure | closed locally | `docs/superpowers/specs/2026-05-25-app7-workbench-session-spine-closure.md`; closes APP1-APP6 as the app workbench session spine and blocks further APP-number drift without a fresh app-shell contract | Future work should select a specific lane name such as WORK, STATUS, PROOF, UI, GRAPH, or RUNTIME |
| WORK1 active work lifecycle status | closed locally | `docs/superpowers/specs/2026-05-25-work1-active-work-lifecycle-status.md`; `WorkProjectLifecycle` owns the six active/fixture/blocked source statuses and `WorkbenchSessionOpenStatus` reads them instead of inline strings; stable app proof passed; `ctest` 86/86 passed | Active-work preparation, save mutation, node functionality, canvas UI, mapping editor, runtime cook, OpenGL backend expansion, Metal, and visual polish remain parked |
| WORK2 active work resolver | closed locally | `docs/superpowers/specs/2026-05-25-work2-active-work-resolver.md`; `WorkProjectResolver` owns active-work file resolution, fixture fallback, PatchDocument load, lifecycle, and error output before `WorkbenchSessionOpenStatus` combines the document with G1 mappings; stable app proof passed; `ctest` 87/87 passed | Active-work preparation, save mutation, node functionality, canvas UI, mapping editor, runtime cook, OpenGL backend expansion, Metal, and visual polish remain parked |
| WORK3 work diagnostics report | closed locally | `docs/superpowers/specs/2026-05-25-work3-work-diagnostics-report.md`; `workDiagnostics` records non-blocking work source diagnostics from resolver through `WorkbenchSessionSnapshot` into stable app proof JSON while `diagnostics` remains the blocking vector; stable app proof passed; `ctest` 87/87 passed | Active-work preparation, save mutation, node functionality, canvas UI, mapping editor, runtime cook, OpenGL backend expansion, Metal, and visual polish remain parked |
| WORK4 app status wording | closed locally | `docs/superpowers/specs/2026-05-25-work4-app-status-wording.md`; `makeWorkbenchSessionStatusText` now includes `source <workSourceStatus>` for ready and blocked status text, and `WorkbenchAppController::statusText()` reads the same source-aware wording; stable app proof passed; `ctest` 87/87 passed | Active-work preparation, save mutation, node functionality, canvas UI, mapping editor, runtime cook, OpenGL backend expansion, Metal, and visual polish remain parked |
| WORK5 work segment closure | closed locally | `docs/superpowers/specs/2026-05-25-work5-work-segment-closure.md`; closes WORK1-WORK4 as the app work-source reporting layer: lifecycle status, active-work resolver, non-blocking work diagnostics, and source-aware app/controller status wording | Future work should leave the WORK prefix and select a fresh lane name by feature surface, such as STATUS, PROOF, UI, GRAPH, RUNTIME, or ACTIVE |
| TiXL parity | ledgered, not main spine | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md` | No active TiXL lane after BUILD1 closure |

## Active Lane Protocol

Only one lane should be marked `in progress` in this file unless the files are disjoint.

Current active lane:

```text
None after WORK5 work segment closure as of 2026-05-25 23:53 Asia/Taipei. Not pushed.

Latest closure:
WORK5 work segment closure closed locally.
Evidence:
- docs/superpowers/specs/2026-05-25-work5-work-segment-closure.md
- docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md

Closed line:
WorkProjectLifecycle
-> WorkProjectResolver
-> WorkbenchSessionSnapshot workDiagnostics
-> makeWorkbenchSessionStatusText source wording
-> stable app workbench proof + MainComponent status label

Latest accepted result:
- git show --quiet --oneline 7a56cc4 confirmed the APP1 local commit exists.
- git branch --contains 7a56cc4 confirmed the current WORK5 lineage contains APP1.
- MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit passed.
- debug/app-workbench-session-proof/workbench_open_status_report.json read back with ok: true, workSourceStatus: fixture-fallback-active-missing, populated workDiagnostics, and diagnostics: [].
- ctest --test-dir build --output-on-failure passed 87/87.
- git diff --check passed.

Previous closure:

WORK4 app status wording closed locally.
Evidence:
- docs/superpowers/specs/2026-05-25-work4-app-status-wording.md
- source/app/WorkbenchSessionOpenStatus.cpp
- tests/WorkbenchSessionOpenStatusTests.cpp
- tests/WorkbenchAppControllerTests.cpp

Closed line:
WorkbenchSessionSnapshot
-> makeWorkbenchSessionStatusText
-> WorkbenchAppController::statusText
-> MainComponent status label

Latest accepted result:
- Red test: cmake --build build --target my_world_workbench_session_open_status_tests my_world_workbench_app_controller_tests && ./build/my_world_workbench_session_open_status_tests failed because status text did not include source fixture-fallback-active-missing.
- cmake --build build --target my_world_workbench_session_open_status_tests my_world_workbench_app_controller_tests passed.
- ./build/my_world_workbench_session_open_status_tests passed.
- ./build/my_world_workbench_app_controller_tests passed.
- cmake --build build --target my-world passed.
- MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit passed and kept stable app proof JSON fields.
- ctest --test-dir build --output-on-failure passed 87/87.
- git diff --check passed.

Previous closure:

WORK3 work diagnostics report closed locally.
Evidence:
- docs/superpowers/specs/2026-05-25-work3-work-diagnostics-report.md
- source/app/WorkProjectResolver.*
- source/app/WorkbenchSession.*
- source/app/WorkbenchSessionOpenStatus.cpp
- tests/WorkProjectResolverTests.cpp
- tests/WorkbenchSessionTests.cpp
- tests/AppWorkbenchSessionProofRunnerTests.cpp

Closed line:
WorkProjectResolver
-> workDiagnostics
-> WorkbenchSessionSnapshot
-> workbench_open_status_report.json

Latest accepted result:
- Red test: cmake --build build --target my_world_work_project_resolver_tests my_world_workbench_session_tests my_world_app_workbench_session_proof_runner_tests failed because WorkProjectResolveResult had no workDiagnostics.
- cmake --build build --target my_world_work_project_resolver_tests my_world_workbench_session_tests my_world_app_workbench_session_proof_runner_tests my_world_workbench_session_open_status_tests passed.
- ./build/my_world_work_project_resolver_tests passed.
- ./build/my_world_workbench_session_tests passed.
- ./build/my_world_app_workbench_session_proof_runner_tests passed.
- ./build/my_world_workbench_session_open_status_tests passed.
- cmake --build build --target my-world passed.
- MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit passed and wrote debug/app-workbench-session-proof/workbench_open_status_report.json with workDiagnostics and diagnostics: [].
- ctest --test-dir build --output-on-failure passed 87/87.
- git diff --check passed.

Previous closure:

WORK2 active work resolver closed locally.
Evidence:
- docs/superpowers/specs/2026-05-25-work2-active-work-resolver.md
- source/app/WorkProjectResolver.*
- source/app/WorkbenchSessionOpenStatus.cpp
- tests/WorkProjectResolverTests.cpp
- CMakeLists.txt

Closed line:
active work path + candidate roots
-> WorkProjectResolver
-> PatchDocument + lifecycle
-> WorkbenchSessionOpenStatus
-> WorkbenchSessionSnapshot

Latest accepted result:
- Red test: cmake -S . -B build && cmake --build build --target my_world_work_project_resolver_tests failed because source/app/WorkProjectResolver.cpp did not exist.
- cmake -S . -B build && cmake --build build --target my_world_work_project_resolver_tests my_world_workbench_session_open_status_tests passed.
- cmake --build build --target my_world_work_project_resolver_tests my_world_work_project_lifecycle_tests my_world_workbench_session_open_status_tests my_world_workbench_app_controller_tests my-world passed.
- ./build/my_world_work_project_resolver_tests passed.
- ./build/my_world_work_project_lifecycle_tests passed.
- ./build/my_world_workbench_session_open_status_tests passed.
- ./build/my_world_workbench_app_controller_tests passed.
- MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit passed and wrote debug/app-workbench-session-proof/workbench_open_status_report.json with ok: true and workSourceStatus: fixture-fallback-active-missing.
- ctest --test-dir build --output-on-failure passed 87/87.
- git diff --check passed.

Previous closure:

WORK1 active work lifecycle status closed locally.
Evidence:
- docs/superpowers/specs/2026-05-25-work1-active-work-lifecycle-status.md
- source/app/WorkProjectLifecycle.*
- source/app/WorkbenchSessionOpenStatus.cpp
- tests/WorkProjectLifecycleTests.cpp
- CMakeLists.txt

Closed line:
active work request
-> WorkProjectLifecycle
-> WorkbenchSessionOpenStatus
-> WorkbenchSessionSnapshot
-> app workbench session proof

Latest accepted result:
- Red test: cmake -S . -B build && cmake --build build --target my_world_work_project_lifecycle_tests failed because source/app/WorkProjectLifecycle.cpp did not exist.
- cmake -S . -B build && cmake --build build --target my_world_work_project_lifecycle_tests my_world_workbench_session_open_status_tests passed.
- cmake --build build --target my_world_work_project_lifecycle_tests my_world_workbench_session_open_status_tests my_world_workbench_app_controller_tests my-world passed.
- ./build/my_world_work_project_lifecycle_tests passed.
- ./build/my_world_workbench_session_open_status_tests passed.
- ./build/my_world_workbench_app_controller_tests passed.
- MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit passed and wrote debug/app-workbench-session-proof/workbench_open_status_report.json with ok: true and workSourceStatus: fixture-fallback-active-missing.
- ctest --test-dir build --output-on-failure passed 86/86.
- git diff --check passed.

Previous closure:

APP7 workbench session spine closure closed locally.
Evidence:
- docs/superpowers/specs/2026-05-25-app7-workbench-session-spine-closure.md
- docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md

Closed line:
APP1-APP6
-> app-held workbench session spine
-> stable app proof command and runner
-> APP prefix closed for this span

Latest accepted result:
- No code changed in APP7.
- APP6 latest verification remains current: ctest --test-dir build --output-on-failure passed 85/85.
- APP6 latest verification remains current: git diff --check passed.

Previous closure:

APP6 stable workbench proof runner closed locally as of 2026-05-25 19:27 Asia/Taipei. Local commit `6ffe589`; not pushed.

APP6 stable workbench proof runner closed locally.
Evidence:
- docs/superpowers/specs/2026-05-25-app6-stable-workbench-proof-runner.md
- source/app/AppWorkbenchSessionProofRunner.*
- source/app/APP2WorkbenchOpenStatusProofRunner.*
- source/app/WorkbenchAppController.*
- source/app/MainComponent.cpp
- tests/AppWorkbenchSessionProofRunnerTests.cpp
- CMakeLists.txt

Closed line:
WorkbenchAppController held snapshot
-> AppWorkbenchSessionProofRunner
-> debug/app-workbench-session-proof/workbench_open_status_report.json

Latest accepted result:
- Red test: cmake --build build --target my_world_app_workbench_session_proof_runner_tests failed because AppWorkbenchSessionProofRunner.h did not exist.
- cmake -S . -B build passed.
- cmake --build build --target my_world_app_workbench_session_proof_runner_tests my_world_app2_workbench_open_status_proof_runner_tests my_world_workbench_app_controller_tests my-world passed.
- ./build/my_world_app_workbench_session_proof_runner_tests passed.
- ./build/my_world_app2_workbench_open_status_proof_runner_tests passed.
- ./build/my_world_workbench_app_controller_tests passed.
- MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit passed and wrote `debug/app-workbench-session-proof/workbench_open_status_report.json` with `ok: true`.
- ctest --test-dir build --output-on-failure passed 85/85.
- git diff --check passed.

Previous closure:

APP5 stable workbench proof alias closed locally as of 2026-05-25 19:22 Asia/Taipei. Local commit `e22713b`; not pushed.

APP5 stable workbench proof alias closed locally.
Evidence:
- docs/superpowers/specs/2026-05-25-app5-stable-workbench-proof-alias.md
- source/app/StartupProof.*
- source/app/Main.cpp
- source/app/MainComponent.*
- source/app/APP2WorkbenchOpenStatusProofRunner.*
- tests/StartupProofTests.cpp

Closed line:
--dump-app-workbench-session-proof-and-exit
or legacy --dump-app2-workbench-open-status-proof-and-exit
-> WorkbenchAppController held snapshot
-> debug/app-workbench-session-proof/workbench_open_status_report.json

Latest accepted result:
- Red test: cmake --build build --target my_world_startup_proof_tests failed because dumpAPPWorkbenchSessionProof and StartupProofTaskId::appWorkbenchSession did not exist.
- cmake --build build --target my_world_startup_proof_tests my_world_app2_workbench_open_status_proof_runner_tests my-world passed.
- ./build/my_world_startup_proof_tests passed.
- ./build/my_world_app2_workbench_open_status_proof_runner_tests passed.
- MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app-workbench-session-proof-and-exit passed and wrote `debug/app-workbench-session-proof/workbench_open_status_report.json` with `ok: true`.
- MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app2-workbench-open-status-proof-and-exit passed as the legacy alias.
- ctest --test-dir build --output-on-failure passed 84/84.
- git diff --check passed.

Previous closure:

APP4 active work source truth closed locally as of 2026-05-25 19:18 Asia/Taipei. Local commit `7e5b6ca`; not pushed.

APP4 active work source truth closed locally.
Evidence:
- docs/superpowers/specs/2026-05-25-app4-active-work-source-truth.md
- source/app/WorkbenchSession.*
- source/app/WorkbenchSessionOpenStatus.*
- tests/WorkbenchSessionOpenStatusTests.cpp

Closed line:
active work manifest path
-> active-work-opened | fixture-fallback-active-missing | active-work-blocked
-> WorkbenchSessionSnapshot
-> workbench_open_status_report.json

Latest accepted result:
- Red test: cmake --build build --target my_world_workbench_session_open_status_tests failed because WorkbenchSessionSnapshot had no workSourceStatus or activeWorkManifestPath.
- cmake --build build --target my_world_workbench_session_open_status_tests my_world_workbench_app_controller_tests my-world passed.
- ./build/my_world_workbench_session_open_status_tests passed.
- ./build/my_world_workbench_app_controller_tests passed.
- MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app2-workbench-open-status-proof-and-exit passed and wrote `debug/app2-workbench-open-status-proof/workbench_open_status_report.json` with `workSourceStatus: fixture-fallback-active-missing`.
- ctest --test-dir build --output-on-failure passed 84/84.
- git diff --check passed.

Previous closure:

APP3 workbench app controller spine closed locally as of 2026-05-25 19:04 Asia/Taipei. Local commit `1c2323e`; not pushed.

APP3 workbench app controller spine closed locally.
Evidence:
- docs/superpowers/specs/2026-05-25-app3-workbench-app-controller-spine.md
- source/app/WorkbenchAppController.*
- source/app/MainComponent.*
- CMakeLists.txt
- tests/WorkbenchAppControllerTests.cpp

Closed line:
WorkbenchSessionOpenStatus
-> WorkbenchAppController::currentSessionSnapshot
-> MainComponent reads controller status
-> APP2 app dump proof reads controller-held snapshot

Latest accepted result:
- Red test: cmake --build build --target my_world_workbench_app_controller_tests failed because WorkbenchAppController.h did not exist.
- cmake -S . -B build passed.
- cmake --build build --target my_world_workbench_app_controller_tests my-world passed.
- ./build/my_world_workbench_app_controller_tests passed.
- MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app2-workbench-open-status-proof-and-exit passed and wrote `debug/app2-workbench-open-status-proof/workbench_open_status_report.json` with `ok: true`.
- ctest --test-dir build --output-on-failure passed 84/84.
- git diff --check passed.

Previous closure:

APP2 workbench open/status spine closed locally as of 2026-05-25 18:50 Asia/Taipei. Local commit `7b4fb0c`; not pushed.

APP2 workbench open/status spine closed locally.
Evidence:
- docs/superpowers/specs/2026-05-25-app2-workbench-open-status-spine.md
- source/app/WorkbenchSession.*
- source/app/WorkbenchSessionOpenStatus.*
- source/app/APP2WorkbenchOpenStatusProofRunner.*
- source/app/Main.cpp
- source/app/MainComponent.*
- source/app/StartupProof.*
- tests/WorkbenchSessionOpenStatusTests.cpp
- tests/APP2WorkbenchOpenStatusProofRunnerTests.cpp
- tests/StartupProofTests.cpp
- CMakeLists.txt

Closed line:
active work manifest if present
or fixtures/storage/c2-compound-work/myworld.work.json
+ fixtures/graphs/g1_loudness_to_shader_uniform.graph.json
-> MainComponent::currentWorkbenchSession
-> debug/app2-workbench-open-status-proof/workbench_open_status_report.json

Latest accepted result:
- cmake -S . -B build passed.
- cmake --build build --target my_world_workbench_session_open_status_tests my_world_app2_workbench_open_status_proof_runner_tests my_world_startup_proof_tests passed.
- ./build/my_world_workbench_session_open_status_tests passed.
- ./build/my_world_app2_workbench_open_status_proof_runner_tests passed.
- ./build/my_world_startup_proof_tests passed.
- cmake --build build --target my-world passed.
- MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app2-workbench-open-status-proof-and-exit passed and wrote `debug/app2-workbench-open-status-proof/workbench_open_status_report.json` with `ok: true`.
- ctest --test-dir build --output-on-failure passed 83/83.
- git diff --check passed.

Previous closure:

None after APP1 workbench session spine closure as of 2026-05-25 18:39 Asia/Taipei.

APP1 workbench session spine closed.
Evidence:
- docs/superpowers/specs/2026-05-25-app1-workbench-session-spine.md
- source/app/WorkbenchSession.*
- source/app/APP1WorkbenchSessionProofRunner.*
- source/app/Main.cpp
- source/app/MainComponent.*
- source/app/StartupProof.*
- tests/WorkbenchSessionTests.cpp
- tests/APP1WorkbenchSessionProofRunnerTests.cpp
- tests/StartupProofTests.cpp
- CMakeLists.txt

Closed line:
fixtures/storage/c2-compound-work/myworld.work.json
+ fixtures/graphs/g1_loudness_to_shader_uniform.graph.json
-> WorkbenchSession snapshot
-> debug/app1-workbench-session-proof/workbench_session_report.json

Latest accepted result:
- cmake -S . -B build passed.
- cmake --build build --target my_world_workbench_session_tests passed.
- ./build/my_world_workbench_session_tests passed.
- cmake --build build --target my_world_app1_workbench_session_proof_runner_tests passed.
- ./build/my_world_app1_workbench_session_proof_runner_tests passed.
- cmake --build build --target my_world_startup_proof_tests passed.
- ./build/my_world_startup_proof_tests passed.
- MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-app1-workbench-session-proof-and-exit passed and wrote `debug/app1-workbench-session-proof/workbench_session_report.json` with `ok: true`.
- ctest --test-dir build --output-on-failure passed 81/81.
- cmake --build build --target my-world passed.
- git diff --check passed.

Previous closure:

None after G1 proof artifact hardening closure as of 2026-05-25 18:23 Asia/Taipei.

G1 proof artifact hardening closed.
Evidence:
- source/app/G1GraphIOMappingProofRunner.*
- tests/G1GraphIOMappingProofRunnerTests.cpp
- docs/superpowers/specs/2026-05-25-g1-graph-io-mapping-foundation.md
- docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md
- CMakeLists.txt

Closed line:
fixtures/graphs/g1_loudness_to_shader_uniform.graph.json
-> G1GraphIOMappingProofRunner
-> debug/g1-graph-io-mapping-proof/graph_io_mapping_report.json

Latest accepted result:
- cmake --build build --target my_world_g1_graph_io_mapping_proof_runner_tests
- ./build/my_world_g1_graph_io_mapping_proof_runner_tests passed.
- ./build/my_world_graph_io_mapping_tests passed.
- ./build/my_world_live_io_bus_tests passed.
- ctest --test-dir build --output-on-failure passed 79/79.
- cmake --build build --target my-world passed.
- git diff --check passed.

Previous closure:

None after G1 graph IO mapping foundation closure as of 2026-05-25 17:48 Asia/Taipei.

G1 graph IO mapping foundation closed.
Evidence:
- docs/superpowers/specs/2026-05-25-g1-graph-io-mapping-foundation.md
- fixtures/graphs/g1_loudness_to_shader_uniform.graph.json
- source/core/GraphIOMapping.*
- source/storage/GraphIOMappingStorage.*
- tests/GraphIOMappingTests.cpp
- CMakeLists.txt

Closed line:
fixtures/graphs/g1_loudness_to_shader_uniform.graph.json
-> GraphIOMapping validation
-> LiveIOBus binding generation
-> graph_io_mapping_report.json

Latest accepted result:
- cmake -S . -B build passed.
- cmake --build build --target my_world_graph_io_mapping_tests
- ./build/my_world_graph_io_mapping_tests passed.
- cmake --build build --target my_world_live_io_bus_tests
- ./build/my_world_live_io_bus_tests passed.
- ctest --test-dir build --output-on-failure passed 78/78.
- git diff --check passed.
- cmake --build build --target my-world passed.

BUILD1 portable JUCE bootstrap closed.
Evidence:
- docs/superpowers/specs/2026-05-25-build1-portable-juce-bootstrap.md
- CMakeLists.txt
- .github/workflows/cmake.yml
- README.md
- docs/portable-build/juce.md

Closed line:
clean checkout
-> cmake -DMY_WORLD_FETCH_JUCE=ON
-> pinned JUCE FetchContent
-> my-world target
-> ctest

Latest accepted result:
- existing local JUCE configure/build still passes.
- fetch-path configure/build smoke passes.
- fetch-path full suite passes 77/77.
- app target `my-world` builds.
- full suite passes.
- `git diff --check` passes.

Previous closure:

None after Gemini risk triage hardening closure as of 2026-05-25.

Gemini risk triage hardening closed.
Evidence:
- docs/superpowers/specs/2026-05-25-gemini-risk-triage-hardening.md
- source/core/LiveIOOscReceiver.cpp
- source/app/LiveIOProofRunner.cpp
- source/render/OpenGLShaderPreview.cpp
- source/ui/ImGuiSmokeOverlayHelpers.cpp
- tests/LiveIOOscReceiverTests.cpp
- tests/LiveIOSendAdapterTests.cpp
- CMakeLists.txt

Closed line:
malformed OSC packet
-> bounded decode
-> failed packet status
-> no null-terminated over-read

OpenGL proof dump
-> capture frames on GL thread
-> write artifacts on background thread
-> status callback on message thread

Latest accepted result:
- focused live IO send adapter/OSC receiver/proof runner tests passed.
- app target `my-world` builds.
- app proof dump command exits cleanly.
- full suite passed.
- `git diff --check` passed.

Previous closure:

None after AV5 audio visual segment closure as of 2026-05-25.

AV5 audio visual segment closure closed.
Evidence:
- docs/superpowers/specs/2026-05-25-av5-audio-visual-segment-closure.md
- docs/superpowers/specs/2026-05-25-av1-audio-to-visual-bridge.md
- docs/superpowers/specs/2026-05-25-av2-visual-reaction-proof.md
- docs/superpowers/specs/2026-05-25-av3-real-opengl-capture-pair.md
- docs/superpowers/specs/2026-05-25-av4-app-dump-visual-reaction-readback.md

Closed line:
live IO shader.uniform evidence
-> MainComponent audio-to-visual bridge
-> OpenGLShaderPreview loudness input
-> quiet/loud OpenGL proof frames
-> V1ShaderProofArtifacts
-> debug/v1-shader-proof/visual_reaction.json
-> ok true / changed / non-zero pixel delta

Latest accepted result:
- AV3 app target `my-world` builds.
- AV3 `77/77 tests passed`.
- AV4 app dump command exited cleanly.
- AV4 readback reported `ok: true`, `status: changed`, `changedPixels: 4147429`, `meanAbsDelta: 0.071055`.
- AV5 `git diff --check` passed.

Previous closure:

None after AV4 app dump visual reaction readback closure as of 2026-05-25.

AV4 app dump visual reaction readback closed.
Evidence:
- docs/superpowers/specs/2026-05-25-av4-app-dump-visual-reaction-readback.md
- debug/v1-shader-proof/visual_reaction.json

Closed line:
my-world --dump-proof-and-exit
-> debug/v1-shader-proof/visual_reaction.json
-> ok true / changed / non-zero pixel delta

Latest accepted result:
- app dump command exited cleanly.
- `visual_reaction.json` readback reported `ok: true`, `status: changed`, `changedPixels: 4147429`, `meanAbsDelta: 0.071055`.
- `git status --short` stayed clean after ignored proof artifacts.

Previous closure:

None after AV3 real OpenGL capture pair closure as of 2026-05-25.

AV3 real OpenGL capture pair closed.
Evidence:
- docs/superpowers/specs/2026-05-25-av3-real-opengl-capture-pair.md
- source/render/OpenGLShaderPreview.cpp

Closed line:
OpenGLShaderPreview proof dump
-> render loudness 0.0 / 1.0 in the active GL context
-> V1ShaderProofArtifacts quiet/loud frames
-> visual_reaction.json from real preview capture path

Latest accepted result:
- app target `my-world` builds.
- `77/77 tests passed`.
- `git diff --check` passed.

Previous closure:

None after AV2 visual reaction proof closure as of 2026-05-25.

AV2 visual reaction proof closed.
Evidence:
- docs/superpowers/specs/2026-05-25-av2-visual-reaction-proof.md
- source/render/V1ShaderProofArtifacts.h
- source/render/V1ShaderProofArtifacts.cpp
- tests/V1ShaderProofArtifactsTests.cpp

Closed line:
quiet/loud proof frames
-> V1ShaderProofArtifacts
-> visual_reaction.json
-> changedPixels + meanAbsDelta

Latest accepted result:
- RED first on missing V1 visual reaction request fields.
- focused `v1_shader_proof_artifacts` test passed.
- app target `my-world` builds.
- `77/77 tests passed`.
- `git diff --check` passed.

Previous closure:

None after AV1 audio-to-visual bridge closure as of 2026-05-25.

AV1 audio-to-visual bridge closed.
Evidence:
- docs/superpowers/specs/2026-05-25-av1-audio-to-visual-bridge.md
- source/app/MainComponent.cpp
- source/render/OpenGLShaderPreview.h
- source/render/OpenGLShaderPreview.cpp
- CMakeLists.txt

Closed line:
LiveIOStatusIndicatorState shader uniform evidence
-> MainComponent::tickLiveIOControl()
-> ShaderPreviewInputSnapshot
-> OpenGLShaderPreview::setInputSnapshot()
-> existing preview loudness render input

Latest accepted result:
- RED first on missing `OpenGLShaderPreview::setInputSnapshot`.
- app target `my-world` builds.
- `77/77 tests passed`.
- `git diff --check` passed.

Previous closure:

None after P-LIVE30 A1 to V1 handoff closure as of 2026-05-25.

P-LIVE30 A1 to V1 handoff closure closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live30-a1-to-v1-handoff-closure.md
- docs/superpowers/specs/2026-05-25-p-live25-shader-uniform-control-evidence.md
- docs/superpowers/specs/2026-05-25-p-live26-shader-uniform-evidence-json-shape.md
- docs/superpowers/specs/2026-05-25-p-live27-shader-uniform-proof-artifact.md
- docs/superpowers/specs/2026-05-25-p-live28-shader-preview-input-bridge-contract.md
- docs/superpowers/specs/2026-05-25-p-live29-shader-preview-smoke-read.md

Closed line:
AudioAnalyzerSnapshot.loudness
-> LiveIO shader.uniform evidence
-> live_io_shader_uniform_report.json
-> ShaderPreviewInputSnapshot
-> RenderFrameInput smoke-read

Latest accepted result:
- P-LIVE25 through P-LIVE29 each closed with focused tests.
- Latest full suite at P-LIVE29: `77/77 tests passed`.
- P-LIVE30 is docs-only closure; `git diff --check` passed.

Previous closure:

None after P-LIVE29 shader preview smoke-read closure as of 2026-05-25.

P-LIVE29 shader preview smoke-read closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live29-shader-preview-smoke-read.md
- source/render/ShaderPreviewInputBridge.h
- source/render/ShaderPreviewInputBridge.cpp
- tests/ShaderPreviewSmokeReadTests.cpp
- CMakeLists.txt

Closed line:
ShaderPreviewInputSnapshot
-> RenderFrameInput
-> fake RenderBackend.renderFrame()
-> smoke-read test

Latest accepted result:
- RED first on missing `makeRenderFrameInputFromShaderPreviewInput`.
- focused `shader_preview_smoke_read` test passed.
- app target `my-world` builds.
- `77/77 tests passed`.
- `git diff --check` passed.

Previous closure:

None after P-LIVE28 shader preview input bridge contract closure as of 2026-05-25.

P-LIVE28 shader preview input bridge contract closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live28-shader-preview-input-bridge-contract.md
- source/render/ShaderPreviewInputBridge.h
- source/render/ShaderPreviewInputBridge.cpp
- tests/ShaderPreviewInputBridgeTests.cpp
- CMakeLists.txt

Closed line:
live_io_shader_uniform_report.json shape
-> ShaderPreviewInputBridge
-> ShaderPreviewInputSnapshot JSON
-> focused bridge test

Latest accepted result:
- RED first on missing `source/render/ShaderPreviewInputBridge.cpp`.
- focused `shader_preview_input_bridge` test passed.
- app target `my-world` builds.
- `76/76 tests passed`.
- `git diff --check` passed.

Previous closure:

None after P-LIVE27 shader uniform proof artifact closure as of 2026-05-25.

P-LIVE27 shader uniform proof artifact closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live27-shader-uniform-proof-artifact.md
- source/app/LiveIOProofRunner.cpp
- tests/LiveIOProofRunnerTests.cpp

Closed line:
LiveIOControlTimerState shader uniform evidence
-> live_io_shader_uniform_report.json
-> artifact list
-> proof runner test

Latest accepted result:
- RED first on expected 11th proof artifact.
- focused `live_io_proof_runner` test passed.
- app target `my-world` builds.
- `75/75 tests passed`.
- `git diff --check` passed.

Previous closure:

None after P-LIVE26 shader uniform evidence JSON shape closure as of 2026-05-25.

P-LIVE26 shader uniform evidence JSON shape closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live26-shader-uniform-evidence-json-shape.md
- source/core/LiveIOStatusIndicator.cpp
- source/app/LiveIOProofRunner.cpp
- tests/LiveIOStatusIndicatorTests.cpp
- tests/LiveIOProofRunnerTests.cpp

Closed line:
P-LIVE25 flat uniform fields
-> nested shaderUniformEvidence object
-> status JSON
-> app timer proof JSON

Latest accepted result:
- RED first on missing `shaderUniformEvidence` object.
- focused `live_io_status_indicator` and `live_io_proof_runner` tests passed.
- app target `my-world` builds.
- `75/75 tests passed`.
- `git diff --check` passed.

Previous closure:

None after P-LIVE25 shader uniform control evidence closure as of 2026-05-25.

P-LIVE25 shader uniform control evidence closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live25-shader-uniform-control-evidence.md
- source/core/LiveIOControlDispatcher.h
- source/core/LiveIOControlDispatcher.cpp
- source/core/LiveIOControlTimer.h
- source/core/LiveIOControlTimer.cpp
- source/core/LiveIOStatusIndicator.h
- source/core/LiveIOStatusIndicator.cpp
- source/app/LiveIOAppController.cpp
- source/app/LiveIOProofRunner.cpp
- tests/LiveIOControlDispatcherTests.cpp
- tests/LiveIOControlTimerTests.cpp
- tests/LiveIOStatusIndicatorTests.cpp
- tests/LiveIOAppControllerTests.cpp
- tests/LiveIOProofRunnerTests.cpp

Closed line:
LiveIOPreferences.outputOperator
-> shader.uniform binding
-> LiveIOControlDispatchReport.shaderUniforms
-> LiveIOControlTimerState latest uniform evidence
-> app status JSON and app timer proof JSON

Latest accepted result:
- RED first on missing `LiveIOControlDispatchReport::shaderUniforms`.
- focused `live_io_control_dispatcher`, `live_io_control_timer`, `live_io_status_indicator`, `live_io_app_controller`, and `live_io_proof_runner` tests passed.
- app target `my-world` builds.
- `75/75 tests passed`.
- `git diff --check` passed.

Previous closure:

None after P-LIVE24 live IO operator status evidence closure as of 2026-05-25.

P-LIVE24 live IO operator status evidence closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live24-live-io-operator-status-evidence.md
- source/core/LiveIOStatusIndicator.h
- source/core/LiveIOStatusIndicator.cpp
- source/app/LiveIOAppController.cpp
- source/app/LiveIOProofRunner.cpp
- tests/LiveIOStatusIndicatorTests.cpp
- tests/LiveIOAppControllerTests.cpp
- tests/LiveIOProofRunnerTests.cpp

Closed line:
LiveIOPreferences.outputOperator
-> LiveIOAppController sanitized preference
-> LiveIOStatusIndicatorState.outputOperator
-> status text and status JSON
-> app timer proof JSON

Latest accepted result:
- RED first on missing `withLiveIOOutputOperator`.
- focused `live_io_status_indicator`, `live_io_app_controller`, and `live_io_proof_runner` tests passed.
- app target `my-world` builds.
- `75/75 tests passed`.
- `git diff --check` passed.

Previous closure:

None after P-LIVE23 UI operator picker foundation closure as of 2026-05-25.

P-LIVE23 UI operator picker foundation closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live23-ui-operator-picker-foundation.md
- source/preferences/PerformancePreferences.h
- source/preferences/PerformancePreferences.cpp
- source/app/PreferencesPanel.h
- source/app/PreferencesPanel.cpp
- source/app/LiveIOAppController.cpp
- tests/PerformancePreferencesTests.cpp
- tests/LiveIOAppControllerTests.cpp

Closed line:
LiveIOPreferences.outputOperator
-> PreferencesPanel combo
-> LiveIOAppController binding selection
-> existing LiveIOControlTimer/dispatcher
-> live IO status evidence

Latest accepted result:
- RED first on missing `LiveIOOutputOperatorPreference` / `LiveIOPreferences::outputOperator`.
- RED first on fixed controller MIDI+OSC binding behavior.
- focused `performance_preferences` and `live_io_app_controller` tests passed.
- app target `my-world` builds.
- `75/75 tests passed`.
- `git diff --check` passed.

Previous closure:

None after P-LIVE22 OSC server lifecycle closure as of 2026-05-25.

P-LIVE22 OSC server lifecycle closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live22-osc-server-lifecycle.md
- source/app/LiveIOAppController.h
- source/app/LiveIOAppController.cpp
- tests/LiveIOAppControllerTests.cpp

Closed line:
LiveIOPreferences OSC receive config
-> LiveIOAppController receiver lifecycle
-> app timer poll
-> LiveIOValueFrame from incoming OSC float
-> status evidence

Latest accepted result:
- RED first on missing `LiveIOAppTimerResult` OSC receiver lifecycle fields.
- focused `live_io_app_controller` and `live_io_osc_receiver` tests passed.
- app target `my-world` builds.
- `75/75 tests passed`.
- `git diff --check` passed.

Previous closure:

None after P-LIVE21 realtime delivery backpressure closure as of 2026-05-25.

P-LIVE21 realtime delivery backpressure closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live21-realtime-delivery-backpressure.md
- source/audio/AudioRealtimeDelivery.h
- source/audio/AudioRealtimeDelivery.cpp
- source/core/LiveIOStatusIndicator.h
- source/core/LiveIOStatusIndicator.cpp
- source/app/MainComponent.cpp
- tests/AudioRealtimeDeliveryTests.cpp
- tests/LiveIOStatusIndicatorTests.cpp

Closed line:
audio callback snapshots
-> bounded multi-slot delivery slots
-> app timer consume latest completed snapshot
-> skipped / overwritten telemetry
-> existing live IO status label

Latest accepted result:
- RED first on missing `AudioRealtimeDeliveryResult::skippedSnapshots` and `overwrittenSnapshots`.
- RED first on missing `LiveIORealtimeIndicatorTelemetry` skipped/overwritten fields.
- focused `audio_realtime_delivery` and `live_io_status_indicator` tests passed.
- app target `my-world` builds.
- `75/75 tests passed`.
- `git diff --check` passed.

Previous closure:

None after P-LIVE20 live IO realtime indicator closure as of 2026-05-25.

P-LIVE20 live IO realtime indicator closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live20-live-io-realtime-indicator.md
- source/core/LiveIOStatusIndicator.h
- source/core/LiveIOStatusIndicator.cpp
- source/app/MainComponent.h
- source/app/MainComponent.cpp
- tests/LiveIOStatusIndicatorTests.cpp

Closed line:
AudioRealtimeDeliveryResult
-> MainComponent app timer
-> LiveIOStatusIndicatorState realtime fields
-> existing live IO status label

Latest accepted result:
- RED first on missing `LiveIORealtimeIndicatorTelemetry` and `withLiveIORealtimeTelemetry()`.
- focused `live_io_status_indicator` passed.
- app target `my-world` builds.
- `75/75 tests passed`.
- `git diff --check` passed.

Previous closure:

None after P-LIVE19 realtime delivery status closure as of 2026-05-25.

P-LIVE19 realtime delivery status closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live19-realtime-delivery-status.md
- source/audio/AudioRealtimeDelivery.h
- source/audio/AudioRealtimeDelivery.cpp
- source/app/MainComponent.cpp
- tests/AudioRealtimeDeliveryTests.cpp

Closed line:
AudioRealtimeDelivery.consumeLatest()
-> status / sequence / droppedSnapshots
-> MainComponent app timer status text
-> existing audio status label

Latest accepted result:
- RED first on missing `AudioRealtimeDeliveryResult::status`, `AudioRealtimeDeliveryResult::droppedSnapshots`, and `makeAudioRealtimeDeliveryStatusText()`.
- focused `audio_realtime_delivery` passed.
- app target `my-world` builds.

Previous closure:

None after P-LIVE18 realtime callback delivery closure as of 2026-05-25 16:00 Asia/Taipei.

P-LIVE18 realtime callback delivery closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live18-realtime-callback-delivery.md
- source/audio/AudioRealtimeDelivery.h
- source/audio/AudioRealtimeDelivery.cpp
- source/audio/AudioInputAnalyzer.h
- source/audio/AudioInputAnalyzer.cpp
- source/app/MainComponent.h
- source/app/MainComponent.cpp
- tests/AudioRealtimeDeliveryTests.cpp
- CMakeLists.txt

Closed line:
audio callback analyzer snapshot
-> atomic realtime delivery slot
-> app timer consume
-> existing LiveIOControlTimer outside callback

Latest accepted result:
- RED first on missing `AudioRealtimeDelivery.cpp`.
- focused `audio_realtime_delivery` passed.
- app target `my-world` builds.
- `75/75 tests passed`.

P-LIVE17 broader MIDI output operators closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live17-broader-midi-output-operators.md
- source/core/LiveIOBus.h
- source/core/LiveIOBus.cpp
- source/core/LiveIOMidiSendProof.h
- source/core/LiveIOMidiSendProof.cpp
- source/core/LiveIOControlDispatcher.cpp
- tests/LiveIOBusTests.cpp
- tests/LiveIOMidiSendProofTests.cpp
- tests/LiveIOControlDispatcherTests.cpp

Closed line:
LiveIO normalized value
-> MIDI CC or MIDI note-on event
-> injected control-rate sender
-> byte-level proof report

Latest accepted result:
- RED first on missing note-on API.
- focused `live_io_bus|live_io_midi_send_proof|live_io_control_dispatcher` passed.
- app target `my-world` builds.

P-LIVE13 arbitrary binding MIDI teach closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live13-arbitrary-binding-midi-teach.md
- source/core/LiveIOMidiTeach.h
- source/core/LiveIOMidiTeach.cpp
- tests/LiveIOMidiTeachTests.cpp

Closed line:
LiveIOBinding list
-> arm MIDI teach for one binding id
-> incoming MIDI CC
-> update only that binding's MIDI channel/CC
-> preserve fixed loudness/map CC teach behavior

Latest verification:
- `cmake --build build --target my_world_live_io_midi_teach_tests` failed RED first on missing `armLiveIOMidiTeachForBinding()`, `LiveIOMidiTeachResult::bindingId`, and `applyLiveIOMidiTeachToBindings()`.
- `cmake --build build --target my_world_live_io_midi_teach_tests`
- `./build/my_world_live_io_midi_teach_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "live_io_midi_teach|live_io_app_controller|performance_preferences"`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:
- `live io midi teach ok`
- app target `my-world` builds.
- `73/73 tests passed`
- `git diff --check passed`

P-LIVE14 OSC target preferences closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live14-osc-target-preferences.md
- source/preferences/PerformancePreferences.h
- source/preferences/PerformancePreferences.cpp
- source/app/LiveIOAppController.cpp
- tests/PerformancePreferencesTests.cpp

Closed line:
PerformancePreferences.liveIO OSC target
-> sanitize host/port/address
-> disk roundtrip
-> keep controlled app timer OSC send disabled until P-LIVE15

Latest verification:
- `cmake --build build --target my_world_performance_preferences_tests` failed RED first on missing OSC target fields in `LiveIOPreferences`.
- `cmake --build build --target my_world_performance_preferences_tests && ./build/my_world_performance_preferences_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "performance_preferences|live_io_app_controller"`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:
- `performance preferences ok`
- app target `my-world` builds.
- `73/73 tests passed`
- `git diff --check passed`

P-LIVE15 external OSC target send proof closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live15-external-osc-target-send-proof.md
- source/core/LiveIOControlDispatcher.h
- source/core/LiveIOControlDispatcher.cpp
- source/core/LiveIOControlPump.h
- source/core/LiveIOControlPump.cpp
- source/core/LiveIOControlTimer.h
- source/core/LiveIOControlTimer.cpp
- source/app/LiveIOAppController.h
- source/app/LiveIOAppController.cpp
- tests/LiveIOAppControllerTests.cpp

Closed line:
PerformancePreferences OSC target
-> LiveIOAppController controlled timer config
-> injected OSC sender receives host/port/address/value
-> no always-on OSC server and no realtime callback delivery

Latest verification:
- `cmake --build build --target my_world_live_io_app_controller_tests` failed RED first on missing `LiveIOAppTimerRequest::oscSender` and OSC message host/port fields.
- `cmake --build build --target my_world_live_io_app_controller_tests && ./build/my_world_live_io_app_controller_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "live_io_app_controller|live_io_control_dispatcher|live_io_control_pump|live_io_control_timer"`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:
- `live io app controller ok`
- app target `my-world` builds.
- `73/73 tests passed`
- `git diff --check passed`

P-LIVE16 OSC receive spine closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live16-osc-receive-spine.md
- source/core/LiveIOOscReceiver.h
- source/core/LiveIOOscReceiver.cpp
- tests/LiveIOOscReceiverTests.cpp
- CMakeLists.txt

Closed line:
OSC float datagram
-> controlled UDP receiver poll
-> decoded address/value
-> LiveIOValueFrame for graph/control use
-> no realtime callback delivery

Latest verification:
- `cmake -S . -B build && cmake --build build --target my_world_live_io_osc_receiver_tests` failed RED first on missing `source/core/LiveIOOscReceiver.cpp`.
- `cmake -S . -B build && cmake --build build --target my_world_live_io_osc_receiver_tests && ./build/my_world_live_io_osc_receiver_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "live_io_osc_receiver|live_io_send_adapter"`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:
- `live io osc receiver ok`
- app target `my-world` builds.
- `74/74 tests passed`
- `git diff --check passed`

Next selectable lane:
- None selected.
- Realtime callback delivery and broader MIDI output operators remain parked.
- External OSC/UDP targets and always-on OSC receive nodes/server remain parked.
- Full render/export window/process states remain parked.
- Variation child enable UI and symbol-browser preset creation remain parked.
```

Previous closure:

```text
None after P-LIVE1.3 controlled OSC loopback proof closure as of 2026-05-25 12:02 Asia/Taipei.

P-LIVE1.3 controlled OSC loopback proof closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live1-3-osc-loopback-proof.md
- source/core/LiveIOSendAdapter.h
- source/core/LiveIOSendAdapter.cpp
- source/app/LiveIOProofRunner.cpp
- tests/LiveIOSendAdapterTests.cpp
- tests/LiveIOProofRunnerTests.cpp

Closed line:
LiveIOBus OSC float event
-> LiveIOSendAdapter controlled loopback route
-> localhost UDP/OSC packet
-> loopback receiver evidence
-> live_io_osc_loopback_report.json

Latest verification:
- `cmake --build build --target my_world_live_io_send_adapter_tests && ./build/my_world_live_io_send_adapter_tests` failed RED first on missing `makeLiveIOControlledOscLoopbackRoute` and `executeLiveIOSendBoundary`.
- After first implementation, the same focused test failed on OSC float byte order.
- `./build/my_world_live_io_proof_runner_tests` failed RED first on missing fourth artifact.
- `cmake --build build --target my_world_live_io_send_adapter_tests my_world_live_io_proof_runner_tests my-world`
- `./build/my_world_live_io_send_adapter_tests`
- `./build/my_world_live_io_proof_runner_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-live-io-proof-and-exit`

Latest accepted result:
- `live io send adapter ok`
- `live io proof runner ok`
- `debug/p-live1-live-io-proof/live_io_osc_loopback_report.json` has `ok: true`, `status: "received"`, `sendStatus: "controlled_send"`, `sent: true`, `received: true`, address `/my-world/loudness`, and `receivedFloatValue: 0.500000`.

Next selectable lane:
- None selected.
- Real MIDI device send, external OSC/UDP targets, always-on OSC receive nodes/server, teach mode, realtime callback wiring, and live UI mapping remain parked.
- Full render/export window/process states remain parked.
- Variation child enable UI and symbol-browser preset creation remain parked.
```

Previous closure:

```text
None after P-LIVE1.2 live IO send boundary closure as of 2026-05-25 11:34 Asia/Taipei.

P-LIVE1.2 live IO send boundary closed.
Evidence:
- docs/superpowers/specs/2026-05-25-p-live1-2-live-io-send-boundary.md
- source/core/LiveIOSendAdapter.h
- source/core/LiveIOSendAdapter.cpp
- source/app/LiveIOProofRunner.cpp
- tests/LiveIOSendAdapterTests.cpp
- tests/LiveIOProofRunnerTests.cpp
- CMakeLists.txt

Closed line:
LiveIOBus target events
-> MIDI/OSC send-boundary adapter
-> dry-run MIDI CC action and OSC float action report
-> live_io_send_report.json

Latest verification:
- `cmake -S . -B build && cmake --build build --target my_world_live_io_send_adapter_tests` failed RED first on missing `LiveIOSendAdapter.h`.
- `./build/my_world_live_io_proof_runner_tests` failed RED first on missing third artifact.
- `cmake --build build --target my_world_live_io_send_adapter_tests my_world_live_io_bus_tests my_world_live_io_proof_runner_tests my_world_startup_proof_tests my-world`
- `./build/my_world_live_io_send_adapter_tests`
- `./build/my_world_live_io_bus_tests`
- `./build/my_world_live_io_proof_runner_tests`
- `./build/my_world_startup_proof_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-live-io-proof-and-exit`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `live io send adapter ok`
- `live io bus ok`
- `live io proof runner ok`
- `debug/p-live1-live-io-proof/live_io_send_report.json` has `ok: true`, `status: "dry_run"`, MIDI CC value `64`, OSC endpoint `127.0.0.1:9000`, `sent: false`, and skipped `shader.uniform: uniform.loudness`.
- `65/65 tests passed`

Next selectable lane:
- None selected.
- Real MIDI device send, OSC UDP send/receive, teach mode, realtime callback wiring, and live UI mapping remain parked.
- Full render/export window/process states remain parked.
- Variation child enable UI and symbol-browser preset creation remain parked.
```

Previous closure:

```text
None after P-LIVE1 live IO bus foundation closure as of 2026-05-25 11:22 Asia/Taipei.

P-LIVE1 live IO bus foundation closed.
Evidence:
- source/core/LiveIOBus.h
- source/core/LiveIOBus.cpp
- source/app/LiveIOProofRunner.h
- source/app/LiveIOProofRunner.cpp
- source/app/StartupProof.h
- source/app/StartupProof.cpp
- source/app/Main.cpp
- source/app/MainComponent.h
- source/app/MainComponent.cpp
- tests/LiveIOBusTests.cpp
- tests/LiveIOProofRunnerTests.cpp
- tests/StartupProofTests.cpp
- CMakeLists.txt

Closed line:
loaded `compound.loudness` runtime public outputs
-> LiveIOValueFrame
-> LiveIOBus bindings
-> MIDI CC target event, OSC float target event, shader uniform target event
-> `live_io_report.json`
-> app CLI `--dump-live-io-proof-and-exit`

Latest verification:
- `cmake -S . -B build && cmake --build build --target my_world_live_io_bus_tests my_world_live_io_proof_runner_tests` failed RED first on missing `source/core/LiveIOBus.cpp` and `source/app/LiveIOProofRunner.cpp`.
- `cmake --build build --target my_world_live_io_bus_tests my_world_live_io_proof_runner_tests my_world_startup_proof_tests my-world`
- `./build/my_world_live_io_bus_tests`
- `./build/my_world_live_io_proof_runner_tests`
- `./build/my_world_startup_proof_tests`
- `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-live-io-proof-and-exit`

Latest accepted result:
- `live io bus ok`
- `live io proof runner ok`
- `debug/p-live1-live-io-proof/live_io_report.json` has `ok: true`, `targetKind` values `midi.cc`, `osc.float`, and `shader.uniform`, and `midiValue: 64`.
- `debug/p-live1-live-io-proof/live_io_runtime_execution.json` has computed `compound.loudness` public outputs.

Next selectable lane:
- None selected.
- Real MIDI/OSC device IO, teach mode, UDP send/receive, realtime callback wiring, and live UI mapping remain parked.
- Full render/export window/process states remain parked.
- Variation child enable UI and symbol-browser preset creation remain parked.

Previous closure:
None after VAR-005 hover preview / Alt blend and R-TN1 real thumbnail headless proof closure as of 2026-05-25 11:05 Asia/Taipei.

VAR-005 hover preview / Alt blend closed.
Evidence:
- source/core/VariationState.h
- source/core/VariationState.cpp
- source/core/InteractionContract.h
- source/core/InteractionContract.cpp
- source/ui/ImGuiSmokeOverlay.cpp
- tests/VariationStateTests.cpp
- docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md

Closed line:
VariationRecord values
-> previewVariationBlend reads current graph state without mutation or command logging
-> numeric values blend deterministically by weight
-> enum/string/bool-style values step: current before full weight, target at weight 1.0
-> commitVariationBlend writes one `apply_variation_blend` command with undo/redo
-> left-rail hover tooltip reads preview report; Alt-click commits a 0.5 blend

R-TN1 real thumbnail headless proof closed.
Evidence:
- source/render/HeadlessRenderRuntime.h
- source/render/HeadlessRenderRuntime.cpp
- tests/HeadlessRenderRuntimeTests.cpp

Closed line:
runtime proof fixture
-> `image.constant -> output.texture_summary`
-> `texture_summary.json`, `cook_order.json`, `node_stats.json`, and `errors.json`
-> deterministic 96x54 `thumbnail.png`
-> `thumbnail_stats.json` records source/thumbnail dimensions, format, color, and source ids

Latest verification:
- `cmake --build build --target my_world_variation_state_tests` failed RED first on missing `VariationPreviewReport`, `VariationPreviewValue`, `previewVariationBlend`, and `commitVariationBlend`.
- `cmake --build build --target my_world_headless_render_runtime_tests` failed RED first on missing `thumbnailPath` and `thumbnailStatsPath`.
- `cmake --build build --target my_world_variation_state_tests`
- `./build/my_world_variation_state_tests`
- `cmake --build build --target my_world_headless_render_runtime_tests`
- `./build/my_world_headless_render_runtime_tests`
- `cmake --build build --target my_world_variation_state_tests my_world_headless_render_runtime_tests my-world`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `variation state ok`
- `headless render runtime ok`
- `62/62 full tests passed`

Next selectable lane:
- At this closure time, P-LIVE1 MIDI/OSC/live IO remained parked; later P-LIVE1 closes the first headless mapping foundation.
- Full render/export window/process states remain parked.
- Variation child enable UI and symbol-browser preset creation remain parked.

Previous closure:
None after P-VAR005 variation thumbnail selection hit-test closure as of 2026-05-25 10:41 Asia/Taipei.

P-VAR005 closed.
Evidence:
- source/core/VariationState.h
- source/core/VariationState.cpp
- source/core/InteractionContract.h
- source/core/InteractionContract.cpp
- source/ui/ImGuiSmokeOverlay.h
- source/ui/ImGuiSmokeOverlay.cpp
- tests/VariationStateTests.cpp
- docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md

Closed line:
VariationLibrary records
-> VariationThumbnailLayout for presets and snapshots
-> point hit-test returns variation kind/id/index
-> selectVariation stores selected variation without applying preset/snapshot or logging a graph command
-> left-rail Presets/Snapshots tabs render and select thumbnails through the same layout/hit-test contract

Latest verification:
- `cmake --build build --target my_world_variation_state_tests` failed RED first on missing `VariationThumbnailHitTest`, `VariationSelection`, `makeVariationThumbnailLayout`, `hitTestVariationThumbnails`, `selectVariation`, and `GraphSession::selectedVariation`.
- `cmake --build build --target my_world_variation_state_tests`
- `./build/my_world_variation_state_tests`
- `cmake --build build --target my_world_variation_state_tests my-world`
- `ctest --test-dir build --output-on-failure -R "variation_state|canvas_hands|node_hit_tests|interaction_storage_roundtrip|graph_commands"`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

Latest accepted result:
- `variation state ok`
- `5/5 focused interaction tests passed`
- `62/62 full tests passed`
- `git diff --check passed`

Next selectable lane:
- At P-VAR005 closure time, hover preview / Alt blend remained parked and were not touched in that thumbnail-only slice; later VAR-005 closes that line.

Previous closure:
None after P-VAR004 variation canvas CRUD foundation closure as of 2026-05-25 10:19 Asia/Taipei.

P-VAR004 closed.
Evidence:
- source/core/InteractionContract.h
- source/core/InteractionContract.cpp
- tests/VariationStateTests.cpp
- docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md
- docs/superpowers/specs/2026-05-24-tixl-taxonomy-parity-spec.md

Closed line:
VariationLibrary records
-> rename/delete/move commandGraph verbs
-> preset and snapshot CRUD separation
-> undo/redo restores title, existence, and order
-> PatchDocument and saveWork preserve updated variation library

Latest verification:
- `cmake --build build --target my_world_variation_state_tests` failed RED first on missing `renameVariation`, `deleteVariation`, and `moveVariation`.
- `cmake --build build --target my_world_variation_state_tests`
- `ctest --test-dir build --output-on-failure -R variation_state`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `variation state ok`
- `1/1 focused tests passed`
- `62/62 full tests passed`

Previous closure:
None after P-OPS1A reconnect/split macro closure as of 2026-05-25 08:36 Asia/Taipei.

P-OPS1A closed.
Evidence:
- source/core/InteractionContract.h
- source/core/InteractionContract.cpp
- source/core/GraphLanguage.cpp
- tests/T3T5CommandTests.cpp
- tests/GraphCommandTests.cpp
- tests/InteractionTraceTests.cpp
- fixtures/interaction/tooll3-t0-t7.behavior.json
- docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md
- docs/superpowers/specs/2026-05-24-tixl-taxonomy-parity-spec.md

Closed line:
reconnect input end + reconnect output beginning + split edge create operator
-> each graph operation lowers to one commandGraph macro
-> undo/redo restores the whole graph mutation as one unit
-> behavior trace names the borrowed Tooll3 interaction without UI polish

Latest verification:
- `cmake --build build --target my_world_t3_t5_command_tests my_world_interaction_trace_tests my_world_graph_command_tests` failed RED first on missing `reconnectInputEnd`, `reconnectOutputBeginning`, and `splitEdgeWithNode`
- `cmake --build build --target my_world_t3_t5_command_tests my_world_interaction_trace_tests`
- `./build/my_world_t3_t5_command_tests`
- `./build/my_world_graph_command_tests`
- `./build/my_world_interaction_trace_tests`
- `ctest --test-dir build --output-on-failure -R "t3_t5_commands|interaction_traces|graph_commands"`

Latest accepted result:
- `t3 t5 commands ok`
- `graph commands ok`
- `interaction traces ok`
- `3/3 focused tests passed`

Next selectable TiXL lane:
- P-OPS1B hidden input / multi-input / drag-existing-node / snap-unsnap / shake disconnect continuation
- P-OUT1 output pinning

Previous closure:
None after P-SEARCH1 browser/search/compatible create fixture closure as of 2026-05-25 02:27 Asia/Taipei.

P-SEARCH1 closed.
Evidence:
- source/core/NodeSpecBrowser.h
- source/core/NodeSpecBrowser.cpp
- tests/NodeSpecBrowserTests.cpp
- fixtures/interaction/tixl-search-compatible-create.behavior.json
- docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md
- docs/superpowers/specs/2026-05-24-tixl-taxonomy-parity-spec.md

Closed line:
TiXL-style fuzzy search + namespace/description/alias match
-> native NodeSpec browser entries
-> compatible create candidates from dragged output or input context
-> behavior fixture rows for output drag, input drag, and cancel-without-mutation

Latest verification:
- `cmake -S . -B build && cmake --build build --target my_world_node_spec_browser_tests` failed RED first on missing `source/core/NodeSpecBrowser.h`
- `cmake --build build --target my_world_node_spec_browser_tests`
- `./build/my_world_node_spec_browser_tests`
- `./build/my_world_interaction_trace_tests`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `node spec browser tests ok`
- `interaction traces ok`
- `55/55 tests passed`

Previous closure:
None after P-TAX1 category browser fixture closure as of 2026-05-25 02:16 Asia/Taipei.

P-TAX1 closed.
Evidence:
- docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md
- docs/superpowers/specs/2026-05-24-tixl-taxonomy-parity-spec.md
- fixtures/tixl-witness/operator-browser-taxonomy.json
- tests/TiXLTaxonomyFixtureTests.cpp

Closed line:
TiXL `Operators/Lib` witness catalog
-> deterministic default browser taxonomy fixture
-> C++ fixture test
-> TAX-001/TAX-002/TAX-003 proof evidence

Latest verification:
- `cmake -S . -B build`
- `cmake --build build --target my_world_tixl_taxonomy_fixture_tests`
- `ctest --test-dir build --output-on-failure -R tixl_taxonomy_fixture`
- `./build/my_world_tixl_taxonomy_fixture_tests`

Latest accepted result:
- `tixl_taxonomy_fixture` passed
- TAX-001 through TAX-003 are proven at L1 witness level

Previous closure:
None after MainComponent 1-4 adapter cleanup as of 2026-05-25 02:05 Asia/Taipei.

Active work service step closed; the requested 1-4 cleanup sequence is complete.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-active-work-service-cleanup.md
- docs/superpowers/specs/2026-05-25-app-path-policy-cleanup.md
- docs/superpowers/specs/2026-05-25-proof-status-facade-cleanup.md
- docs/superpowers/specs/2026-05-25-startup-proof-adapter-cleanup.md

Closed support line:
MainComponent visible save/publish helper bodies
-> ActiveWorkService
-> MainComponent trigger/status adapter
-> existing saveWork / publishModule command paths

Latest verification:
- `cmake -S . -B build`
- `cmake --build build --target my_world_active_work_service_tests my-world`
- `ctest --test-dir build --output-on-failure -R active_work_service`
- `ctest --test-dir build --output-on-failure -R "active_work_service|save_work_command|module_publish"`
- `git diff --check`

Latest accepted result:
- app build passed
- `active_work_service` passed
- `save_work_command`, `module_publish`, `c5_module_publish_proof_runner`, and `active_work_service` passed together
- `git diff --check` passed

Previous closure:
App path policy step closed as of 2026-05-25 01:58 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-app-path-policy-cleanup.md

Closed support line:
MainComponent project/debug/candidate path helpers
-> AppPaths
-> MainComponent trigger/status adapter
-> existing proof output directories and fixture candidate roots

Latest verification:
- `cmake --build build --target my-world`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c2-storage-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-visible-module-publish-proof-and-exit`
- `git diff --check`

Latest accepted result:
- app build passed
- C2 CLI proof exited 0 and wrote `debug/c2-storage-proof/reload_report.json` plus `saved_main.patch.json`
- C5 visible module publish proof CLI exited 0 and wrote `debug/c5-visible-module-publish-proof/visible_module_publish_report.json`
- `git diff --check` passed

Previous closure:
Proof status facade step closed as of 2026-05-25 01:54 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-proof-status-facade-cleanup.md

Closed support line:
MainComponent repeated proof result status blocks
-> finishProofDump()
-> MainComponent status adapter
-> existing proof runners and artifacts

Latest verification:
- `cmake --build build --target my-world`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-attack-detector-proof-and-exit`
- `git diff --check`

Latest accepted result:
- app build passed
- PV attack detector proof CLI exited 0 and wrote `debug/pv-attack-detector-proof/attack_detector_report.json`, `cook_order.json`, `errors.json`, and `node_stats.json`
- `git diff --check` passed

Previous closure:
Startup proof adapter step closed as of 2026-05-25 01:51 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-startup-proof-adapter-cleanup.md

Closed support line:
MainComponent constructor CLI proof booleans
-> StartupProofOptions
-> startupProofTasks
-> MainComponent trigger adapter
-> existing proof runners and artifacts

Latest verification:
- `cmake -S . -B build`
- `cmake --build build --target my_world_startup_proof_tests my-world`
- `ctest --test-dir build --output-on-failure -R startup_proof`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c2-storage-proof-and-exit`
- `git diff --check`

Latest accepted result:
- `startup_proof` passed
- app build passed
- C2 CLI proof exited 0 and wrote `debug/c2-storage-proof/reload_report.json` plus `saved_main.patch.json`
- `git diff --check` passed

Previous closure:
None after ProofRunSupport C5/C6 cleanup as of 2026-05-25 01:40 Asia/Taipei.

ProofRunSupport C5/C6 cleanup closed as of 2026-05-25 01:40 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-proof-run-support-c5-c6.md

Closed support line:
C5/C6 proof runners
-> ProofRunSupport
-> proof text file writing / output directory clear/create / candidate path dedupe
-> existing C5/C6 artifacts and CLI flags

Latest verification:
- `cmake -S . -B build`
- `cmake --build build --target my_world_c5_module_publish_proof_runner_tests my_world_c6_analyzer_family_proof_runner_tests my_world_c6_ai_repair_loop_proof_runner_tests my_world_proof_run_support_tests`
- `ctest --test-dir build --output-on-failure -R "proof_run_support|c5_module_publish_proof_runner|c6_analyzer_family_proof_runner|c6_ai_repair_loop_proof_runner|module_publish|ai_worker_command|analyzer_compound_family|runtime_registry|compound_module"`
- `cmake --build build --target my-world`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-module-publish-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-ai-worker-module-publish-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-visible-module-publish-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-analyzer-family-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-ai-repair-loop-proof-and-exit`
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `proof_run_support` passed
- `c5_module_publish_proof_runner` passed
- `c6_analyzer_family_proof_runner` passed
- `c6_ai_repair_loop_proof_runner` passed
- focused module publish, AI worker, analyzer family, runtime registry, and compound module tests passed
- app build passed
- all three C5 CLI proofs exited 0 and kept `ok: true`, expected `kind`, `operation: "publish_module"`, and empty `error`
- C6 analyzer family CLI proof exited 0 and kept `ok: true`, `familyEntryCount: 2`, `runtimeCoverageStatus: "ready"`, `createdRawEnergyNode: true`, and `loudnessStillPresent: true`
- C6 AI repair-loop CLI proof exited 0 and kept `ok: true`, `status: "repaired"`, `attemptsRun: 2`, `maxAttempts: 3`, `successfulAttemptIndex: 2`, `finalCommandLogStatus: "ai_worker_repair_loop:repaired"`, `graphMutationApplied: true`, and `collaborationLogEntries: 7`
- full `ctest` passed 51/51
- `git diff --check` passed

Parked:
- `ProofRunSupport` remains a small helper layer, not a generic proof runner
- Future cleanup should target adapter duplication in `MainComponent`

Previous closure:
ProofRunSupport PV/PV-B1 cleanup closed as of 2026-05-25 01:30 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-proof-run-support-pv-pvb1.md

Closed support line:
PV/PV-B1 proof runners
-> ProofRunSupport
-> proof text file writing / output directory clear/create / candidate path dedupe
-> existing PV detector and PV-B1 artifacts and CLI flags

Latest verification:
- `cmake -S . -B build`
- `cmake --build build --target my_world_pv_detector_proof_runner_tests my_world_pv_b1_analyzer_environment_proof_runner_tests my_world_proof_run_support_tests`
- `ctest --test-dir build --output-on-failure -R "proof_run_support|pv_detector_proof_runner|pv_b1_analyzer_environment_proof_runner|analyzer_detector|analyzer_density_detector|analyzer_silence_detector|analyzer_sustain_detector|analyzer_residue_detector|analyzer_aggregate_pressure|analyzer_visible_catalog"`
- `cmake --build build --target my-world`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-attack-detector-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-density-detector-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-silence-detector-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-sustain-detector-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-residue-detector-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-aggregate-pressure-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-b1-analyzer-environment-proof-and-exit`
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `proof_run_support` passed
- `pv_detector_proof_runner` passed
- `pv_b1_analyzer_environment_proof_runner` passed
- detector and visible catalog focused tests passed
- app build passed
- all six PV detector CLI proofs exited 0 and kept `ok: true`, expected `kind`, expected `operation`, and empty `error`
- PV-B1 CLI proof exited 0 and kept `ok: true`, `loadedModuleNodeCount: 8`, `visibleCatalogContainsAllRequired: true`, `runtimeDiagnosticsReadyForAllRequired: true`, and `createdNodeCount: 8`
- full `ctest` passed 51/51
- `git diff --check` passed

Parked:
- Do not migrate all proof helpers in one sweep
- C5/C6 helper cleanup needs separate selected slices
- Support helpers are not a new generic proof runner

Previous closure:
ProofRunSupport A1/C2 cleanup closed as of 2026-05-25 01:22 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-proof-run-support-a1-c2.md

Closed support line:
A1/C2 proof runners
-> ProofRunSupport
-> proof text file writing / output directory setup / candidate path dedupe
-> existing A1/C2 artifacts and CLI flags

Latest verification:
- `cmake -S . -B build`
- `cmake --build build --target my_world_a1_audio_proof_runner_tests my_world_c2_storage_proof_runner_tests my_world_proof_run_support_tests`
- `ctest --test-dir build --output-on-failure -R "proof_run_support|a1_audio_proof_runner|c2_storage_proof_runner|runtime_registry|performance_preferences|storage_contract|patch_document"`
- `cmake --build build --target my-world`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-audio-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c2-storage-proof-and-exit`
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `proof_run_support` passed
- `a1_audio_proof_runner` passed
- `c2_storage_proof_runner` passed
- `runtime_registry` passed
- `performance_preferences` passed
- `storage_contract` passed
- `patch_document` passed
- app build passed
- A1 CLI proof exited 0 and kept runtime `kind: "runtimeExecution"` plus `nodeType: "compound.loudness"`
- C2 CLI proof exited 0 and kept `ok: true`, `saveStatus: "save-ok commit-pending"`, public input/output edges, and matching expanded layout
- full `ctest` passed 51/51
- `git diff --check` passed

Parked:
- Do not migrate all proof helpers in one sweep
- C5/C6/PV/PV-B1 helper cleanup needs separate selected slices
- Support helpers are not a new generic proof runner

Previous closure:
ProofRunSupport C3/C4 cleanup closed as of 2026-05-25 01:15 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-proof-run-support-c3-c4.md

Closed support line:
C3/C4 proof runners
-> ProofRunSupport
-> proof text file writing / output directory setup / candidate path dedupe / first candidate copy
-> existing C3/C4 reports and CLI flags

Latest verification:
- `cmake -S . -B build`
- `cmake --build build --target my_world_proof_run_support_tests my_world_c3_save_work_proof_runner_tests my_world_c4_ai_worker_save_work_proof_runner_tests`
- `ctest --test-dir build --output-on-failure -R "proof_run_support|c3_save_work_proof_runner|c4_ai_worker_save_work_proof_runner|save_work_command|ai_worker_command"`
- `cmake --build build --target my-world`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c3-save-work-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c4-ai-worker-save-work-proof-and-exit`
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `proof_run_support` passed
- `c3_save_work_proof_runner` passed
- `c4_ai_worker_save_work_proof_runner` passed
- `save_work_command` passed
- `ai_worker_command` passed
- app build passed
- C3 CLI proof exited 0 and kept `ok: true`, `saveStatus: "save-ok commit-pending"`, `commandLogStatus: "save_work:save-ok commit-pending"`, and `commitStatus: "not-started"`
- C4 CLI proof exited 0 and kept `ok: true`, `status: "save-ok commit-pending"`, `graphMutationApplied: true`, `patchReloaded: true`, and `collaborationLogEntries: 4`
- full `ctest` passed 51/51
- `git diff --check` passed

Parked:
- Do not migrate all proof helpers in one sweep
- Support helpers are not a new generic proof runner
- Remaining proof cleanup needs separate selected slices

Previous closure:
V1 shader proof artifact extraction closed as of 2026-05-25 01:05 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-v1-shader-proof-artifact-extraction.md

Closed proof line:
OpenGLShaderPreview live frame capture
-> V1ShaderProofArtifactRequest
-> writeV1ShaderProofArtifacts()
-> frame.png + 13 JSON artifacts
-> existing proof status callback

Latest verification:
- `cmake -S . -B build` red before artifact writer source existed, then green after implementation
- `cmake --build build --target my_world_v1_shader_proof_artifacts_tests`
- `ctest --test-dir build --output-on-failure -R "v1_shader_proof_artifacts|runtime_registry|opengl_render_backend|render_backend_contract"`
- `cmake --build build --target my-world`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit`
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `v1_shader_proof_artifacts` passed
- `runtime_registry` passed
- `opengl_render_backend` passed
- `render_backend_contract` passed
- app build passed
- CLI V1 shader proof exited 0, wrote all 14 artifacts, and kept nonempty `frame.png`, `renderer: "OpenGL"`, `kind: "runtimeExecution"`, `nodeType: "compound.loudness"`, and missing-runtime-op evidence
- full `ctest` passed 50/50
- `git diff --check` passed

Parked:
- V1 still depends on live OpenGL for `frame.png`
- headless visual proof / future Metal work remains a separate selected lane

Previous closure:
A1 audio proof harness extraction closed as of 2026-05-25 00:54 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-a1-audio-proof-harness-extraction.md

Closed proof line:
A1AudioProofRunRequest
-> runA1AudioProof()
-> analyzer snapshot + loaded runtime execution
-> audio_stats.json / loudness_compound.json / loudness_runtime_execution.json / loudness_runtime_bridge.json
-> MainComponent status facade

Latest verification:
- `cmake -S . -B build` red before runner source existed, then green after implementation
- `cmake --build build --target my_world_a1_audio_proof_runner_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "a1_audio_proof_runner|audio_analyzer_state|runtime_registry|performance_preferences"`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-audio-proof-and-exit`
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `a1_audio_proof_runner` passed
- `audio_analyzer_state` passed
- `runtime_registry` passed
- `performance_preferences` passed
- app build passed
- CLI A1 audio proof exited 0 and wrote all four artifacts with `kind: "runtimeExecution"`, `nodeType: "compound.loudness"`, and `kind: "loudnessRuntimeBridge"`
- full `ctest` passed 49/49
- `git diff --check` passed

Parked:
- V1 shader proof remains tied to `OpenGLShaderPreview`

Previous closure:
C2 storage proof harness extraction closed as of 2026-05-25 00:48 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-c2-storage-proof-harness-extraction.md

Closed proof line:
C2StorageProofRunRequest
-> runC2StorageProof()
-> PatchDocument save + reload proof
-> reload_report.json / saved_main.patch.json
-> MainComponent status facade

Latest verification:
- `cmake -S . -B build` red before runner source existed, then green after implementation
- `cmake --build build --target my_world_c2_storage_proof_runner_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "c2_storage_proof_runner|storage_contract|patch_document"`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c2-storage-proof-and-exit`
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `c2_storage_proof_runner` passed
- `storage_contract` passed
- `patch_document` passed
- app build passed
- CLI C2 storage proof exited 0 and wrote `debug/c2-storage-proof/reload_report.json` with `ok: true`, `saveStatus: "save-ok commit-pending"`, public input/output edges, and matching expanded layout
- full `ctest` passed 48/48
- `git diff --check` passed

Parked:
- V1 shader proof remains tied to `OpenGLShaderPreview`
- A1 audio proof remains in `MainComponent`

Previous closure:
C3 save-work proof harness extraction closed as of 2026-05-25 00:43 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-c3-save-work-proof-harness-extraction.md

Closed proof line:
C3SaveWorkProofRunRequest
-> runC3SaveWorkProof()
-> direct moveNode + saveWork proof
-> save_work_report.json
-> MainComponent status facade

Latest verification:
- `cmake -S . -B build` red before runner source existed, then green after implementation
- `cmake --build build --target my_world_c3_save_work_proof_runner_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "c3_save_work_proof_runner|save_work_command|patch_document"`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c3-save-work-proof-and-exit`
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `c3_save_work_proof_runner` passed
- `save_work_command` passed
- `patch_document` passed
- app build passed
- CLI C3 save-work proof exited 0 and wrote `debug/c3-save-work-proof/save_work_report.json` with `ok: true`, `saveStatus: "save-ok commit-pending"`, `commandLogStatus: "save_work:save-ok commit-pending"`, and `commitStatus: "not-started"`
- full `ctest` passed 47/47
- `git diff --check` passed

Parked:
- C2 proof orchestration remains in `MainComponent`
- V1 shader proof remains tied to `OpenGLShaderPreview`

Previous closure:
C4 AI worker save-work proof harness extraction closed as of 2026-05-25 00:36 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-c4-ai-worker-save-work-proof-harness-extraction.md

Closed proof line:
C4AIWorkerSaveWorkProofRunRequest
-> runC4AIWorkerSaveWorkProof()
-> AI worker move_node + save_work proof
-> ai_worker_save_work_report.json
-> MainComponent status facade

Latest verification:
- `cmake -S . -B build` red before runner source existed, then green after implementation
- `cmake --build build --target my_world_c4_ai_worker_save_work_proof_runner_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "c4_ai_worker_save_work_proof_runner|ai_worker_command|save_work_command"`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c4-ai-worker-save-work-proof-and-exit`
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`

Latest accepted result:
- `c4_ai_worker_save_work_proof_runner` passed
- `ai_worker_command` passed
- `save_work_command` passed
- app build passed
- CLI C4 save-work proof exited 0 and wrote `debug/c4-ai-worker-save-work-proof/ai_worker_save_work_report.json` with `ok: true`, `status: "save-ok commit-pending"`, `patchReloaded: true`, and `collaborationLogEntries: 4`
- full `ctest` passed 46/46
- `git diff --check` passed

Parked:
- C2-C3 proof orchestration remains in `MainComponent`
- V1 shader proof remains tied to `OpenGLShaderPreview`

Previous closure:
C6 AI repair-loop proof harness extraction closed as of 2026-05-25 00:15 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-c6-ai-repair-loop-proof-harness-extraction.md

Closed proof line:
C6AIRepairLoopProofRunRequest
-> runC6AIRepairLoopProof()
-> AI worker repair plan + retry execution
-> ai_repair_loop_report.json
-> MainComponent status facade

Latest verification:
- `cmake -S . -B build` red before runner source existed, then green after implementation
- `cmake --build build --target my_world_c6_ai_repair_loop_proof_runner_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "c6_ai_repair_loop_proof_runner|ai_worker_command"`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-ai-repair-loop-proof-and-exit`

Latest accepted result:
- `c6_ai_repair_loop_proof_runner` passed
- `ai_worker_command` passed
- app build passed
- CLI C6 repair proof exited 0 and wrote `debug/c6-ai-repair-loop-proof/ai_repair_loop_report.json` with `ok: true`, `status: "repaired"`, `attemptsRun: 2`, and `collaborationLogEntries: 7`

Parked:
- C2-C4 proof orchestration remains in `MainComponent`
- V1 shader proof remains tied to `OpenGLShaderPreview`

Previous closure:
C5 proof harness extraction closed as of 2026-05-25 00:08 Asia/Taipei.
Spec / closure evidence:
- docs/superpowers/specs/2026-05-25-c5-proof-harness-extraction.md

Closed proof line:
C5ModulePublishProofRunRequest
-> runC5ModulePublishProof()
-> direct / AI worker / visible module publish proof
-> module_publish_report.json / ai_worker_module_publish_report.json / visible_module_publish_report.json
-> MainComponent status facade

Latest verification:
- `cmake -S . -B build` red before runner source existed, then green after implementation
- `cmake --build build --target my_world_c5_module_publish_proof_runner_tests`
- `cmake --build build --target my-world`
- `ctest --test-dir build --output-on-failure -R "c5_module_publish_proof_runner|module_publish|ai_worker_command"`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-module-publish-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-ai-worker-module-publish-proof-and-exit`
- `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-visible-module-publish-proof-and-exit`

Latest accepted result:
- `module_publish` passed
- `ai_worker_command` passed
- `c5_module_publish_proof_runner` passed
- app build passed
- all three C5 CLI proofs exited 0 and wrote reports with `ok: true`

Parked:
- C2-C4 proof orchestration remains in `MainComponent`
- V1 shader proof remains tied to `OpenGLShaderPreview`

Previous closure:
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
- C2-C4 proof orchestration remains in `MainComponent`
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
| `2026-05-25-g1-graph-io-mapping-foundation.md` | G1 graph IO mapping foundation closure evidence | no, unless auditing G1 evidence |
| `2026-05-25-app1-workbench-session-spine.md` | APP1 workbench session closure evidence | no, unless auditing APP1 evidence |
| `2026-05-25-app2-workbench-open-status-spine.md` | APP2 workbench open/status closure evidence | no, unless auditing APP2 evidence |
| `2026-05-25-app3-workbench-app-controller-spine.md` | APP3 workbench app controller closure evidence | no, unless auditing APP3 evidence |
| `2026-05-25-app4-active-work-source-truth.md` | APP4 active work source truth closure evidence | no, unless auditing APP4 evidence |
| `2026-05-25-app5-stable-workbench-proof-alias.md` | APP5 stable workbench proof alias closure evidence | no, unless auditing APP5 evidence |
| `2026-05-25-app6-stable-workbench-proof-runner.md` | APP6 stable workbench proof runner closure evidence | no, unless auditing APP6 evidence |
| `2026-05-25-app7-workbench-session-spine-closure.md` | APP7 closure marker for APP workbench session spine | no, unless auditing APP segment closure |
| `2026-05-25-work1-active-work-lifecycle-status.md` | WORK1 active work lifecycle status closure evidence | no, unless auditing WORK1 evidence |
| `2026-05-25-work2-active-work-resolver.md` | WORK2 active work resolver closure evidence | no, unless auditing WORK2 evidence |
| `2026-05-25-work3-work-diagnostics-report.md` | WORK3 work diagnostics report closure evidence | no, unless auditing WORK3 evidence |
| `2026-05-25-work4-app-status-wording.md` | WORK4 app status wording closure evidence | no, unless auditing WORK4 evidence |
| `2026-05-25-work5-work-segment-closure.md` | WORK5 closure marker for WORK1-WORK4 app work-source reporting segment | no, unless auditing WORK segment closure |

## Session Safety

WORK5 owned files:

```text
docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md
docs/superpowers/specs/2026-05-25-work5-work-segment-closure.md
```

Avoid unrelated files and parked lanes:

```text
AGENTS.md
docs/superpowers/handoffs/2026-05-24-repo-relocation-note.md
docs/superpowers/plans/2026-05-24-flow-runner-automation.md
docs/superpowers/specs/2026-05-24-post-c-three-layer-cleanup.md
docs/superpowers/specs/2026-05-24-r-runtime-render-backbone-roadmap.md
docs/superpowers/plans/2026-05-24-r-segment-implementation.md
scripts/
tests/test_myworld_flow.py
```

## Next Handoff Sentence

Open this master plan first. Active lane is `None` after WORK5 work segment closure. APP1-APP7 and WORK1-WORK5 are local-only commits; do not push unless explicitly requested. Treat the APP workbench session spine and WORK app work-source reporting segment as closed; choose a fresh lane name by the next selected feature surface, such as STATUS, PROOF, UI, GRAPH, RUNTIME, or ACTIVE. Do not add analyzer DSP, full mapping editor, graph UI node surface, extra MIDI operators, external OSC/UDP UI lifecycle, direct realtime MIDI/OSC send, shader preview live binding expansion, browser polish, Metal, image.blur, interactive node thumbnails, SOP/MAT/POINT, full render export, TiXL runtime work beyond the selected lane, relocation note, flow-runner files, `scripts/`, `tests/test_myworld_flow.py`, or `AGENTS.md`.

## Next Master-Plan Maintenance

After the next lane finishes:

1. Re-check `git status -sb`.
2. Read the lane closure spec or create it if missing.
3. Update this file:
   - move the active lane from `in progress` to `closed` or `blocked`;
   - record proof commands;
   - set the next active lane explicitly.
4. Do not start broader MIDI teach work, shader mapping, live callback-buffer runtime, external OSC/UDP targets, TiXL, browser polish, or another analyzer family until a new active lane row is updated.
