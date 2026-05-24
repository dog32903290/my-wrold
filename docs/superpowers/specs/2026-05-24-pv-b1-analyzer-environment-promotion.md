# PV-B1 Analyzer Environment Promotion

Date: 2026-05-24 22:01 Asia/Taipei

Status: closed by `docs/superpowers/specs/2026-05-24-pv-b1-analyzer-environment-promotion-closure.md`.

Closure timestamp: 2026-05-24 22:12 Asia/Taipei.

## Progress Gate Result

PV analyzer work is closed through attack, density, silence, sustain, residue, and aggregate pressure. The next selected lane is environment promotion, not another analyzer detector, not MIDI mapping, not shader uniform mapping, not browser polish, and not live callback-buffer runtime.

Do not touch Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, TiXL, relocation note, flow-runner files, `scripts/`, `tests/test_myworld_flow.py`, or `AGENTS.md` in this slice.

## First Line

```text
fixtures/module-libraries/pv-analyzer-visible.module-library.json
-> visible node browser/create registry
-> debug/pv-b1-analyzer-environment-proof/analyzer_environment_report.json
```

This proof promotes already-closed analyzer module packages into the tool environment. It does not prove new DSP behavior and does not require live audio input.

## Environment Contract

```text
Node admission surface: PV-B1 analyzer environment
Question: can the already-proven analyzer modules be found, inspected, and created through the same visible command path as other native nodes?
Conversion: closed module library specs -> visible NodeSpec catalog + runtime-op diagnostics + create_node command proof
Family: analyzer / environment promotion

Required visible node types:
- compound.loudness
- compound.raw-energy
- compound.attack
- compound.density
- compound.silence
- compound.sustain
- compound.residue
- compound.aggregate-pressure

Inputs:
- module library fixture: `fixtures/module-libraries/pv-analyzer-visible.module-library.json`
- existing module package manifests under `fixtures/modules/**/module.json`
- existing compound specs under `fixtures/compounds/*.compound.json`

Outputs:
- visible node catalog includes every required node type
- runtime-op diagnostics mark every required node type as create-enabled
- createNode command path creates every required node type in a GraphSession
- app proof JSON records visible catalog and create evidence

Parameters:
- none. This lane is catalog admission, not analyzer calibration.

State:
- no runtime analyzer state is introduced.
- no UI-only browser filter state is part of the proof.

Failure:
- missing module library or manifest: proof fails with load error
- missing visible NodeSpec: proof fails and names the missing node type
- runtime-op coverage blocked: proof fails and names the blocked node type
- createNode failure: proof fails and records command error

Diagnostics:
- library path
- required node types
- visible node count
- per-node input/output/param counts
- runtime coverage status
- create command log status
- flags proving no MIDI/shader/live-callback work entered this lane

Evidence:
- focused test target `my_world_analyzer_visible_catalog_tests`
- app proof dump `debug/pv-b1-analyzer-environment-proof/analyzer_environment_report.json`

Split / compound decision:
- This is an environment promotion proof, not a detector or aggregate.
- It consumes existing closed module packages and exposes them through the visible catalog.
- Browser visual polish, MIDI CC output shaping, shader uniform mapping, and live callback-buffer runtime remain separate lanes.
```

## Target Node Set

```text
compound.loudness:
  already seed/default module; kept visible so the new analyzer surface does not lose the original feature compound.

compound.raw-energy:
  raw facts entry point for RMS / peak / sampleCount.

compound.attack:
  closed attack detector module.

compound.density:
  closed density detector module.

compound.silence:
  closed silence detector module.

compound.sustain:
  closed sustain detector module.

compound.residue:
  closed residue detector module.

compound.aggregate-pressure:
  closed aggregate pressure module.
```

## Runtime / Debug Proof

Target proof artifacts:

```text
debug/pv-b1-analyzer-environment-proof/analyzer_environment_report.json
```

Report minimum fields:

```text
ok
operation: pv_b1_analyzer_environment_promotion
libraryPath
requiredNodeTypes
visibleNodeCount
visibleCatalogContainsAllRequired
runtimeDiagnosticsReadyForAllRequired
createdNodeCount
nodes
usesExistingAnalyzerRuntimeProofs: true
addsNewAnalyzerDSP: false
usesMidiMapping: false
usesShaderUniformMapping: false
usesLiveCallbackRuntime: false
error
```

Target verification:

```text
cmake --build build --target my_world_analyzer_visible_catalog_tests my-world
./build/my_world_analyzer_visible_catalog_tests
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-b1-analyzer-environment-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```
