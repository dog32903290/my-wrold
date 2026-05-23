# Native Canvas Progress

Date: 2026-05-23 09:05 Asia/Taipei

## Current Head

```text
pending C1.3 visible module registry proof commit
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

## 試壓結果

```text
cmake --build build
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
latest accepted source commit before C1.3: 7b3ca79
```

## 還沒承重

- Visible node browser/module-registry integration is wired, but module discovery still uses app-side candidate manifest paths instead of a saved `ModuleLibrary` index.
- `RenderBackend` has not been extracted; Metal remains the production direction but is still parked.
- Timeline editing is visual/status only; animation commandGraph does not exist yet.
- Output pinning, multi-output workflow, and live node thumbnails are not proven.
- AI worker graph edits are still parked until saved commandGraph/module evidence is stronger.

## 下一根線

C1.4 module-library index proof:

```text
module-library manifest
-> list module packages
-> load visible module registry
-> visible node browser / command path
-> create compound in workspace without hardcoded app paths
-> run tests and proof dump
```

Reason:

```text
The Tooll3-like UI skin and first module package now bear weight.
The next weakness is module discovery. The visible workspace can consume module registries, but it still needs a storage-backed library index before AI worker edits or user module libraries can safely sit on this line.
```
