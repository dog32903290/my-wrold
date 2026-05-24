# A1 Audio Proof Harness Extraction

Date: 2026-05-25
Status: closed

## Load-Bearing Goal

Move A1 audio proof orchestration out of `MainComponent` while preserving the existing CLI flag, artifact names, analyzer snapshot evidence, loaded loudness runtime execution, and runtime bridge evidence.

The app shell may keep:

- startup trigger
- status label text
- quit-after-dump scheduling
- collection of the live analyzer snapshot, sample rate, buffer size, and preferences

The runner must own:

- output directory creation
- default module library candidate lookup
- synthetic runtime input creation from `AudioAnalyzerSnapshot`
- loaded runtime registry execution
- loudness compound JSON artifact writing
- runtime execution JSON artifact writing
- runtime bridge JSON artifact writing
- audio stats JSON artifact writing

## Preserved External Contract

CLI:

```text
--dump-audio-proof-and-exit
```

Artifacts:

```text
debug/a1-audio-proof/audio_stats.json
debug/a1-audio-proof/loudness_compound.json
debug/a1-audio-proof/loudness_runtime_execution.json
debug/a1-audio-proof/loudness_runtime_bridge.json
```

Stable JSON fields:

```text
audio_stats.json:
sampleRate
bufferSize
rms
peak
loudness
gate
confidence
active
analysisGain
midi.streamEnabled
midi.mapModeEnabled
midi.channel
midi.loudnessCc
midi.mapCc
midi.outputName
sampleCounter

loudness_runtime_execution.json:
kind = runtimeExecution
nodeType = compound.loudness
status = computed
childId = audio_in / mono_mix / rms / analysis_gain / pre_gate / output_smoother / loudness_out

loudness_runtime_bridge.json:
kind = loudnessRuntimeBridge
sourceMode = loaded-runtime-publicOutputs
usesLoadedRuntimeOutputs = true
```

## Implementation

- `source/app/A1AudioProofRunner.h`
- `source/app/A1AudioProofRunner.cpp`
- `tests/A1AudioProofRunnerTests.cpp`

`MainComponent::dumpAudioProof()` now builds a small request from the live analyzer state, calls `runA1AudioProof()`, and maps the result to UI status text.

## Verification

Red:

```text
cmake -S . -B build
```

Expected failure before runner implementation:

```text
Cannot find source file:
  source/app/A1AudioProofRunner.cpp
```

Green:

```text
cmake -S . -B build
cmake --build build --target my_world_a1_audio_proof_runner_tests
cmake --build build --target my-world
ctest --test-dir build --output-on-failure -R "a1_audio_proof_runner|audio_analyzer_state|runtime_registry|performance_preferences"
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-audio-proof-and-exit
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted result:

- `a1_audio_proof_runner` passed.
- `audio_analyzer_state` passed.
- `runtime_registry` passed.
- `performance_preferences` passed.
- app target built.
- CLI proof exited 0.
- The CLI wrote all four A1 artifacts and retained loaded-runtime bridge evidence.
- Full `ctest` passed 49/49.
- `git diff --check` passed.

## Parked

- V1 shader proof remains tied to `OpenGLShaderPreview`.
