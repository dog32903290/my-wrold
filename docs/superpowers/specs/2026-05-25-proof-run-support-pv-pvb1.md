# ProofRunSupport PV/PV-B1 Cleanup

Date: 2026-05-25
Status: closed

## Load-Bearing Goal

Move PV detector and PV-B1 analyzer environment proof file/path primitives onto `ProofRunSupport` while preserving all detector semantics, visible-catalog checks, CLI flags, artifact names, and report schemas.

This is a support cleanup only. It must not change:

- attack / density / silence / sustain / residue / aggregate pressure detector proof behavior
- PV-B1 analyzer visible catalog behavior
- runtime registry / fixture loading semantics
- artifact filenames
- stable JSON fields
- MainComponent trigger/status facade

## Scope

`ProofRunSupport` now serves these PV/PV-B1 primitives:

- proof text file writing
- proof output directory clearing
- proof output directory creation
- candidate fixture/module-library path expansion and deduplication

PV and PV-B1 runners still own their proof-family orchestration, report selection, and semantic checks.

## Preserved External Contract

PV detector CLI flags:

```text
--dump-pv-attack-detector-proof-and-exit
--dump-pv-density-detector-proof-and-exit
--dump-pv-silence-detector-proof-and-exit
--dump-pv-sustain-detector-proof-and-exit
--dump-pv-residue-detector-proof-and-exit
--dump-pv-aggregate-pressure-proof-and-exit
```

PV detector artifacts stay under:

```text
debug/pv-*-proof/*_report.json
debug/pv-*-proof/cook_order.json
debug/pv-*-proof/node_stats.json
debug/pv-*-proof/errors.json
```

PV-B1 CLI and artifact:

```text
--dump-pv-b1-analyzer-environment-proof-and-exit
debug/pv-b1-analyzer-environment-proof/analyzer_environment_report.json
```

Stable fields retained:

```text
PV detector reports: kind, ok, operation, error
PV-B1 report: kind, ok, operation, loadedModuleNodeCount, visibleCatalogContainsAllRequired, runtimeDiagnosticsReadyForAllRequired, createdNodeCount, error
```

## Implementation

- `source/app/PVDetectorProofRunner.cpp`
- `source/app/PVB1AnalyzerEnvironmentProofRunner.cpp`
- `CMakeLists.txt`

Both runner targets now link `my_world_proof_run_support`.

## Verification

```text
cmake -S . -B build
cmake --build build --target my_world_pv_detector_proof_runner_tests my_world_pv_b1_analyzer_environment_proof_runner_tests my_world_proof_run_support_tests
ctest --test-dir build --output-on-failure -R "proof_run_support|pv_detector_proof_runner|pv_b1_analyzer_environment_proof_runner|analyzer_detector|analyzer_density_detector|analyzer_silence_detector|analyzer_sustain_detector|analyzer_residue_detector|analyzer_aggregate_pressure|analyzer_visible_catalog"
cmake --build build --target my-world
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-attack-detector-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-density-detector-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-silence-detector-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-sustain-detector-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-residue-detector-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-aggregate-pressure-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-b1-analyzer-environment-proof-and-exit
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted targeted result:

- `proof_run_support` passed.
- `pv_detector_proof_runner` passed.
- `pv_b1_analyzer_environment_proof_runner` passed.
- detector and visible catalog focused tests passed.
- app target built.
- All six PV detector CLI proofs exited 0 and kept `ok: true`, expected `kind`, expected `operation`, and empty `error`.
- PV-B1 CLI proof exited 0 and kept `ok: true`, `loadedModuleNodeCount: 8`, `visibleCatalogContainsAllRequired: true`, `runtimeDiagnosticsReadyForAllRequired: true`, and `createdNodeCount: 8`.
- Full `ctest` passed 51/51.
- `git diff --check` passed.

## Parked

- Do not migrate every remaining proof runner in one sweep.
- C5/C6 helper cleanup stays separate selected slices.
- This remains support extraction, not a new generic proof runner.
