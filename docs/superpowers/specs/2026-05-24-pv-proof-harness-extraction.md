# PV Proof Harness Extraction

Date: 2026-05-24 23:17 Asia/Taipei.

## Contract

Extract PV detector proof orchestration out of `MainComponent` without changing proof evidence.

Two requirements both stay active:

- `MainComponent` keeps only adapter duties: trigger proof, show status text, quit after startup dump.
- PV detector proof evidence keeps the existing artifact contract:
  `*_report.json`, `cook_order.json`, `node_stats.json`, `errors.json`.

## Boundary

New runner:

```text
PVDetectorProofRunRequest
-> runPVDetectorProof()
-> PVDetectorProofRunResult
```

Runner owns:

- PV proof kind to fixture/library/report mapping.
- Candidate root lookup.
- Runtime registry loading.
- Fixture loading.
- Artifact directory clear/create.
- Failure report writing.

`MainComponent` owns:

- CLI/startup/button trigger path.
- Status label text.
- `quitAfterStartupDump`.

## Closed Proof

Added:

- `source/app/PVDetectorProofRunner.h`
- `source/app/PVDetectorProofRunner.cpp`
- `tests/PVDetectorProofRunnerTests.cpp`

MainComponent PV detector methods now call one facade:

```text
dumpPVDetectorProof(PVDetectorProofKind)
```

Existing PV-B1 analyzer environment proof remains in `MainComponent` because it is a visible-catalog promotion proof, not part of the detector proof family.

## Verification

Commands run:

```text
cmake -S . -B build
cmake --build build --target my_world_pv_detector_proof_runner_tests
ctest --test-dir build --output-on-failure -R pv_detector_proof_runner
cmake --build build --target my-world
ctest --test-dir build --output-on-failure -R "analyzer_detector|analyzer_density_detector|analyzer_silence_detector|analyzer_sustain_detector|analyzer_residue_detector|analyzer_aggregate_pressure|pv_detector_proof_runner"
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-attack-detector-proof-and-exit
```

Observed:

- `pv_detector_proof_runner` passed.
- 7 focused analyzer/PV tests passed.
- `my-world` target built.
- CLI attack proof exited 0 and wrote `debug/pv-attack-detector-proof/{attack_detector_report.json,cook_order.json,node_stats.json,errors.json}` with `ok: true`.

## Parked

- C2-C6 proof orchestration is still mostly in `MainComponent`.
- PV-B1 analyzer environment proof is still in `MainComponent`.
- V1 shader proof stays tied to `OpenGLShaderPreview` until a headless/live render proof boundary is selected.
