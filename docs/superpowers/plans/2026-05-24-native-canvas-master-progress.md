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

Date: 2026-05-24 16:02 Asia/Taipei.

Branch:

```text
codex/tooll3-interaction-t0-t7
```

Latest known commits:

```text
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
| V1 visual proof | proven | `--dump-proof-and-exit` | R lane will absorb runtime/render ownership later |
| A1 audio proof | proven | `--dump-audio-proof-and-exit` | Raw callback-buffer runtime remains parked |
| C1 compound core | closed | `docs/superpowers/specs/2026-05-24-c1-24-compound-proof-closure.md` | Do not reopen C1 |
| C2 PatchDocument work persistence | closed | `docs/superpowers/specs/2026-05-24-c2-compound-work-closure.md` | Do not reopen C2 |
| C3 storage command path | closed | `docs/superpowers/specs/2026-05-24-c3-storage-command-path.md` | Do not reopen C3 |
| C4 AI worker command path | closed | `docs/superpowers/specs/2026-05-24-c4-ai-worker-command-contract.md` | Do not reopen C4 |
| C5 module publish/reuse path | closed | `docs/superpowers/specs/2026-05-24-c5-module-publish-reuse-path.md` | Do not reopen C5 for C6 work |
| C6.1 analyzer compound family seed | closed | `docs/superpowers/specs/2026-05-24-c6-analyzer-compound-family.md` | do not reopen for detector semantics |
| C6.2 AI repair loop closure | closed | `docs/superpowers/specs/2026-05-24-c6-analyzer-compound-family.md` | do not reopen for natural-language parsing |
| H1 post-C three-layer cleanup | closed | `docs/superpowers/specs/2026-05-24-post-c-three-layer-cleanup.md` | Do not reopen for R/PV/TiXL work |
| R runtime/render backbone | closed | R1 closed: OpenGL proof now runs behind `RenderBackend`; R2 closed: headless `image.constant -> output.texture_summary` writes debug artifacts; R3 closed: V1/R2 proof JSON fields are compatible; `ctest` 33/33 | Select next lane explicitly |
| TiXL parity | ledgered, not main spine | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md` | P-TAX1 when not colliding with C4 |

## Active Lane Protocol

Only one lane should be marked `in progress` in this file unless the files are disjoint.

Current active lane:

```text
None after R3 closure.
H1.1 UI overlay split is closed.
H1.2 RuntimeRegistry split is closed.
H1.3 StorageContract split is closed in the current slice.
R0 runtime/render backbone roadmap spec is closed in the current slice.
R1 OpenGL proof backend extraction is closed in d884f2d.
R2 headless image.constant runtime proof is closed in ebeab9f.
R3 runtime/render proof compatibility closure is closed in the current slice.
No active implementation plan until the next lane is explicitly selected.
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
attack / density / sustain / silence compound semantics
multi-node group-to-compound extraction
publish dialog / browser polish beyond the visible proof hand
remote sync / shared module registry
RenderBackend extraction
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
| `2026-05-24-post-c-three-layer-cleanup.md` | H1 closure evidence | no, unless auditing H1 evidence |
| `2026-05-24-r-runtime-render-backbone-roadmap.md` | R0 roadmap spec and R1/R2 routing authority | only after this master plan selects R work |
| `2026-05-24-r-segment-implementation.md` | active R implementation plan | yes, for R1/R2/R3 execution |
| `2026-05-24-flow-runner-automation.md` | untracked separate flow-runner lane owned outside C6 | no |
| `2026-05-24-tixl-parity-construction-ledger.md` | active sub-ledger for TiXL-visible parity | only from this master plan |

## Session Safety

Files outside C cleanup ownership that this pass must not edit or include unless explicitly requested:

```text
AGENTS.md
docs/superpowers/plans/2026-05-24-flow-runner-automation.md
scripts/
tests/test_myworld_flow.py
```

H cleanup owned files:

```text
docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md
docs/superpowers/specs/2026-05-24-post-c-three-layer-cleanup.md
source/core/RuntimeRegistry.*
source/storage/StorageContract.*
source/ui/ImGuiSmokeOverlay.*
CMakeLists.txt
```

R0 routing owned files:

```text
docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md
docs/superpowers/specs/2026-05-24-r-runtime-render-backbone-roadmap.md
docs/superpowers/plans/2026-05-24-r-segment-implementation.md
```

## Next Handoff Sentence

Open this master plan first. R runtime/render backbone is closed through R3; select the next lane explicitly before editing. Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, and PV/TiXL work remain parked until selected. Do not touch the flow-runner files, `scripts/`, `tests/test_myworld_flow.py`, or `AGENTS.md`.

## Next Master-Plan Maintenance

After the next lane finishes:

1. Re-check `git status -sb`.
2. Read the lane closure spec or create it if missing.
3. Update this file:
   - move the active lane from `in progress` to `closed` or `blocked`;
   - record proof commands;
   - set the next active lane explicitly.
4. Do not start R/PV/TiXL implementation until the active lane row is updated.
