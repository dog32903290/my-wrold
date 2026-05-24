# PV-B1 Analyzer Environment Promotion Closure

Date: 2026-05-24 22:12 Asia/Taipei

Status: closed for the analyzer environment promotion proof.

## Closure Line

```text
fixtures/module-libraries/pv-analyzer-visible.module-library.json
-> visible node browser/create registry
-> runtime-op diagnostics + createNode command path
-> debug/pv-b1-analyzer-environment-proof/analyzer_environment_report.json
```

This slice does not add analyzer DSP, MIDI mapping, shader uniform mapping, browser polish, live callback-buffer runtime, Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, TiXL, flow-runner behavior, relocation-note edits, or `AGENTS.md` edits.

## Closed Environment Promotion

Visible analyzer catalog:

```text
compound.loudness
compound.raw-energy
compound.attack
compound.density
compound.silence
compound.sustain
compound.residue
compound.aggregate-pressure
```

Proof says:

```text
ok: true
loadedModuleNodeCount: 8
visibleNodeCount: 22
visibleCatalogContainsAllRequired: true
runtimeDiagnosticsReadyForAllRequired: true
createdNodeCount: 8
addsNewAnalyzerDSP: false
usesMidiMapping: false
usesShaderUniformMapping: false
usesLiveCallbackRuntime: false
```

Every required analyzer compound reports `runtime-op-ready` and `createCommandLogStatus: create_node`.

## Runtime / Fixture Evidence

New fixture:

```text
fixtures/module-libraries/pv-analyzer-visible.module-library.json
```

New helper proof:

```text
source/core/AnalyzerVisibleCatalog.h
source/core/AnalyzerVisibleCatalog.cpp
tests/AnalyzerVisibleCatalogTests.cpp
```

New app proof command:

```text
--dump-pv-b1-analyzer-environment-proof-and-exit
```

The proof writes:

```text
debug/pv-b1-analyzer-environment-proof/analyzer_environment_report.json
```

The visible OpenGL overlay loader now merges:

```text
fixtures/module-libraries/default.module-library.json
fixtures/module-libraries/pv-analyzer-visible.module-library.json
```

V1 proof `debug/v1-shader-proof/runtime_ui_diagnostics.json` now records the PV analyzer compounds as `runtime ready`, while V1 render output still reports `renderer: OpenGL`.

## Verification Run

```text
cmake --build build --target my_world_analyzer_visible_catalog_tests my-world
./build/my_world_analyzer_visible_catalog_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-b1-analyzer-environment-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```

Latest accepted result:

```text
analyzer visible catalog ok
debug/pv-b1-analyzer-environment-proof/analyzer_environment_report.json has ok: true
debug/v1-shader-proof/runtime_ui_diagnostics.json includes PV analyzer compounds as runtime ready
40/40 tests passed
git diff --check passed
```

## Still Parked

```text
MIDI mapping
shader uniform mapping
browser polish beyond admission/create proof
live callback-buffer runtime changes
multi-node group-to-compound extraction
additional aggregate families such as breathiness, instability, source_legibility, rupture, or memory_distance
```
