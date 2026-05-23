# Native Canvas Progress

Date: 2026-05-23 23:41 Asia/Taipei

## Current Head

```text
pending C1.13 saved negative RuntimeOp fixture proof commit
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
runtime registry tests record unsupported child nodeType failure as missing-runtime-op for dry-run and execution
compile-first-semantic-porting C1.12 measurement records 0 compile repairs, 0 test repairs, and 1 prevented coverage-honesty bug
runtime registry tests load the saved negative module fixture and verify dry-run/execution fail as missing-runtime-op without public outputs
debug/v1-shader-proof/runtime_missing_runtimeop_registry.json records compound.loudness.missing-runtimeop and debug.unsupported
debug/v1-shader-proof/runtime_missing_runtimeop_dry_run.json records missing-runtime-op before execution
debug/v1-shader-proof/runtime_missing_runtimeop_execution.json records missing-runtime-op and no publicOutputs
compile-first-semantic-porting C1.13 measurement records 0 compile repairs, 0 test repairs, and one storage-backed proof gap closed
latest accepted source commit before C1.13: 4cf4b77
```

## 還沒承重

- Module discovery, runtime registry snapshots, child dry-run statuses, value handoff, first public output map, internal-edge source evidence, named RuntimeOp dispatch, missing RuntimeOp coverage failure, and saved negative proof export are storage-backed/test-backed.
- RuntimeOp support is not yet exposed as an inspectable catalog/coverage report for future module authors.
- `RenderBackend` has not been extracted; Metal remains the production direction but is still parked.
- Timeline editing is visual/status only; animation commandGraph does not exist yet.
- Output pinning, multi-output workflow, and live node thumbnails are not proven.
- AI worker graph edits are still parked until saved commandGraph/module evidence is stronger.

## 下一根線

C1.14 RuntimeOp catalog coverage proof:

```text
RuntimeOp table
-> support matrix JSON
-> module-library coverage report
-> proof dump names supported and missing child nodeTypes before execution
-> run tests and proof dump
```

Reason:

```text
Saved positive and negative module fixtures now bear weight.
The next weakness is author feedback: future modules need to know RuntimeOp coverage before they hit execution failure.
```
