# Native Canvas Skeleton Design

Date: 2026-05-22
Status: V1 shader preview proof, S0 storage proof, G0 graph language proof, A0 ImGui workspace, A1 audio/MIDI proof, C1.1-C1.24 loudness compound proof closure, Tooll3 T0-T7 interaction core, visible T0-T7 canvas workspace, Tooll3 skin parity P0-P7 first pass, H1-H4 hygiene cleanup, C2.1-C2.4 compound work PatchDocument closure, C3.1-C3.5 storage command path closure, and C4.1-C4.3 AI worker command path closure are implemented and verified. Raw callback-buffer runtime execution, RenderBackend extraction, production node previews, 13-patch analyzer expansion, AI worker natural-language planning/repair loop, remote sync, full JSON parser/library replacement, broad enum/hash typing, and full ImGui component split are parked as later high-risk work.

## Purpose

`我的世界` is the new native body for 柏為的畫布.

This is not a Web wrapper and not a generic C++ port. The first skeleton must prove that a local app can host live shader editing, native audio analysis, compound patchers, and eventually AI worker collaboration without inheriting LiteGraph as the law.

## Locked Decisions

- The repo lives at `/Users/chenbaiwei/Desktop/我的世界`.
- Use JUCE + CMake for the first native app skeleton.
- Build a standalone app first, not a plugin.
- Use display name `我的世界`; keep internal target names ASCII, starting with `my-world`.
- Use OpenGL/GLSL only as the first proof backend to preserve GLSL vibe-coding speed.
- Treat `我的世界` as a Mac-first native instrument. The production GPU direction is Metal, not MoltenVK/Vulkan.
- Extract the current preview behind `RenderBackend` before building high-resolution node previews, compute-heavy shader graphs, Metal backend work, or performance claims.
- Park WGPU/bgfx/MoltenVK unless cross-platform pressure becomes a real product requirement. DirectX/HLSL remains out of scope for this app body.
- Old Web canvas works are migration fixtures, not the native graph law.
- Tooll3 / TiXL is the primary reference for ImGui canvas style, Symbol/Instance separation, JSON graph shape, parameter override behavior, and panel rhythm. It is not a codebase to fork into this repo.
- CodeGraph is a local development index for C++ symbol, caller/callee, and impact lookup. `.codegraph/` is ignored and must not become source or graph contract data.

## Current Progress Snapshot

Date: 2026-05-24 11:36 Asia/Taipei.

目前工程切面:

```text
V1 visual proof      proven
A1 audio/MIDI proof proven through snapshot-shaped loaded-runtime bridge
C1 compound proof   closed through loaded runtime, public ports, persistence, layout
H1-H4 hygiene       low/medium-risk cleanup complete and verified
C2 storage proof    closed through formal PatchDocument file save/reload and app proof dump
C3.1 save command   proven through save_work -> PatchDocument write/reload + save log
C3.2 save proof     proven through structured save log readback + app proof dump
C3.3 visible hand    proven through Save Work button / Command+S callback path
C3.4 local commit    proven through opt-in background git worker success path
C3.5 commit failure  proven through save-ok commit-failed save log readback
C4 AI command path   proven through AI move_node -> AI save_work -> PatchDocument reload + collaboration evidence
```

已鎖定:

- The node canvas is now the primary workspace surface, with `out1` rendered as the workspace background and nodes floating over it.
- Tooll3-like interaction T0-T7 is command-backed: pan/zoom, select/move, connect/disconnect, create-and-connect, compound enter/exit/collapse, inspector param/binding, dirty/save state, and behavior trace replay.
- Delete now follows selected-object behavior. A selected edge lowers to `disconnect`; a selected node lowers to `delete_node`, removes incident edges, syncs `runtimeGraph`, and preserves undo/redo.
- Tooll3 skin parity P0-P7 first pass is in place: flat dark shell, left rail, bottom strip, typed node colors, port strips, connection colors, selected-node shader source inspector, workspace browser/context menu, and transport/status strip.
- C1.3 visible module registry proof is implemented: `fixtures/modules/loudness/module.json` loads into a merged visible `NodeSpec` registry, overrides the seed `compound.loudness` spec, feeds the ImGui node browser, and creates module-backed compound nodes through the command path.
- C1.4 module-library index proof is implemented: `fixtures/module-libraries/default.module-library.json` lists module packages, storage parses `ModuleLibraryManifest`, `CompoundModule` loads a registry from that library, and the visible app startup uses the library index instead of a hardcoded module manifest path.
- C1.5 loaded compound runtime-registry proof is implemented: `RuntimeRegistry` loads the default module library into runtime entries, records `compound.patch` execution kind, public ports, child counts, and cook order, and `--dump-proof-and-exit` writes `debug/v1-shader-proof/runtime_registry.json`.
- C1.6 loaded compound dry-run proof is implemented: the runtime registry stores child metadata, `dryRunRuntimeRegistry()` walks the loaded compound cook order, and `--dump-proof-and-exit` writes `debug/v1-shader-proof/runtime_dry_run.json` with per-child `dry-run-ready` statuses.
- C1.7 first executable child RuntimeOp proof is implemented: `executeRuntimeRegistryWithSyntheticAudio()` runs the loaded `analyzer.rms` child over synthetic mono samples through `AudioAnalyzerState`, records `rms=0.707107` and `peak=1.000000`, keeps unimplemented children explicitly `not-executed`, and `--dump-proof-and-exit` writes `debug/v1-shader-proof/runtime_execution.json`.
- C1.8 loaded loudness mini-chain value-handoff proof is implemented: synthetic audio input can carry multiple channels, runtime execution now records `inputs` and `outputs`, `audio.mono_mix` feeds `analyzer.rms`, `analyzer.rms` feeds `analyzer.analysis_gain`, and proof dump records `analysis_gain.out=0.795495` for the current two-channel synthetic fixture.
- C1.9 loaded loudness public-output proof is implemented: `pre_gate`, `output_smoother`, and `loudness_out` now execute after `analysis_gain`, entry status becomes `computed`, and `runtime_execution.json` records `publicOutputs` with `out`, `rms`, `peak`, `gate`, and `confidence`.
- C1.10 loaded internal-edge value-bus proof is implemented: runtime registry entries now store loaded `internalEdges` and `publicOutputMappings`, runtime execution uses `child.port` bus keys for input lookup, and proof JSON records `inputSources` plus `publicOutputSources`.
- C1.11 RuntimeOp dispatch-table proof is implemented: loaded-compound execution now resolves each child `nodeType` through a named synthetic RuntimeOp function, and `runtime_execution.json` records each executed child's `runtimeOp` id.
- C1.12 RuntimeOp coverage failure proof is implemented: unsupported loaded child node types fail dry-run and execution as `missing-runtime-op`, keep serializable failure snapshots, and do not publish public outputs on coverage failure.
- C1.13 saved negative module fixture proof is implemented: `fixtures/module-libraries/missing-runtimeop.module-library.json` loads `compound.loudness.missing-runtimeop`, records the saved `debug.unsupported` child, and app proof dump writes `runtime_missing_runtimeop_registry.json`, `runtime_missing_runtimeop_dry_run.json`, and `runtime_missing_runtimeop_execution.json`.
- C1.14 RuntimeOp catalog coverage proof is implemented: `makeRuntimeOpCatalog()` exposes the seven supported synthetic RuntimeOps, `inspectRuntimeOpCoverage()` reports supported/missing child coverage before execution, and app proof dump writes `runtime_op_catalog.json`, `runtime_op_coverage.json`, and `runtime_missing_runtimeop_coverage.json`.
- C1.15 visible RuntimeOp diagnostics proof is implemented: `makeRuntimeOpModuleDiagnostics()` turns coverage snapshots into browser/inspector labels, app proof dump writes `runtime_ui_diagnostics.json`, and the ImGui workspace shows `runtime ready` plus `missing RuntimeOp: debug.unsupported` in the left rail without UI-only status.
- C1.16 coverage-gated module creation proof is implemented: RuntimeOp diagnostics now carry creation affordance state, `InteractionContract::NodeCreationGate` blocks missing-runtime module creation before graph mutation, and the ImGui browser/create popup passes those gates into the command path.
- C1.17 explicit debug override proof is implemented: blocked module creation can now be intentionally inserted only through `create_node_debug_override` / `create_node+connect_debug_override`, with required reason, stored debug params, distinct command log entries, and undo coverage.
- C1.18 expanded/collapsed compound drag-drop proof is implemented: `makeCompoundPatchInteractionGraph()` creates parent-qualified child patcher view graphs, CanvasHands tests drag collapsed loaded compounds and expanded child nodes, behavior traces record the roundtrip, and the ImGui canvas switches to the child graph while inside a compound.
- C1.19 loaded loudness runtime bridge proof is implemented: `makeLoudnessRuntimeBridgeSnapshot()` consumes loaded runtime public outputs when present, falls back to direct `AudioAnalyzerSnapshot` values with the same public vocabulary, and audio proof dump writes `debug/a1-audio-proof/loudness_runtime_bridge.json`.
- C1.20 live-safe loudness runtime proof is implemented: `makeRuntimeSyntheticAudioInputFromAnalyzerSnapshot()` prepares a non-realtime snapshot-shaped input, app audio proof executes the loaded loudness runtime, writes `loudness_runtime_execution.json`, and bridge JSON records `sourceMode: loaded-runtime-publicOutputs`.
- C1.21 compound public-port surface proof is implemented: `connectPorts(session, specs, from, to)` validates against the loaded visible `NodeSpec` registry, CanvasHands/ImGui connection gestures use that registry, and collapsed `compound.loudness` public input/output ports connect at root level.
- C1.22 compound public-port persistence proof is implemented: behavior trace `compound public ports persist undo` proves root public-port edges survive interaction serialize/deserialize into both editorGraph/runtimeGraph, then disconnect/undo/redo cleanly.
- C1.23 expanded child layout persistence proof is implemented: `storeExpandedPatchLayout()` stores moved expanded child positions on the parent compound instance, regenerated expanded graphs apply that layout, and visible Exit stores layout before leaving a compound patch.
- C1.24 closes the first loudness compound proof. C1 is no longer extended for raw callback-buffer runtime, patch document schema, visual mapping polish, 13-patch analyzer expansion, or AI worker modules; those are C2+.
- H1-H4 hygiene proofs are implemented: endpoint/geometry helpers now live in `GraphEndpoint` and `CanvasGeometry`, graph/runtime/compound/storage JSON emitters share `JsonWriter` for quoting and string arrays, module/runtime path lookup uses `PathResolution` with attempted-path evidence, and ImGui node-spec queries now live in `NodeSpecQueries`. Full JSON parser/library replacement remains parked until a schema boundary requires it.
- C2.1 first patch-document boundary is implemented: `PatchDocument` serializes/parses `GraphContract` as JSON, `fixtures/storage/c2-compound-work/patches/main.patch.json` loads a saved compound instance with root public-port edges, and regenerated expanded child graphs restore `mono_mix` layout without using `interaction-state-v1`.
- C2.2 active patch file roundtrip is implemented: `savePatchDocument()` writes a `PatchDocument` generated from a dirty `GraphSession` to a real file path, `loadPatchDocument()` reloads it, and a fresh `GraphSession` preserves editor/runtime edge parity, public-port edges, and expanded child layout.
- C2.3 work manifest main patch reload is implemented: `loadWorkProjectManifest()` parses `fixtures/storage/c2-compound-work/myworld.work.json`, keeps module-library refs as work-level data, and `loadMainPatchDocumentForWork()` resolves `patches/main.patch.json` relative to the work manifest.
- C2.4 app-level storage proof dump is implemented: `--dump-c2-storage-proof-and-exit` loads the C2 work fixture, saves `debug/c2-storage-proof/saved_main.patch.json`, reloads it, and writes `debug/c2-storage-proof/reload_report.json` with `ok: true`, `source: PatchDocument`, `usesInteractionState: false`, public-port evidence, and `library_loud1/mono_mix` layout evidence.
- C3.1 save_work command contract is implemented: `StorageCommand::saveWork()` takes a dirty `GraphSession` plus active `WorkProject` manifest path, writes the main patch through `savePatchDocument()`, reload-validates the saved `PatchDocument`, clears dirty state, records `save_work:save-ok commit-pending`, and appends `.myworld/save_log.jsonl`.
- C3.2 save log readback and app proof are implemented: `loadSaveLog()` reads `.myworld/save_log.jsonl` into structured entries, and `--dump-c3-save-work-proof-and-exit` writes `debug/c3-save-work-proof/save_work_report.json` proving command log status, save log status, PatchDocument reload, public ports, and expanded child layout.
- C3.3 visible save_work hand is implemented: the ImGui command strip exposes `Save Work`, `Command+S` triggers the same path, `MainComponent::saveActiveWork()` calls `saveWork()` on the active `GraphSession`, and the visible save hand no longer writes `interaction-state-v1`.
- C3.4 background local git commit success path is implemented: `SaveWorkOptions::startLocalGitCommit` makes `saveWork()` return `commit-pending` plus a `SaveWorkCommitJob`, the worker commits only the active work repo files, and the save log records the final `saved-and-committed` status plus commit id.
- C3.5 commit failure path is implemented: when the worker cannot commit, `saveWork()` still preserves the successful PatchDocument write, the commit job returns `save-ok commit-failed`, and the save log records final failure status plus error.
- C4.1-C4.3 AI worker command path is closed: `AIWorkerCommandRequest`/`AIWorkerCommandResult` define the minimal command caller shape, `allowedAIWorkerOperations()` admits `save_work` and `move_node`, `executeAIWorkerCommand()` records intent/result/proof evidence, routes `move_node` through `InteractionContract::moveNode()`, routes `save_work` through `StorageCommand::saveWork()`, and `--dump-c4-ai-worker-save-work-proof-and-exit` writes `debug/c4-ai-worker-save-work-proof/ai_worker_save_work_report.json`.
- Latest C2 verification: `cmake --build build --target my-world my_world_patch_document_tests`, `./build/my_world_patch_document_tests`, and `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c2-storage-proof-and-exit`.
- Latest C3.5 verification: `cmake --build build --target my_world_save_work_command_tests` and `./build/my_world_save_work_command_tests`.
- Latest C4 verification: `cmake --build build --target my-world my_world_ai_worker_command_tests`, `./build/my_world_ai_worker_command_tests`, and `./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c4-ai-worker-save-work-proof-and-exit`.

正在試壓:

- Whether C5 should start from module publish/reuse or first persist collaboration log evidence as `.myworld/collaboration_log.jsonl`.
- Whether P-SEARCH1 browser/search work should wait for the patch-document boundary, now that `NodeSpecQueries` exists as the small shared query helper.

還沒承重:

- True module discovery beyond the default saved `ModuleLibrary` index is not implemented.
- `RenderBackend` is still not extracted; OpenGL/GLSL remains the proof backend and Metal work stays parked.
- Timeline editing, output pinning, live node thumbnails, and AI worker graph edits beyond `move_node` do not yet have commandGraph/storage contracts.
- App audio proof feeds loaded runtime execution from a non-realtime snapshot-shaped input; raw callback-buffer capture and live UI cached runtime snapshots are still parked.
- Collapsed compound ports are command/hit-test backed and persist through interaction state; deeper visual grouping is still parked.
- Expanded child node positions persist per compound instance; expanded view pan/zoom is still session-local.
- C3.1-C3.5 close the storage command path. C4.1-C4.3 prove the first AI worker save/mutation caller path; AI natural-language parsing, repair loop, remote sync, and user-facing commit preference remain parked.
- High-risk cleanup is intentionally parked: no full ImGui component split, graph schema rewrite, JSON library swap, enum/hash migration, or ownership model rewrite until a proof line requires it.

下一根線:

- C5 candidate: `selected compound/work graph -> module publish/reuse path -> reload through ModuleLibrary`.

## First Stage Proofs

### S0 Storage Proof

Goal:

```text
work project -> atomic save -> background local git commit -> module/work library export
```

Contract:

- Planned: a work is a project folder with a manifest, one or more patch documents, assets, debug/proof outputs, and a local git history.
- Planned: a patch can be a mother patch containing smaller nodes, compound patchers, and references to saved modules.
- Planned: patch documents store parameter binding state so manual values survive connected or animated overrides.
- Planned: a selected patch or compound can be published into a module repository as a reusable module package.
- Planned: large works can be stored in a work repository/library separately from the module repository.
- Planned: `Command+S` performs an atomic save of the active work project on the UI path, then schedules the local git commit on a background worker when files changed.
- Planned: `Command+S` commits the artwork/project repository only; it must never commit this app source repository unless the user is editing this repo directly.
- Planned: auto commit is local-only. Push/sync to a remote repository must be an explicit later command.
- Planned: after the synchronous save returns, the UI can report `save-ok commit-pending`; the background worker later records `saved-and-committed` or `save-ok commit-failed`.
- Planned: if save succeeds but commit fails, the UI reports `save-ok commit-failed` and leaves evidence; it must not silently claim a full save.
- Locked: `Command+S` creates at most one local commit per explicit save gesture.
- Locked: `Command+S` never runs git add/commit on the UI thread.
- Locked: no remote push happens on save.
- Locked: no commit happens when there are no file changes.
- Locked: save log records both save result and commit result.
- Proven: minimal storage contract names WorkProject, PatchDocument, ModulePackage, WorkLibrary, and ModuleLibrary.
- Proven: Command+S save statuses are explicit: clean, save-ok commit-pending, saved-and-committed, save-ok commit-failed, validation-failed, write-failed.
- Proven: minimal work fixture stores a main patch with `shader1 -> out1` as reloadable project data.
- Proven: `ModuleLibraryManifest` serializes/parses/loads as a storage-backed index of module package paths, with `fixtures/module-libraries/default.module-library.json` as the first default library.
- Proven: C2.1-C2.4 patch documents now serialize/load `GraphContract` through `PatchDocument`, including node positions, collapsed compound state, params, port bindings, root edges, runtime graph, fixture-backed compound child layout evidence, active file save/reload, work-manifest main-patch resolution, and app-level C2 proof dumps.
- Proven: C3.1 `save_work` command writes the work main patch through the formal `PatchDocument` API, reload-validates it, records command/save log status, preserves compound public ports and expanded child layout, and leaves background git commit parked.
- Proven: C3.2 reads save logs back through `loadSaveLog()` and app-level `--dump-c3-save-work-proof-and-exit` records command/save-log/reload evidence in `debug/c3-save-work-proof/save_work_report.json`.
- Proven: C3.3 replaces the visible temporary interaction-state save buttons with `Save Work`, wires `Command+S`, and routes both through the same active-work `saveWork()` callback.
- Proven: C3.4 adds an opt-in background local git worker that commits the active work repository and records final committed status in the save log.
- Proven: C3.5 adds the commit-failed path: successful PatchDocument save remains valid, background commit failure returns `save-ok commit-failed`, and the error is readable from the save log.
- Proven: C4.1-C4.3 add the first AI worker command path: `executeAIWorkerCommand()` records AI intent/result evidence in `commandLog` and `collaborationLog`, calls `InteractionContract::moveNode()` for mutation, calls `StorageCommand::saveWork()` for saving, and proves the moved node remains in the reloaded `PatchDocument`.
- Forbidden: graph state that only exists inside UI widgets, ImGui ids, or in-memory node objects.

Current storage execution plan:

```text
docs/superpowers/plans/2026-05-22-s0-storage-proof.md
```

### G0 Graph Language Contract

Goal:

```text
NodeSpec + RegionSpec + TypeSpec + StreamKind + Command schema -> typed graph IR
```

Contract:

- Planned: graph objects include nodes and regions; `NodeSpec` is not the only visible or serializable graph object.
- Planned: `RegionSpec` supports future `if`, `for_each`, `repeat`, and `while` control blocks without spaghetti node wiring.
- Planned: `TypeSpec` distinguishes values such as `audio.channels`, `audio.mono`, `signal.float`, `texture.rgba`, `geometry.mesh`, `event.midi`, `command.graph`, and future generics.
- Planned: `StreamKind` distinguishes continuous data, event streams, command streams, and resources.
- Planned: edges carry `dataType` and `streamKind`, not only `from` / `to`.
- Planned: graph validation produces a typed graph IR / AST before runtime execution or code generation.
- Planned: AI worker mutates graph state only through typed commands such as `create_node`, `create_region`, `connect`, `set_param`, `publish_module`, and `save_work`.
- Proven: the first AI worker allowed-operation skeleton exists for `save_work` and `move_node`, with request/result shape and command/collaboration evidence. Natural language parsing, repair loops, remote sync, and additional graph mutation commands are not part of C4.
- Planned: future compiler workers may consume graph IR and generate GLSL, C++, validation reports, migration output, or documentation.
- Planned: file-based `graphIR.json` / `validation_report.json` exchange is only for batch fixtures, CI, and proof dumps. A live C# compiler worker needs an explicit interactive bridge such as named pipes, gRPC, ZeroMQ, or shared memory before it can support drag-time feedback.
- Proven: G0 first graph language contract has region types, TypeSpec including `audio.channels`, StreamKind, PortBinding modes, typed edges, Tooll3-inspired command names, AI-safe mutation rules, and C# external compiler worker boundary.
- Proven: graphIR and compiler-worker fixtures exist for batch proof exchange, with live editor IPC explicitly separated from file IO.
- Forbidden: direct AI JSON surgery.
- Forbidden: C# / .NET code in realtime audio callbacks, render hot paths, or native app lifecycle for the first stage.

Current graph language execution plan:

```text
docs/superpowers/plans/2026-05-22-g0-graph-language-contract.md
```

### A0 UI / Node Contract Guard

Goal:

```text
Tooll3-seeded NodeSpec taxonomy -> Dear ImGui patch interaction smoke -> no JUCE Component NodeView trap
```

Contract:

- Planned: first node taxonomy registry separates `type`, `category`, `subcategory`, `runtimeDomain`, and port `dataType`.
- Planned: seed categories borrow Tooll3's library domains: `image`, `render`, `mesh`, `point`, `numbers`, `io`, `field`, `flow`, `particle`, `string`, `data`, and `assets`.
- Planned: project-specific seed categories add `shader`, `material`, `audio`, `analyzer`, `signal`, `output`, and `compound`.
- Planned: familiar vocabularies such as `top`, `sop`, `mat`, `geometry`, `texture`, and `midi` are aliases or browser filters, not first-level saved category law. `signal` is now also a first-level project category because `signal.smoother` is a standalone node.
- Planned: `type` remains stable saved graph identity; `category` / `subcategory` are registry metadata and can be extended or migrated later.
- Planned: each node spec points to a human manual; machine-readable behavior lives in `NodeSpec`, not prose.
- Planned: Dear ImGui mounts in the OpenGL render loop before any production node editor work.
- Planned: A0 proves Tooll3-inspired patch gestures: zoom, pan, selection, framing, drag from pin to empty canvas, compatible node search, connect, parameter override, compound enter/exit, and undo/redo.
- Planned: production node surfaces follow Tooll3-like compact ImGui patching: compact name, pins, tiny status, doc/patch icons, optional inline preview affordance, and inspector/parameter panels for depth.
- Proven: first node taxonomy registry exists with Tooll3-seeded categories `image`, `render`, `mesh`, `point`, `numbers`, `io`, `field`, `flow`, `particle`, `string`, `data`, and `assets`, plus project categories `shader`, `material`, `audio`, `analyzer`, `signal`, `output`, and `compound`.
- Proven: TiXL user-facing taxonomy parity now has a dedicated gate in `docs/superpowers/specs/2026-05-24-tixl-taxonomy-parity-spec.md`; browser labels/paths mirror TiXL, while saved graph `type` remains native law.
- Proven: the seed registry now includes standalone C1 child nodes `audio.mono_mix`, `analyzer.analysis_gain`, `analyzer.pre_gate`, `signal.smoother`, and `analyzer.loudness_out`, plus the mother node `compound.loudness`.
- Proven: node `type` is stable identity; `category` and `subcategory` are browser metadata and can move through aliases without changing saved graph identity.
- Proven: node specs point to human Markdown manuals and preview policies while machine-readable behavior stays in `NodeSpec`.
- Proven: Tooll3-inspired patch gestures lower to explicit commandGraph command names through `PatchInteraction`.
- Proven: Dear ImGui is mounted in the OpenGL render loop before production node editor work.
- Proven: first UI adapter is immediate-mode; no JUCE `Component` NodeView exists.
- Proven: Tooll3-style T0-T7 interaction core exists below the drawing layer: canvas transform, node move, connect/disconnect, create-and-connect, compound enter/exit/collapse, param/binding commands, dirty/save state, and behavior trace replay.
- Proven: the app now shows a visible T0-T7 ImGui canvas proof over the shader preview, backed by the same `InteractionContract` command path rather than UI-only graph state.
- Proven: first visible T0-T2 canvas gestures exist: empty-canvas pan, wheel zoom, node drag to `move_node`, output-port drag to input-port `connect`, edge click selection, and selected-edge disconnect.
- Proven: selected node deletion now has a command-backed path: UI Delete button/key lowers to `delete_node`, removes incident edges, syncs runtimeGraph, and preserves undo/redo.
- Proven: first visible T3 gesture exists: dragging from an output port to empty canvas opens a compatible-node popup filtered by `NodeSpec` input type, and candidate selection runs `create_node+connect`.
- Proven: visible T4-T7 controls exist: compound add/enter/exit/collapse, selected-node inspector param/binding commands, interaction state save/reload, and in-app Tooll3 behavior trace replay.
- Proven: Codex Hands V0 exists as a low-token internal canvas operation layer in `source/core/CanvasHands.*`; semantic targets such as node, port, canvas point, and viewport center resolve through graph data, `canvasToScreen`, and `hitTestGraph` before pointer actions run.
- Proven: Codex Hands V0 can test semantic click selection, node drag, port-to-port connection drag, right click, middle click, and wheel evidence without screenshot guessing or global macOS mouse control.
- Proven: first Tooll3-like workspace layout promotes the node canvas to the OpenGL/ImGui main surface; the prior shader editor lives in the JUCE side panel, while `out1` preview, shader status, and compound controls live in workspace side panels.
- Proven: Tooll3 skin parity P0-P3 first pass exists: `Tooll3SkinContract` makes output-as-background, hidden global shader source, non-framed canvas, left rail, and bottom transport testable; the app proof frame now draws nodes floating over the shader output background.
- Proven: Tooll3 skin parity P4 first pass exists: node fills, labels, selection outlines, input/output side strips, and connection colors now come from `Tooll3SkinContract` typed visual grammar instead of ad hoc ImGui colors.
- Proven: Tooll3 skin parity P5/P6 first pass exists: selected-node inspector policy and row-state styling are testable, and `shader.fragment` source editing now lives in the ImGui inspector with `set_param` commandGraph evidence plus OpenGL compile handoff.
- Proven: Tooll3 skin parity P7 first pass exists: the left rail has Presets/Snapshots/Library tabs, right-click empty canvas opens a gesture-anchored commandGraph-backed node browser, and the bottom strip reads as transport/status/timeline boundary.
- Planned: future Codex Hands layers may inject into ImGui IO or OS-level mouse control, but only after the internal semantic trace layer stays commandGraph-backed and replayable.
- Forbidden: implementing a temporary JUCE `Component` node editor or `NodeView` that would later be replaced wholesale by ImGui.
- Forbidden: writing long explanatory sentences directly on node surfaces.
- Forbidden: copying Tooll3's exact appearance, icons, branding, C# ownership model, DirectX/HLSL backend, or `SymbolPackage` compilation system.

### V1 Visual Proof

Goal:

```text
native app -> hand-written Shader node -> Output preview
```

Contract:

- Proven: the app opens as a native desktop app.
- Proven: a default graph contains one Shader node connected to Output.
- Proven: the user can edit fragment shader text with the current JUCE `TextEditor` proof UI.
- Proven: compile success updates the preview.
- Proven: compile failure keeps the last valid frame and shows an error.
- Proven: the runtime can dump `frame.png`, `cook_order.json`, and `node_stats.json` for proof.

Current proof command:

```bash
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
```

Current proof output:

```text
debug/v1-shader-proof/frame.png
debug/v1-shader-proof/cook_order.json
debug/v1-shader-proof/node_stats.json
debug/v1-shader-proof/loudness_compound.json
debug/v1-shader-proof/runtime_registry.json
debug/v1-shader-proof/runtime_op_catalog.json
debug/v1-shader-proof/runtime_op_coverage.json
debug/v1-shader-proof/runtime_ui_diagnostics.json
debug/v1-shader-proof/runtime_dry_run.json
debug/v1-shader-proof/runtime_execution.json
debug/v1-shader-proof/runtime_missing_runtimeop_registry.json
debug/v1-shader-proof/runtime_missing_runtimeop_coverage.json
debug/v1-shader-proof/runtime_missing_runtimeop_dry_run.json
debug/v1-shader-proof/runtime_missing_runtimeop_execution.json
```

The V1 dump currently reads back the OpenGL framebuffer from `OpenGLShaderPreview`. This proves native render evidence, but the render backend boundary is not fully extracted yet.

### A1 Audio Proof

Goal:

```text
native preferences -> audio input -> native analyzer meter rows
```

Contract:

- Proven: preferences include audio input device, channel/mono mix, sample rate, buffer size, analysis gain, and analyzer profile.
- Proven: pure realtime-safe analyzer state can calculate rms, peak, loudness, active, and sampleCounter from input buffers.
- Proven: JUCE audio callback bridge exists and only writes bounded analyzer state plus output silence.
- Proven: UI reads analyzer values outside the realtime callback through the C1 loudness runtime bridge and displays `rms`, `peak`, and `loudness` meter rows.
- Proven: A1 can dump `debug/a1-audio-proof/audio_stats.json`, `debug/a1-audio-proof/loudness_compound.json`, `debug/a1-audio-proof/loudness_runtime_execution.json`, and `debug/a1-audio-proof/loudness_runtime_bridge.json` from `--dump-audio-proof-and-exit`.
- Proven: live input was observed on this Mac in the current A1 proof dump (`48000Hz`, `512` samples, `sampleCounter` greater than `0`, `active: true`).
- Proven: analyzer `loudness` snapshots are wired into shader uniform `u_loudness`; V1 proof `node_stats.json` records `u_loudness` as a system uniform.
- Proven: analyzer snapshot now exposes `rms`, `peak`, `loudness`, `gate`, `confidence`, `active`, and `sampleCounter`.
- Proven: the realtime audio path currently only measures or writes bounded realtime-safe state; it does not mutate graph structure.
- Proven: MIDI preferences are present early: output device, channel, CC map, stream on/off, and map mode, with default loudness mapping to CC20.
- Proven: the macOS app bundle includes `NSMicrophoneUsageDescription` before live audio proof asks for input.
- Known gate: macOS microphone permission cannot be silently self-granted. First live mic run requires the user to click Allow; unattended agent tests should use synthetic analyzer/unit proofs unless permission is already granted.
- Forbidden: mutating or rebuilding the audio runtime graph directly inside the audio callback. Live audio graph changes must be compiled/prepared off-thread and swapped through a realtime-safe snapshot or command queue.

### C1 Compound Proof

Goal:

```text
loudness compound patcher -> expanded child patchers -> collapsed public ports
```

Contract:

- Proven: `compound.loudness` exists as the first mother patch contract.
- Proven: it can be represented as one collapsed node with public ports.
- Proven: it expands to child patchers `AudioIn`, `MonoMix`, `RMS`, `AnalysisGain`, `PreGate`, `OutputSmoother`, and `LoudnessOut`.
- Proven: the compound exposes `out`, `rms`, `peak`, `gate`, and `confidence` as public outputs.
- Proven: child patchers are also standalone module-library node specs, so `audio.mono_mix`, `analyzer.rms`, `analyzer.analysis_gain`, `analyzer.pre_gate`, `signal.smoother`, and `analyzer.loudness_out` can be called directly outside the mother patch.
- Proven: `makeCompoundPatchJson()` creates proof evidence, and both V1/A1 proof dumps write `loudness_compound.json`.
- Proven: `fixtures/compounds/loudness.compound.json` is reloadable into `CompoundPatchSpec`, validates the same child/internal/public port contract, and shares the same JSON proof shape as generated `debug/v1-shader-proof/loudness_compound.json`.
- Proven: `fixtures/modules/loudness/module.json` loads as `ModulePackageManifest`, points to the loudness compound fixture, produces a `NodeSpec` from public ports, and can create `compound.loudness` through a module registry overload of `createNode`.
- Proven: `loadCompoundModuleNodeSpecs()` turns module manifests into a loaded registry, `mergeNodeSpecs()` lets loaded modules override seed specs by type, and the visible ImGui node browser now uses that passed registry when creating nodes.
- Proven: `loadCompoundModuleNodeSpecsFromLibrary()` loads the default module library index, resolves listed package paths near the library, and produces the visible registry consumed by app startup.
- Proven: `RuntimeRegistry` loads the same default module library into runtime entries and proof dumps now write `runtime_registry.json` with `compound.loudness`, `executionKind: compound.patch`, public ports, child counts, internal edge counts, and cook order.
- Proven: loaded runtime entries now include child metadata and `dryRunRuntimeRegistry()` writes `runtime_dry_run.json` with per-child cook index, node type, role, and `dry-run-ready` status.
- Proven: the first loaded executable child RuntimeOp exists: `executeRuntimeRegistryWithSyntheticAudio()` runs the `analyzer.rms` child through existing analyzer code over synthetic mono samples, records `rms` and `peak` outputs, and writes `runtime_execution.json` with computed vs not-executed child statuses.
- Proven: the first loaded value-handoff mini-chain exists: `RuntimeSyntheticAudioInput` supports multi-channel synthetic fixtures, `audio.mono_mix`, `analyzer.rms`, and `analyzer.analysis_gain` execute in cook order, and `runtime_execution.json` records both input and output value evidence per child.
- Proven: the first loaded public-output proof exists: `pre_gate`, `output_smoother`, and `loudness_out` execute in cook order, silent synthetic fixtures close gate/confidence, and `runtime_execution.json` records entry-level `publicOutputs`.
- Proven: loaded compound internal edges now participate in runtime execution: `runtime_registry.json` records `internalEdges` and `publicOutputMappings`, while `runtime_execution.json` records `inputSources` and `publicOutputSources` derived from those loaded routes.
- Proven: loaded compound children now execute through named synthetic RuntimeOps selected by node type, and `runtime_execution.json` records the `runtimeOp` id for each executed child.
- Proven: unsupported loaded child node types now fail dry-run and execution as `missing-runtime-op`, keep serializable failure snapshots, and do not publish public outputs on coverage failure.
- Proven: the missing RuntimeOp failure is now storage-backed by `fixtures/module-libraries/missing-runtimeop.module-library.json` and persisted by app proof dump as `runtime_missing_runtimeop_registry.json`, `runtime_missing_runtimeop_dry_run.json`, and `runtime_missing_runtimeop_execution.json`.
- Proven: RuntimeOp support is inspectable before execution: `runtime_op_catalog.json` lists the seven supported synthetic RuntimeOps, `runtime_op_coverage.json` reports the supported default loudness children, and `runtime_missing_runtimeop_coverage.json` names the saved `debug.unsupported` gap with supported/missing child counts.
- Proven: RuntimeOp support is visible before create/execute: `runtime_ui_diagnostics.json` declares browser/inspector visibility, `compound.loudness` reads as `runtime ready`, the saved negative fixture reads as `missing RuntimeOp`, and the workspace left rail shows the same status without inventing separate UI truth.
- Proven: normal module creation is coverage-gated: `runtime_ui_diagnostics.json` records `creationStatus` as `create-enabled` or `create-blocked`, `NodeCreationGate` blocks `compound.loudness.missing-runtimeop` without mutating the graph, and the ImGui browser/create popup passes diagnostics into the command path.
- Proven: blocked module insertion has an explicit repair/debug path: `create_node_debug_override` and `create_node+connect_debug_override` require visible reasons, store `debug.creationOverride*` node params, and remain undoable.
- Proven: loaded runtime public outputs can feed a live/debug-facing loudness bridge: `makeLoudnessRuntimeBridgeSnapshot()` prefers `out`, `rms`, `peak`, `gate`, and `confidence` from `RuntimeExecutionSnapshot`, falls back to direct analyzer values with the same ids, and serializes value/source evidence to `loudness_runtime_bridge.json`.
- Proven: app audio proof now feeds loaded runtime execution from a non-realtime snapshot-shaped input: `makeRuntimeSyntheticAudioInputFromAnalyzerSnapshot()` emits a mono synthetic window from `AudioAnalyzerSnapshot`, `loudness_runtime_execution.json` records the loaded seven-child runtime trace, and `loudness_runtime_bridge.json` records `sourceMode: loaded-runtime-publicOutputs`.
- Proven: the ImGui smoke overlay has a C1 debug view that shows collapsed/expanded loudness compound structure.
- Proven: the interaction command layer can create `compound.loudness`, enter/exit its patch path, collapse it, and roundtrip that editor state.
- Proven: a loaded `compound.loudness` fixture can be created as a graph node through `InteractionContract`, entered/exited, collapsed to its loaded default, and preserved through interaction state roundtrip.
- Proven: production canvas drag/drop for collapsed vs expanded compounds has first evidence: loaded compounds drag/select as collapsed root nodes, expanded child patcher graphs are generated from `CompoundPatchSpec`, child nodes/edges are parent-qualified, and root/expanded interaction states roundtrip.
- Proven: collapsed compound public ports are command-backed at root level: `connectPorts(session, specs, from, to)` validates against the loaded visible registry, `library_loud1.audio.in` and `library_loud1.out` have port centers, and root edges keep `audio.channels` / `signal.float` data types.
- Proven: public-port root edges survive interaction persistence pressure: the behavior trace fixture now runs `compound public ports persist undo`, checks editorGraph/runtimeGraph endpoints after deserialize, and verifies disconnect/undo/redo for `loud1.out -> midi1.value`.
- Proven: expanded child layout persists per compound instance: `storeExpandedPatchLayout()` writes moved child positions onto the parent compound node, root serialize/deserialize keeps them, and regenerated expanded graphs restore user-moved child positions.
- Proven: first C1 loudness compound proof is closed. Remaining compound work is C2+ work, not unfinished C1 evidence.
- Not yet proven: the runtime proof input is shaped from the analyzer snapshot, not raw callback buffers; live UI timer does not yet cache a prepared loaded runtime snapshot.
- Not yet proven: expanded view pan/zoom, raw callback-buffer runtime, and visual mapping-editor polish remain parked outside the closed first C1 spine.

## Architecture

### Data Layers

Keep four separate truths:

```text
editorGraph        node positions, selection, expanded/collapsed state, UI metadata
runtimeGraph       executable graph for render/audio/control runtime
commandGraph       formal mutation operations used by UI, AI, importers, and scripts
collaborationLog   AI tasks, commands, diagnostics, repair attempts, and proof evidence
```

Add storage truths before the editor grows:

```text
workManifest       project identity, main patch, library refs, save policy
patchDocuments     serializable graph documents that can be reloaded headlessly
modulePackages     reusable saved mother patches / compounds with public ports
libraryIndexes     known work repositories and module repositories
saveLog            save attempts, commit ids, failures, validation evidence
```

Storage is not an export layer. It is the graph source-of-truth boundary that lets UI, runtime, AI worker, and future module libraries read the same work back.

### Graph Language

The graph is closer to a visual programming language than to a canvas of UI widgets.

First-class graph objects:

```text
Node         ordinary operation or value source
Region       visual control-flow block such as if / for_each / repeat
Edge         typed connection with dataType and streamKind
PortBinding  default/manual/connected/animated parameter ownership
Command      validated graph mutation
Module       saved patch/compound with public ports
```

Graph flow:

```text
editorGraph
-> command validation
-> type validation
-> region boundary validation
-> graphIR / AST
-> runtimeGraph or compiler worker output
```

This allows the first runtime to remain direct C++/OpenGL while preserving a path to future compilation.

### Reference Systems

Tooll3 / TiXL is useful as a pressure reference, not as an inherited foundation.

Borrow:

```text
Symbol / Instance split
plain-text graph serialization
Tooll3-seeded library taxonomy
pin-drag node search and connection gestures
parameter value overridden by connection or animation
ImGui draw-list canvas techniques
undoable command discipline
timeline / parameter / preview / node graph panel rhythm
```

Do not borrow:

```text
C# runtime ownership
DirectX / HLSL first backend
SymbolPackage / C# compilation as module law
UI ids as saved graph ids
JSON edits that bypass commandGraph validation
exact Tooll3 appearance, icons, branding, or window layout as identity
```

Current borrowing note:

```text
docs/research/2026-05-22-tooll3-borrowing-notes.md
```

### Language / Runtime Boundary

C++ owns the first-stage app body:

```text
JUCE app shell
CoreAudio / audio callback
OpenGL / future Metal render loop
shader preview
realtime analyzer
work save and local commit
runtimeGraph hot path
```

External compiler workers are parked behind files/process boundaries:

```text
graphIR.json
commandGraph.json
validation_report.json
generated shader/source artifacts
```

C# may be useful later for graph compiler tooling, type inference experiments, module indexing, migration, documentation generation, or AI worker-side validation. It must not be embedded into realtime audio/render paths in the first stage.

### C++ Boundary

Avoid one giant class per node. Use this split:

```text
NodeSpec      static description: ports, params, widgets, compound children
NodeInstance  per-work state: values, position, exposed ports, editor flags
NodeView      generic drawing and interaction
RuntimeOp     execution logic for render/audio/control domains
Command       single mutation entrypoint
```

`NodeView` means a generic view adapter over `NodeSpec` / `NodeInstance`; it must not mean one JUCE `Component` subclass per node. The first graphics-side adapter should be Dear ImGui, proved by A0 before building production node editor interactions.

### Node Taxonomy

Use four separate labels:

```text
type           stable saved graph identity, e.g. analyzer.loudness
category       UI / browser grouping, e.g. analyzer
subcategory    UI / browser subgrouping, e.g. feature
runtimeDomain  cook owner, e.g. audioAnalysis
dataType       port compatibility, e.g. signal.float
```

First category registry is seeded from Tooll3 library domains plus project-specific domains:

```text
image
render
mesh
point
numbers
io
field
flow
particle
string
data
assets
shader
material
audio
analyzer
signal
output
compound
```

First subcategory examples:

```text
generate
modify
draw
color
analyze
transform
camera
postfx
shading
scene
input
output
midi
osc
audio
file
context
feature
detector
aggregate
calibration
gate
shaping
use
measurement
```

Aliases and browser filters:

```text
geometry -> mesh
midi -> io.midi
texture / top -> image
sop -> mesh / point
mat -> material
```

New categories can be added later through the registry. Existing saved graph identity should move through aliases or migration, not by casually renaming `type`. Category and subcategory never decide runtime execution; `runtimeDomain` does.

### Node Surface

The patch surface should lean directly into a Tooll3-like ImGui instrument feel: compact, dark, technical, fast to scan, and built around graph writing. Borrow the operating feel, spacing logic, and panel relationship; do not clone exact icons, branding, or pixel identity.

Default visible node:

```text
name
input pins
output pins
tiny status mark
doc icon
patch-behind / expand icon when available
```

Hidden until hover, selection, inspector, or a dedicated display node:

```text
long description
usage examples
failure modes
large values
meters
previews
full parameter explanations
```

Preview policy:

```text
no preview       utility/control nodes
tiny preview     texture/material/shader nodes when a cached output exists
meter/scope      audio/analyzer/control signals
selected preview larger inspector or preview panel
```

Node previews are required for visual debugging, but they must be cached outputs owned by runtime/debug state. The node surface can display them; it must not cook expensive render work just because ImGui is drawing a node.

Values should be exposed through explicit nodes:

```text
IOBox
Pad
Meter
Scope
Preview
```

Parameter controls must support Tooll3-style binding without inheriting Tooll3's runtime:

```text
default value     NodeSpec default
manual value      saved user value
connected value   data-flow override
animated value    timeline / event override
```

Connecting a cable to a parameter does not erase the manual value. It changes the live binding mode, and disconnecting can reveal the stored manual value again.

The UI must expose binding mode clearly:

```text
default     neutral control, inherited default value
manual      editable control, saved user value
connected   control is visibly overridden and shows source
animated    control is visibly timeline-owned and shows clip/key source
```

When a parameter is connected or animated, dragging the slider may edit the stored manual fallback only if the UI labels that action clearly. It must not pretend the live value changed when the graph is overriding it.

Human documentation and machine documentation are separate:

```text
human manual   docs/nodes/<node-type>.md
machine spec   NodeSpec registry / serialized graph schema
```

The node surface may show a `?` or small book icon that opens the human manual in an inspector. The machine spec is used by validation, node browser filtering, graph migration, AI worker commands, and runtime dispatch.

### Patch Interaction Grammar

Borrow Tooll3's operation feel, but lower every gesture into our commands.

```text
zoom / pan / selection / framing
drag from pin to empty canvas -> compatible node search
drag from pin to compatible pin -> connect
parameter slider -> manual PortBinding
connect parameter pin -> connected PortBinding
timeline assignment -> animated PortBinding
double-click compound/module -> enter patch body
collapse compound/module -> show public ports only
undo / redo -> commandGraph-backed history
```

Expected command lowering:

```text
create_node
connect
set_param
set_port_binding
set_view
select
enter_patch
exit_patch
publish_module
undo
redo
```

The first A0 smoke proof can fake the node library visually, but it must still record the intended command names. Production UI cannot mutate graph state directly through ImGui widget state.

Tooll3's panel rhythm can be borrowed as a layout reference:

```text
node graph      primary writing surface
preview         live visual/audio proof surface
parameters      selected node and PortBinding editor
timeline        animated bindings and time clips
library/search  category/subcategory node browser
```

The first implementation may show only a subset, but the layout should not force node graph, preview, parameters, and timeline into one overloaded panel.

### Render Backend

Initial implementation:

```text
RenderBackend interface
OpenGLShaderPreview direct implementation
```

Risk stance:

```text
OpenGL    proof backend only
Metal     Mac-first production backend direction
WGPU/bgfx parked portable backend candidates
MoltenVK  parked Vulkan portability path, not first backend
```

Minimum backend methods:

```text
compileShader
createRenderTarget
renderNode
readPixels
resize
destroy
```

Metal is the production direction, but it should not be implemented before the backend boundary bears weight. `RenderBackend` exists as an interface, but `OpenGLShaderPreview` still owns compile/render/readback directly. Extracting the OpenGL implementation behind `RenderBackend` remains pending and blocks Metal backend work, production node previews, high-resolution render claims, and compute-heavy visual graph work.

## Audio Analyzer Migration

Current execution plan:

```text
docs/superpowers/plans/2026-05-22-a1-audio-proof.md
```

The analyzer should borrow from:

- `/Users/chenbaiwei/Documents/GitHub/sound-in-area-analyzer-plugin`
- `/Users/chenbaiwei/Desktop/vibe coding/0512＿現場表演系統01/sound_in_area/src/analyzer`

Do not copy it as one black-box node. Split analyzer work into:

```text
Raw Facts
rms, peak, diffMean, zeroCrossHz

Feature
loudness, highMotion, zcrShape, registerEstimate, active

Detector / State
attackRaw, attackEnvelope, onsetEvent, densityEnvelope, sustainEnvelope, silenceTimer, residueEnvelope

Aggregate
globalPressure, breathiness, instability, sourceLegibility, rupture, residue

Output Shaping
gate, curve, smooth, outMin/outMax, MIDI CC, shader uniform mapping
```

First audio-visual closure:

```text
audio input -> loudness meter -> shader uniform u_loudness -> Output preview reacts
```

## AI Worker Position

AI worker is a first-class future client of the graph, not a chat box.

Initial command-loop shape:

```text
natural language task
-> command list
-> graph mutation
-> compile/render/analyze proof
-> repair loop
-> final state + evidence
```

The first skeleton does not need full AI UI, but it must not design graph mutation paths that AI cannot use later. Batch file exchange is acceptable for proof dumps; live editor feedback cannot depend on filesystem polling once compiler workers become interactive.

C4 proves the first non-chat AI worker command path:

```text
AIWorkerCommandRequest(move_node)
-> allowed operation check
-> InteractionContract::moveNode()
-> AIWorkerCommandRequest(save_work)
-> StorageCommand::saveWork()
-> commandLog + collaborationLog evidence
-> PatchDocument reload + save log readback
```

This is intentionally not a natural-language planner or repair loop yet. C4 only admits `move_node` and `save_work`; mutation remains owned by `InteractionContract`, and the actual file write remains owned by `StorageCommand`.

## Non-Goals For First Skeleton

- No full old Web project importer.
- No black velvet material demo as the first proof.
- No Metal backend implementation yet, but Metal is the production direction. No performance promise may depend on OpenGL until `RenderBackend` is actually extracted.
- No plugin build target yet.
- No complete 13-patch analyzer UI yet.
- No full AI worker UI yet.
- No production node editor UI yet; however Dear ImGui smoke proof is now required before A1 meter UI grows further.
- No temporary JUCE `Component` node editor.

## Open Questions

- Exact JUCE version / dependency acquisition method for portable builds. Current local proof references `/Users/chenbaiwei/Documents/GitHub/sound-in-area-analyzer-plugin/JUCE`.
- Whether to vendor JUCE, use CPM/FetchContent, or keep a local checkout path for early work.
- Exact first Metal backend vertical slice after the OpenGL proof backend is extracted behind `RenderBackend`.
- Exact first graph serialization format once `NodeSpec` is drafted.
- Exact Dear ImGui node editor library choice (`imnodes`, `imgui-node-editor`, or custom layer) after the command graph contract is less soft.

These are implementation questions, not design blockers.
