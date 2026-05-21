# Native Canvas Skeleton Design

Date: 2026-05-22
Status: Initial design for repo scaffold

## Purpose

`我的世界` is the new native body for 柏為的畫布.

This is not a Web wrapper and not a generic C++ port. The first skeleton must prove that a local app can host live shader editing, native audio analysis, compound patchers, and eventually AI worker collaboration without inheriting LiteGraph as the law.

## Locked Decisions

- The repo lives at `/Users/chenbaiwei/Desktop/我的世界`.
- Use JUCE + CMake for the first native app skeleton.
- Build a standalone app first, not a plugin.
- Use display name `我的世界`; keep internal target names ASCII, starting with `my-world`.
- Use OpenGL/GLSL for the first shader backend to preserve GLSL vibe-coding speed.
- Do not hardwire the app to OpenGL. Create a `RenderBackend` boundary so Metal can replace or sit beside it later.
- Old Web canvas works are migration fixtures, not the native graph law.

## First Stage Proofs

### V1 Visual Proof

Goal:

```text
native app -> hand-written Shader node -> Output preview
```

Contract:

- The app opens as a native desktop app.
- A default graph contains one Shader node connected to Output.
- The user can edit fragment shader text.
- Compile success updates the preview.
- Compile failure keeps the last valid frame and shows an error.
- The runtime can dump `frame.png`, `cook_order.json`, and `node_stats.json` for proof.

### A1 Audio Proof

Goal:

```text
native preferences -> audio input -> native analyzer meter rows
```

Contract:

- Preferences include audio input device, channel/mono mix, sample rate, buffer size, analysis gain, and analyzer profile.
- The realtime audio path only measures or writes bounded realtime-safe state.
- UI reads analyzer values outside the realtime callback.
- First values include at least `rms`, `peak`, and `loudness`.
- MIDI preferences are present early: output device, channel, CC map, stream on/off, and map mode.

### C1 Compound Proof

Goal:

```text
loudness compound patcher -> expanded child patchers -> collapsed public ports
```

Contract:

- `loudness` can appear as one collapsed node.
- It can expand to show child patchers such as `AudioIn`, `MonoMix`, `RMS`, `AnalysisGain`, `PreGate`, `OutputSmoother`, and `LoudnessOut`.
- The compound exposes public ports such as `loudness.out`.
- It can also expose selected inner ports such as `loudness.rms`, `loudness.peak`, `loudness.gate`, and `loudness.confidence`.

## Architecture

### Data Layers

Keep four separate truths:

```text
editorGraph        node positions, selection, expanded/collapsed state, UI metadata
runtimeGraph       executable graph for render/audio/control runtime
commandGraph       formal mutation operations used by UI, AI, importers, and scripts
collaborationLog   AI tasks, commands, diagnostics, repair attempts, and proof evidence
```

### C++ Boundary

Avoid one giant class per node. Use this split:

```text
NodeSpec      static description: ports, params, widgets, compound children
NodeInstance  per-work state: values, position, exposed ports, editor flags
NodeView      generic drawing and interaction
RuntimeOp     execution logic for render/audio/control domains
Command       single mutation entrypoint
```

### Render Backend

Initial implementation:

```text
RenderBackend interface
OpenGLBackend implementation
```

Minimum backend methods:

```text
compileShader
createRenderTarget
renderNode
readPixels
resize
destroy
```

Metal is parked, not rejected. The backend boundary exists so the first app is not trapped by OpenGL.

## Audio Analyzer Migration

The analyzer should borrow from:

- `/Users/chenbaiwei/Documents/GitHub/sound-in-area-analyzer-plugin`
- `/Users/chenbaiwei/Desktop/vibe coding/0512＿現場表演系統01/sound_in_area/src/analyzer`

Do not copy it as one black-box node. Split analyzer work into:

```text
Raw Facts
rms, peak, diffMean, zeroCrossHz

Feature
loudness, highMotion, zcrShape, registerEstimate, active

Detector / State
attackRaw, attackEnvelope, onsetEvent, densityEnvelope, sustainEnvelope, silenceTimer, residueEnvelope

Aggregate
globalPressure, breathiness, instability, sourceLegibility, rupture, residue

Output Shaping
gate, curve, smooth, outMin/outMax, MIDI CC, shader uniform mapping
```

First audio-visual closure:

```text
audio input -> loudness meter -> shader uniform u_loudness -> Output preview reacts
```

## AI Worker Position

AI worker is a first-class future client of the graph, not a chat box.

Initial command-loop shape:

```text
natural language task
-> command list
-> graph mutation
-> compile/render/analyze proof
-> repair loop
-> final state + evidence
```

The first skeleton does not need full AI UI, but it must not design graph mutation paths that AI cannot use later.

## Non-Goals For First Skeleton

- No full old Web project importer.
- No black velvet material demo as the first proof.
- No Metal backend implementation yet.
- No plugin build target yet.
- No complete 13-patch analyzer UI yet.
- No full AI worker UI yet.

## Open Questions

- Exact JUCE version / dependency acquisition method.
- Whether to vendor JUCE, use CPM/FetchContent, or reference an existing local JUCE checkout.
- Exact first shader editor component choice.
- Exact first graph serialization format once `NodeSpec` is drafted.

These are implementation questions, not design blockers.

