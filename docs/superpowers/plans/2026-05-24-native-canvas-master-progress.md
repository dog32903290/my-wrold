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

Date: 2026-05-24 11:57 Asia/Taipei.

Branch:

```text
codex/tooll3-interaction-t0-t7
```

Latest known commits:

```text
ec3efe2 Close C4 AI worker command proof
d73b893 Add C4.2 AI worker move_node command
835ce85 Add C4.1 AI worker save_work contract
09edd86 Add C3.5 save commit failure proof
57ba0c5 Add C3.4 background save commit worker
f163d80 Wire C3.3 visible save_work hand
fd2976d Add C3.2 save_work app proof
f872999 Add C3.1 save_work command contract
f7fbc10 Advance native canvas proof spine
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
C5.2 AI worker publish_module command path is closed locally.
Active C5 spec:
- docs/superpowers/specs/2026-05-24-c5-module-publish-reuse-path.md
Current C5 evidence is in:
- tests/ModulePublishTests.cpp
- tests/AIWorkerCommandTests.cpp
- debug/c5-module-publish-proof/module_publish_report.json
- debug/c5-ai-worker-module-publish-proof/ai_worker_module_publish_report.json

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
| C5 module publish/reuse path | active | `docs/superpowers/specs/2026-05-24-c5-module-publish-reuse-path.md` | C5.3 visible publish hand |
| C6 analyzer compound family / AI repair loop closure | planned | not yet specified | split if C5 grows too large |
| R runtime/render backbone | roadmap only | skeleton spec parks RenderBackend/Metal | write roadmap spec after C lane stabilizes |
| TiXL parity | ledgered, not main spine | `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md` | P-TAX1 when not colliding with C4 |

## Active Lane Protocol

Only one lane should be marked `in progress` in this file unless the files are disjoint.

Current active lane:

```text
C5.3 visible publish hand
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

Park during C5:

```text
AI repair loop / retry policy
AI worker publish_module command
new analyzer compound family
multi-node group-to-compound extraction
publish dialog / browser polish
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
| C4 closes save_work and move_node only, not the full AI worker loop | C4 spec vs future AI worker expectations | Natural language parsing, additional graph mutation commands, repair loop, and remote sync remain parked outside C4. |
| C5 can be mistaken for new analyzer/module vocabulary work | C5 title vs C6 planned analyzer family | C5.1 only proves publishing and reusing one selected compound source. New analyzer families and repair loop remain C6+. |

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
| `2026-05-24-c5-module-publish-reuse-path.md` | active C5 slice spec | yes, after this master plan |
| `2026-05-24-tixl-parity-construction-ledger.md` | active sub-ledger for TiXL-visible parity | only from this master plan |

## Next Master-Plan Maintenance

After the next lane finishes:

1. Re-check `git status -sb`.
2. Read the lane closure spec or create it if missing.
3. Update this file:
   - move the active lane from `in progress` to `closed` or `blocked`;
   - record proof commands;
   - set the next active lane explicitly.
4. Do not start R/PV/TiXL implementation until the active lane row is updated.
