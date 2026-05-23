# Native Canvas Progress

Date: 2026-05-24 01:58 Asia/Taipei

## Current Head

```text
C1.19 loaded loudness runtime bridge ready for commit
a81eba2 Add compound expanded canvas interaction proof
29dc4b2 Add debug override module creation proof
29a9fda Gate module creation by RuntimeOp diagnostics
bdab7fc Add visible RuntimeOp coverage diagnostics
701aeb9 Add RuntimeOp catalog coverage proof
fbc8a0c Add saved negative RuntimeOp fixture proof
4cf4b77 Fail runtime execution on missing RuntimeOps
0df9775 Dispatch loaded runtime nodes through named ops
c47a59c Route loaded runtime values through internal edges
918545b Publish loaded loudness runtime outputs
b3a3a72 Add loaded loudness mini-chain execution proof
81c10cb Add synthetic RMS runtime execution proof
b5705f4 Add loaded compound dry run proof
a1eef36 Add loaded compound runtime registry proof
a9faa34 Add module library registry proof
ec734b6 Wire visible module registry
7b3ca79 Add loudness module package proof
0d782b5 Add reloadable loudness compound fixture
6838957 Document current native canvas progress
b594a73 Add command-backed node deletion
d76e353 Add Tooll3 workspace browser and transport
07b2db9 Move shader source into Tooll3 inspector
```

## 已鎖定

- The node canvas is the main workspace, not a small proof window.
- `out1` fills the workspace background and nodes/connections float over the output.
- T0-T7 interaction is command-backed: canvas navigation, select/move, connect/disconnect, create-and-connect, compound enter/exit/collapse, inspector param/binding, dirty/save state, and behavior trace replay.
- Delete now follows selected-object behavior. Selected edge uses `disconnect`; selected node uses `delete_node` and clears incident edges.
- Tooll3 skin parity P0-P7 first pass exists: dark flat shell, typed node skin, inspector source editing, left rail tabs, right-click browser, and bottom transport/status strip.
- C1.1 exists: `fixtures/compounds/loudness.compound.json` loads into `CompoundPatchSpec`, validates, and can be created/entered/collapsed through `InteractionContract`.
- C1.2 exists: `fixtures/modules/loudness/module.json` loads into `ModulePackageManifest`, produces a compound `NodeSpec`, and creates `compound.loudness` from a module registry command path.
- C1.3 exists: module manifests load into a visible `NodeSpec` registry, override seed specs by type, feed the ImGui node browser, and create `compound.loudness` through the same command path.
- C1.4 exists: `fixtures/module-libraries/default.module-library.json` lists module packages, storage parses that `ModuleLibrary` index, and visible app startup consumes the library index rather than a hardcoded module manifest path.
- C1.5 exists: `RuntimeRegistry` loads the same module library into runtime entries and app proof dump writes `debug/v1-shader-proof/runtime_registry.json`.
- C1.6 exists: runtime registry entries include child metadata, `dryRunRuntimeRegistry()` walks loaded compound cook order, and app proof dump writes `debug/v1-shader-proof/runtime_dry_run.json`.
- C1.7 exists: `executeRuntimeRegistryWithSyntheticAudio()` executes the loaded `analyzer.rms` child over synthetic mono samples, records `rms=0.707107` and `peak=1.000000`, keeps unimplemented siblings explicit, and app proof dump writes `debug/v1-shader-proof/runtime_execution.json`.
- C1.8 exists: `RuntimeSyntheticAudioInput` supports multi-channel synthetic fixtures, runtime execution records per-child `inputs` and `outputs`, and `audio.mono_mix -> analyzer.rms -> analyzer.analysis_gain` now passes values in cook order.
- C1.9 exists: `pre_gate`, `output_smoother`, and `loudness_out` execute after `analysis_gain`; app proof dump writes entry-level `publicOutputs` for `out`, `rms`, `peak`, `gate`, and `confidence`.
- C1.10 exists: runtime registry entries retain loaded `internalEdges` and `publicOutputMappings`; execution uses a `child.port` value bus and writes per-child `inputSources` plus entry `publicOutputSources`.
- C1.11 exists: synthetic loaded-compound execution dispatches child node types through named RuntimeOp functions and writes each executed child's `runtimeOp` id into `runtime_execution.json`.
- C1.12 exists: unsupported loaded child node types now fail dry-run and execution as `missing-runtime-op`, preserve a serializable failure snapshot, and avoid publishing public outputs on coverage failure.
- C1.13 exists: `fixtures/module-libraries/missing-runtimeop.module-library.json` loads a saved `compound.loudness.missing-runtimeop` module with `debug.unsupported`; runtime tests and app proof dump persist `runtime_missing_runtimeop_registry.json`, `runtime_missing_runtimeop_dry_run.json`, and `runtime_missing_runtimeop_execution.json`.
- C1.14 exists: the synthetic RuntimeOp table is exposed as `runtime_op_catalog.json`, registry coverage is reported before execution as `runtime_op_coverage.json`, and the saved negative fixture writes `runtime_missing_runtimeop_coverage.json` with supported vs missing child counts.
- C1.15 exists: RuntimeOp coverage now has a UI-facing diagnostic contract, app proof dump writes `runtime_ui_diagnostics.json`, and the ImGui workspace shows `runtime ready` / `missing RuntimeOp` status in the browser/inspector/left rail without inventing UI-only truth.
- C1.16 exists: RuntimeOp diagnostics now carry `create-enabled` / `create-blocked` affordance state, `InteractionContract` has a command-level `NodeCreationGate`, and the ImGui browser/create popup disables missing-runtime modules instead of letting UI-only confidence create them.
- C1.17 exists: blocked modules can now be inserted only through explicit debug override commands with visible reason, stored debug params, distinct command log entries, and undo coverage.
- C1.18 exists: loaded compound nodes can be dragged/selected in collapsed root view, entered into a parent-qualified expanded child patcher graph, and roundtripped with child positions/internal edges.
- C1.19 exists: `makeLoudnessRuntimeBridgeSnapshot()` prefers loaded runtime public outputs when available, falls back to direct analyzer snapshots with the same `out`, `rms`, `peak`, `gate`, and `confidence` vocabulary, and app audio proof dump writes `debug/a1-audio-proof/loudness_runtime_bridge.json`.

## 試壓結果

```text
cmake --build build
./build/my_world_storage_tests
./build/my_world_compound_module_tests
./build/my_world_runtime_registry_tests
./build/my_world_t3_t5_command_tests
./build/my_world_compound_interaction_tests
ctest --test-dir build --output-on-failure
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-audio-proof-and-exit
git diff --check
```

Latest accepted result:

```text
20/20 tests passed
debug/v1-shader-proof/frame.png regenerated
debug/v1-shader-proof/loudness_compound.json includes publicInputs and matches the reloadable fixture shape
fixtures/modules/loudness/module.json validated through storage and compound module tests
visible registry creates module-backed `compound.loudness` through `create_node`
fixtures/module-libraries/default.module-library.json feeds visible module registry
debug/v1-shader-proof/runtime_registry.json records compound.loudness as executionKind compound.patch
debug/v1-shader-proof/runtime_dry_run.json records seven dry-run-ready child statuses
debug/v1-shader-proof/runtime_execution.json records analyzer.rms as computed with rms/peak outputs
debug/v1-shader-proof/runtime_execution.json records audio.mono_mix, analyzer.rms, and analyzer.analysis_gain as computed with input/output handoff evidence
debug/v1-shader-proof/runtime_execution.json records publicOutputs { out, rms, peak, gate, confidence } and all seven children as computed
debug/v1-shader-proof/runtime_registry.json records loaded internalEdges and publicOutputMappings
debug/v1-shader-proof/runtime_execution.json records inputSources and publicOutputSources from loaded routes
debug/v1-shader-proof/runtime_execution.json records named runtimeOp ids for executed children
runtime registry tests record unsupported child nodeType failure as missing-runtime-op for dry-run and execution
compile-first-semantic-porting C1.12 measurement records 0 compile repairs, 0 test repairs, and 1 prevented coverage-honesty bug
runtime registry tests load the saved negative module fixture and verify dry-run/execution fail as missing-runtime-op without public outputs
debug/v1-shader-proof/runtime_missing_runtimeop_registry.json records compound.loudness.missing-runtimeop and debug.unsupported
debug/v1-shader-proof/runtime_missing_runtimeop_dry_run.json records missing-runtime-op before execution
debug/v1-shader-proof/runtime_missing_runtimeop_execution.json records missing-runtime-op and no publicOutputs
compile-first-semantic-porting C1.13 measurement records 0 compile repairs, 0 test repairs, and one storage-backed proof gap closed
runtime registry tests expose seven RuntimeOps through makeRuntimeOpCatalog()
runtime registry tests inspect positive and saved negative registries before execution
debug/v1-shader-proof/runtime_op_catalog.json records the supported synthetic RuntimeOp table
debug/v1-shader-proof/runtime_op_coverage.json records 7 supported children and 0 missing children
debug/v1-shader-proof/runtime_missing_runtimeop_coverage.json records 7 supported children and 1 missing debug.unsupported child
compile-first-semantic-porting C1.14 measurement records 0 compile repairs, 0 test repairs, and one author-feedback risk moved before execution
runtime registry tests produce UI-facing RuntimeOp module diagnostics for positive and saved negative registries
debug/v1-shader-proof/runtime_ui_diagnostics.json records visibleIn ["browser", "inspector", "leftRail"], runtime ready for compound.loudness, and missing RuntimeOp: debug.unsupported for the saved negative fixture
debug/v1-shader-proof/frame.png shows Runtime Coverage as readable workspace diagnostics in the left rail
compile-first-semantic-porting C1.15 measurement records 0 compile repairs, 0 test repairs, and one skin-contract drift avoided by keeping diagnostics downstream of coverage snapshots
runtime registry tests record create-enabled/create-blocked diagnostics for positive and saved negative registries
t3-t5 command tests prove NodeCreationGate blocks compound.loudness.missing-runtimeop without mutating the graph and allows runtime-ready compound.loudness
debug/v1-shader-proof/runtime_ui_diagnostics.json records creationStatus create-enabled for compound.loudness and create-blocked for compound.loudness.missing-runtimeop
compile-first-semantic-porting C1.16 measurement records 0 compile repairs, 0 test repairs, and one UI-only gate risk moved into command-level NodeCreationGate
t3-t5 command tests prove debug override creation requires a reason, writes debug.creationOverride params, logs create_node_debug_override, undo removes the inserted node, and create_node+connect_debug_override wires a repair node through a compatible audio source
ImGui browser/create popup shows an explicit Override affordance for blocked diagnostics and routes it through the debug override command path
compile-first-semantic-porting C1.17 measurement records 0 compile repairs, 1 test repair for compatible source typing, and one hidden-bypass risk moved into explicit override commands
compound interaction tests prove collapsed loaded-compound drag logs move_node, preserves selection/collapsed state, enters patch path, and roundtrips root state
compound interaction tests prove makeCompoundPatchInteractionGraph emits parent-qualified child nodes/edges, validates against seed NodeSpecs, supports CanvasHands child drag, and roundtrips expanded child graph state
interaction trace fixture now replays compound collapsed drag expanded roundtrip as create_node, collapse_compound, move_node, enter_patch, save_work:saved-and-committed
ImGui canvas switches to the expanded child patcher graph while inside a compound patch instead of showing only the root graph/bullet list
runtime registry tests prove the loudness bridge prefers loaded publicOutputs, preserves fallback sampleCounter, and emits serializable source/value JSON
debug/a1-audio-proof/loudness_runtime_bridge.json records direct-analyzer-fallback with publicOutputs { out, rms, peak, gate, confidence } and field-level sources
latest accepted source commit before C1.19: a81eba2
```

## 還沒承重

- Module discovery, runtime registry snapshots, child dry-run statuses, value handoff, first public output map, internal-edge source evidence, named RuntimeOp dispatch, missing RuntimeOp coverage failure, saved negative proof export, visible RuntimeOp diagnostics, coverage-gated creation, debug override insertion, expanded/collapsed compound interaction, and the loaded/fallback loudness bridge are storage-backed/test-backed.
- Missing-runtime modules are blocked by command-level creation gates; intentional repair insertion has explicit debug override commands.
- `RenderBackend` has not been extracted; Metal remains the production direction but is still parked.
- Timeline editing is visual/status only; animation commandGraph does not exist yet.
- Output pinning, multi-output workflow, and live node thumbnails are not proven.
- AI worker graph edits are still parked until saved commandGraph/module evidence is stronger.
- The live A1 analyzer surface now reads through the C1 bridge vocabulary, but app runtime execution still receives an empty live runtime snapshot; loaded runtime publicOutputs are proven through synthetic runtime execution, not a live sample-window runner.

## 下一根線

C1.20 live-safe loudness sample-window runner:

```text
AudioAnalyzerState/direct snapshot or prepared sample window
-> non-realtime RuntimeSyntheticAudioInput
-> loaded compound runtime execution snapshot
-> bridge sourceMode loaded-runtime-publicOutputs during app proof
-> run tests and proof dump
```

Reason:

```text
The bridge now gives UI/debug one vocabulary for loaded runtime outputs and direct fallback.
The next weakness is that the app proof still passes an empty runtime snapshot, so loaded runtime execution is not yet fed by a live-safe sample window.
```
