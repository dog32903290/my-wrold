# C1.20 Live-Safe Loudness Runtime Proof

Date: 2026-05-24 02:07 Asia/Taipei

## Behavior

```text
AudioAnalyzerSnapshot
-> non-realtime snapshot-shaped RuntimeSyntheticAudioInput
-> loaded compound runtime execution snapshot
-> loudness bridge sourceMode loaded-runtime-publicOutputs
-> app audio proof writes execution and bridge evidence
```

## Acceptance Trace

Core helper:

```text
AudioAnalyzerSnapshot { rms, peak, loudness, gate, confidence, sampleCounter }
-> makeRuntimeSyntheticAudioInputFromAnalyzerSnapshot(snapshot, 8)
-> one mono channel, 8 samples, alternating +/- rms
-> analysisGain 1.0 because snapshot values are already analyzed scale
-> executeRuntimeRegistryWithSyntheticAudio(default registry, input)
-> makeLoudnessRuntimeBridgeSnapshot(execution.snapshot, snapshot)
-> sourceMode loaded-runtime-publicOutputs
```

App proof:

```text
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-audio-proof-and-exit
-> debug/a1-audio-proof/audio_stats.json keeps direct analyzer facts
-> debug/a1-audio-proof/loudness_runtime_execution.json records seven loaded child RuntimeOps
-> debug/a1-audio-proof/loudness_runtime_bridge.json records usesLoadedRuntimeOutputs true
```

## Proven

```text
source/core/RuntimeRegistry.h
source/core/RuntimeRegistry.cpp
source/app/MainComponent.cpp
tests/RuntimeRegistryTests.cpp
debug/a1-audio-proof/loudness_runtime_execution.json
debug/a1-audio-proof/loudness_runtime_bridge.json
```

## Limits

```text
The synthetic input is shaped from AudioAnalyzerSnapshot, not copied from raw callback buffers.
The app proof now exercises loaded runtime publicOutputs, but live UI timer still uses direct fallback unless a prepared runtime snapshot is cached.
Peak in loaded runtime proof equals the snapshot-shaped rms amplitude; audio_stats.json remains the direct measurement source for true observed peak.
The realtime audio callback still has no JSON parsing, file IO, graph mutation, or runtime execution.
```

## Follow-On

```text
C1.21 compound public-port surface proof is now implemented:
collapsed compound node
-> visible public input/output ports from loaded module
-> connect/disconnect public ports through command path
-> expanded child graph remains parent-qualified and roundtrippable
```
