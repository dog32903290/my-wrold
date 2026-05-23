# C1.19 Loaded Loudness Runtime Bridge

Date: 2026-05-24 01:58 Asia/Taipei

## Behavior

```text
loaded compound runtime publicOutputs
-> loudness runtime bridge snapshot
-> UI/debug surface reads out/rms/peak/gate/confidence vocabulary
-> direct AudioInputAnalyzer snapshot remains fallback
```

## Acceptance Trace

Loaded runtime path:

```text
fixtures/module-libraries/default.module-library.json
-> executeRuntimeRegistryWithSyntheticAudio()
-> RuntimeExecutionSnapshot publishes publicOutputs out/rms/peak/gate/confidence
-> makeLoudnessRuntimeBridgeSnapshot()
-> sourceMode loaded-runtime-publicOutputs
-> analyzer snapshot reads loaded public output values
-> sampleCounter remains from direct fallback snapshot
```

Direct fallback path:

```text
AudioInputAnalyzer::getSnapshot()
-> makeLoudnessRuntimeBridgeSnapshot(empty runtime snapshot, fallback)
-> sourceMode direct-analyzer-fallback
-> publicOutputs keeps out/rms/peak/gate/confidence ids
-> publicOutputSources names audioInputAnalyzer fields
```

App proof path:

```text
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-audio-proof-and-exit
-> debug/a1-audio-proof/audio_stats.json
-> debug/a1-audio-proof/loudness_compound.json
-> debug/a1-audio-proof/loudness_runtime_bridge.json
```

## Proven

```text
source/core/RuntimeRegistry.h
source/core/RuntimeRegistry.cpp
source/app/MainComponent.cpp
tests/RuntimeRegistryTests.cpp
debug/a1-audio-proof/loudness_runtime_bridge.json
```

## Limits

```text
The realtime audio callback still only writes bounded AudioAnalyzerState.
The loaded runtime path is proven through synthetic RuntimeExecutionSnapshot, not live callback buffers.
The live UI currently passes an empty runtime snapshot and therefore reports direct-analyzer-fallback until a safe live sample-window runner exists.
No JSON parsing, runtime graph mutation, file IO, logging, locking, or allocation was added to the audio callback.
```

## Follow-On

```text
C1.20 live-safe loudness runtime proof is now implemented:
AudioAnalyzerState/direct snapshot
-> non-realtime prepared RuntimeSyntheticAudioInput window
-> loaded compound runtime execution snapshot
-> bridge sourceMode loaded-runtime-publicOutputs during app proof
```
