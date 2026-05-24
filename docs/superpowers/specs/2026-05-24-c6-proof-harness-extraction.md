# C6 Proof Harness Extraction

Date: 2026-05-24
Status: closed

## Load-Bearing Goal

Move C6 analyzer family proof orchestration out of `MainComponent` while preserving the existing CLI flag, report file, and raw-energy evidence.

The app shell may keep:

- startup trigger
- status label text
- quit-after-dump scheduling

The runner must own:

- analyzer family module-library candidate lookup
- output directory clearing/creation
- visible catalog / runtime registry / synthetic audio checks
- graph create-node proof for `compound.raw-energy`
- `analyzer_family_report.json` writing

## Preserved External Contract

CLI:

```text
--dump-c6-analyzer-family-proof-and-exit
```

Artifact:

```text
debug/c6-analyzer-family-proof/analyzer_family_report.json
```

Stable JSON fields:

```text
kind = c6AnalyzerFamilyProof
ok
operation = analyzer_compound_family_seed
familyEntryCount
visibleRegistryContainsRawEnergy
runtimeRegistryContainsRawEnergy
runtimeCoverageStatus
createdRawEnergyNode
loudnessStillPresent
error
```

## Implementation

- `source/app/C6AnalyzerFamilyProofRunner.h`
- `source/app/C6AnalyzerFamilyProofRunner.cpp`
- `tests/C6AnalyzerFamilyProofRunnerTests.cpp`
- `my_world_proof_reports` CMake library so app proof runners and app shell share the same report serializer.

`MainComponent::dumpC6AnalyzerFamilyProof()` now builds a small request, calls `runC6AnalyzerFamilyProof()`, and maps the result to UI status text.

## Verification

Red:

```text
cmake -S . -B build
```

Expected failure before runner implementation:

```text
Cannot find source file:
  source/app/C6AnalyzerFamilyProofRunner.cpp
```

Green:

```text
cmake -S . -B build
cmake --build build --target my_world_c6_analyzer_family_proof_runner_tests
cmake --build build --target my-world
ctest --test-dir build --output-on-failure -R "c6_analyzer_family_proof_runner|analyzer_compound_family"
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-analyzer-family-proof-and-exit
```

Accepted result:

- `analyzer_compound_family` passed.
- `c6_analyzer_family_proof_runner` passed.
- app target built.
- CLI proof exited 0.
- `analyzer_family_report.json` retained `ok: true`, `familyEntryCount: 2`, `runtimeCoverageStatus: "ready"`, and `loudnessStillPresent: true`.

## Parked

- C2-C5 proof orchestration remains in `MainComponent`.
- C6 AI repair-loop proof remains in `MainComponent`.
- V1 shader proof remains tied to `OpenGLShaderPreview`.
