# Native Canvas Progress

Date: 2026-05-23 09:12 Asia/Taipei

## Current Head

```text
pending C1.4 module-library index proof commit
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

## 試壓結果

```text
cmake --build build
./build/my_world_storage_tests
./build/my_world_compound_module_tests
ctest --test-dir build --output-on-failure
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
git diff --check
```

Latest accepted result:

```text
18/18 tests passed
debug/v1-shader-proof/frame.png regenerated
debug/v1-shader-proof/loudness_compound.json includes publicInputs and matches the reloadable fixture shape
fixtures/modules/loudness/module.json validated through storage and compound module tests
visible registry creates module-backed `compound.loudness` through `create_node`
fixtures/module-libraries/default.module-library.json feeds visible module registry
latest accepted source commit before C1.4: ec734b6
```

## 還沒承重

- Module discovery is storage-backed through the default `ModuleLibrary` index, but loaded compounds are still editor/browser `NodeSpec`s rather than independently executable `RuntimeOp`s.
- `RenderBackend` has not been extracted; Metal remains the production direction but is still parked.
- Timeline editing is visual/status only; animation commandGraph does not exist yet.
- Output pinning, multi-output workflow, and live node thumbnails are not proven.
- AI worker graph edits are still parked until saved commandGraph/module evidence is stronger.

## 下一根線

C1.5 loaded compound runtime-registry proof:

```text
module-library manifest
-> loaded compound NodeSpec
-> runtime registry snapshot
-> debug proof that loaded compound execution is explicit instead of editor-only
-> run tests and proof dump
```

Reason:

```text
The Tooll3-like UI skin and first module package now bear weight.
The next weakness is runtime ownership. The visible workspace can discover saved modules through a library index, but the runtime still needs an explicit loaded-compound execution registry before AI worker edits or real module libraries can safely sit on this line.
```
