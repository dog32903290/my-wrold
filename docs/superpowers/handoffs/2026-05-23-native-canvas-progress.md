# Native Canvas Progress

Date: 2026-05-23 18:13 Asia/Taipei

## Current Head

```text
pending C1.11 RuntimeOp dispatch-table proof commit
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

## 試壓結果

```text
cmake --build build
./build/my_world_storage_tests
./build/my_world_compound_module_tests
./build/my_world_runtime_registry_tests
ctest --test-dir build --output-on-failure
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
git diff --check
```

Latest accepted result:

```text
19/19 tests passed
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
latest accepted source commit before C1.11: c47a59c
```

## 還沒承重

- Module discovery, runtime registry snapshots, child dry-run statuses, value handoff, first public output map, internal-edge source evidence, and named RuntimeOp dispatch are storage-backed, but missing RuntimeOp coverage is not yet validated as a first-class failure.
- `RenderBackend` has not been extracted; Metal remains the production direction but is still parked.
- Timeline editing is visual/status only; animation commandGraph does not exist yet.
- Output pinning, multi-output workflow, and live node thumbnails are not proven.
- AI worker graph edits are still parked until saved commandGraph/module evidence is stronger.

## 下一根線

C1.12 RuntimeOp coverage failure proof:

```text
module-library manifest
-> runtime registry entry
-> RuntimeOp coverage check
-> missing child nodeType fixture
-> explicit dry-run/execution failure reason
-> proof JSON records missing runtimeOp
-> run tests and proof dump
```

Reason:

```text
The Tooll3-like UI skin and first module package now bear weight.
The next weakness is runtime coverage law. The loaded routes and named RuntimeOps now execute, but a module can still contain an unsupported child node without a dedicated validation proof.
```
