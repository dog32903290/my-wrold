# Native Canvas Progress

Date: 2026-05-23 08:12 Asia/Taipei

## Current Head

```text
b594a73 Add command-backed node deletion
d76e353 Add Tooll3 workspace browser and transport
07b2db9 Move shader source into Tooll3 inspector
3de4790 Add Tooll3 typed node skin
3831b25 Apply Tooll3 skin parity shell
0428eb8 Promote node canvas workspace
```

## 已鎖定

- The node canvas is the main workspace, not a small proof window.
- `out1` fills the workspace background and nodes/connections float over the output.
- T0-T7 interaction is command-backed: canvas navigation, select/move, connect/disconnect, create-and-connect, compound enter/exit/collapse, inspector param/binding, dirty/save state, and behavior trace replay.
- Delete now follows selected-object behavior. Selected edge uses `disconnect`; selected node uses `delete_node` and clears incident edges.
- Tooll3 skin parity P0-P7 first pass exists: dark flat shell, typed node skin, inspector source editing, left rail tabs, right-click browser, and bottom transport/status strip.
- C1.1 exists: `fixtures/compounds/loudness.compound.json` loads into `CompoundPatchSpec`, validates, and can be created/entered/collapsed through `InteractionContract`.
- C1.2 exists: `fixtures/modules/loudness/module.json` loads into `ModulePackageManifest`, produces a compound `NodeSpec`, and creates `compound.loudness` from a module registry command path.

## 試壓結果

```text
cmake --build build
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
```

## 還沒承重

- `compound.loudness` is reloadable from a fixture and module package, but visible node browser/module-library integration is not wired yet.
- `RenderBackend` has not been extracted; Metal remains the production direction but is still parked.
- Timeline editing is visual/status only; animation commandGraph does not exist yet.
- Output pinning, multi-output workflow, and live node thumbnails are not proven.
- AI worker graph edits are still parked until saved commandGraph/module evidence is stronger.

## 下一根線

C1.3 visible module registry proof:

```text
fixtures/modules/loudness/module.json
-> loaded module registry
-> visible node browser / command path
-> create compound in workspace from registry source
-> save/load evidence
-> run tests and proof dump
```

Reason:

```text
The Tooll3-like UI skin and first module package now bear weight.
The next weakness is whether the visible workspace can consume module registries instead of only the compiled seed node list.
```
