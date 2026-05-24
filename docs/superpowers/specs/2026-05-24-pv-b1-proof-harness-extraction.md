# PV-B1 Proof Harness Extraction

Date: 2026-05-24
Status: closed

## Load-Bearing Goal

Move PV-B1 analyzer environment proof orchestration out of `MainComponent` while preserving the existing CLI flag and artifact contract.

The app shell may keep:

- startup trigger
- status label text
- quit-after-dump scheduling

The runner must own:

- visible analyzer module-library candidate lookup
- output directory clearing/creation
- `analyzer_environment_report.json` writing
- success / mismatch / failure result shape

## Preserved External Contract

CLI:

```text
--dump-pv-b1-analyzer-environment-proof-and-exit
```

Artifact:

```text
debug/pv-b1-analyzer-environment-proof/analyzer_environment_report.json
```

Stable JSON fields:

```text
kind = pvB1AnalyzerEnvironmentProof
ok
operation = pv_b1_analyzer_environment_promotion
loadedModuleNodeCount
visibleCatalogContainsAllRequired
runtimeDiagnosticsReadyForAllRequired
createdNodeCount
error
```

## Implementation

- `source/app/PVB1AnalyzerEnvironmentProofRunner.h`
- `source/app/PVB1AnalyzerEnvironmentProofRunner.cpp`
- `tests/PVB1AnalyzerEnvironmentProofRunnerTests.cpp`

`MainComponent::dumpPVB1AnalyzerEnvironmentProof()` now builds a small request, calls `runPVB1AnalyzerEnvironmentProof()`, and maps the result to UI status text.

## Verification

Red:

```text
cmake -S . -B build
```

Expected failure before runner implementation:

```text
Cannot find source file:
  source/app/PVB1AnalyzerEnvironmentProofRunner.cpp
```

Green:

```text
cmake -S . -B build
cmake --build build --target my_world_pv_b1_analyzer_environment_proof_runner_tests
cmake --build build --target my-world
ctest --test-dir build --output-on-failure -R "pv_b1_analyzer_environment_proof_runner|analyzer_visible_catalog"
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-b1-analyzer-environment-proof-and-exit
```

Accepted result:

- `analyzer_visible_catalog` passed.
- `pv_b1_analyzer_environment_proof_runner` passed.
- app target built.
- CLI proof exited 0.
- `analyzer_environment_report.json` retained `ok: true`, `loadedModuleNodeCount: 8`, and `createdNodeCount: 8`.

## Parked

- C2-C6 proof orchestration remains in `MainComponent`.
- V1 shader proof remains tied to `OpenGLShaderPreview`.
