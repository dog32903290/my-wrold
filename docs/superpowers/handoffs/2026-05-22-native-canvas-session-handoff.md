# Native Canvas Session Handoff - 2026-05-22

This file is the restart line for the next Codex session.

## Repo State

- Repo: `/Users/chenbaiwei/Desktop/我的世界`
- Current branch: `main`
- Latest implementation commit before this docs handoff: `44b6db6 Add loudness compound patch contract`
- Do not assume Web canvas or LiteGraph is the native law. The native contract is being built from proof lines.
- CodeGraph is initialized locally for this repo; `.codegraph/` is ignored and used only as a C++ symbol/caller/impact development index.

## What Is Proven

### V1 Visual

- Native JUCE app opens as `我的世界`.
- Default graph is hand-written fragment shader -> output preview.
- Compile success swaps to the new shader.
- Compile failure keeps last valid frame and reports the error.
- Shader preview receives `u_loudness` as a system uniform.
- Proof dump writes:

```text
debug/v1-shader-proof/frame.png
debug/v1-shader-proof/cook_order.json
debug/v1-shader-proof/node_stats.json
debug/v1-shader-proof/loudness_compound.json
```

### S0 Storage

- Minimal storage contract names `WorkProject`, `PatchDocument`, `ModulePackage`, `WorkLibrary`, and `ModuleLibrary`.
- Save statuses are explicit: `clean`, `save-ok commit-pending`, `saved-and-committed`, `save-ok commit-failed`, `validation-failed`, and `write-failed`.
- Minimal work fixture stores a reloadable main patch with `shader1 -> out1`.
- `Command+S` local commit behavior is specified but not yet wired into the production app shortcut.

### G0 Graph Language

- Graph language contract includes nodes, regions, typed edges, stream kinds, port binding modes, command vocabulary, and external compiler-worker boundary.
- Type vocabulary includes `audio.channels`, `audio.mono`, `signal.float`, `texture.rgba`, `geometry.mesh`, `event.midi`, `command.graph`, and future generic slots.
- AI-safe mutation rule is locked: UI, AI worker, importers, and scripts use typed commands, not direct JSON surgery.

### A0 UI / Node Contract

- Dear ImGui is mounted in the OpenGL render loop as the first UI adapter.
- No JUCE `Component` NodeView trap has been introduced.
- Node taxonomy exists with stable `type`, separate `category` / `subcategory`, `runtimeDomain`, port `dataType`, preview policy, and human manual path.
- Patch interaction smoke lowers Tooll3-like gestures to command names such as `create_node`, `connect`, `set_param`, `enter_patch`, and `undo`.

### A1 Audio / MIDI

- JUCE audio input bridge exists.
- Realtime callback only writes bounded analyzer state and clears output buffers.
- Analyzer snapshot exposes `rms`, `peak`, `loudness`, `gate`, `confidence`, `active`, and `sampleCounter`.
- UI reads analyzer state outside the callback and displays meter rows.
- Audio/MIDI preferences panel exists:
  - audio input device
  - channel / mono mix
  - sample rate
  - buffer size
  - analysis gain
  - analyzer profile
  - MIDI output device
  - channel
  - CC map
  - stream on/off
  - map mode
- Default loudness MIDI mapping is CC20.
- A1 proof dump writes:

```text
debug/a1-audio-proof/audio_stats.json
debug/a1-audio-proof/loudness_compound.json
```

Known gate: macOS microphone permission cannot be silently self-granted. First live mic run requires the user to click Allow. Do not redesign this into "audio off by default" unless 柏為 asks again; the current preference is to keep live input ready and use synthetic/unit proofs for unattended tests.

### C1 Loudness Compound

- First mother patch is `compound.loudness`.
- Expanded child chain:

```text
AudioIn -> MonoMix -> RMS -> AnalysisGain -> PreGate -> OutputSmoother -> LoudnessOut
```

- Public outputs:

```text
out
rms
peak
gate
confidence
```

- Child patchers are also standalone node specs:

```text
audio.mono_mix
analyzer.rms
analyzer.analysis_gain
analyzer.pre_gate
signal.smoother
analyzer.loudness_out
```

- ImGui smoke overlay shows the C1 loudness compound in collapsed/expanded debug form.
- Proof JSON is created by `makeCompoundPatchJson()`.

## Build And Test Gates

Use these after reopening:

```bash
cd /Users/chenbaiwei/Desktop/我的世界
git status --short
cmake --build build --target my-world
ctest --test-dir build --output-on-failure
codegraph status /Users/chenbaiwei/Desktop/我的世界
```

Last known full gate: `ctest` passed 8/8 and `cmake --build build --target my-world` passed.
Last known CodeGraph gate: 36 C++ files, 349 nodes, 784 edges, 0.95 MB, index up to date.

Avoid launching the app just to test docs or pure contracts; launching may hit the macOS microphone permission prompt if the app has not already been allowed.

## Load-Bearing Files

- `source/core/CompoundPatch.h`
- `source/core/CompoundPatch.cpp`
- `source/core/NodeSpec.cpp`
- `source/core/GraphLanguage.cpp`
- `source/audio/AudioAnalyzerState.h`
- `source/audio/AudioAnalyzerState.cpp`
- `source/render/OpenGLShaderPreview.h`
- `source/render/OpenGLShaderPreview.cpp`
- `source/ui/ImGuiSmokeOverlay.h`
- `source/ui/ImGuiSmokeOverlay.cpp`
- `source/app/MainComponent.cpp`

## Still Parked

- RenderBackend extraction. OpenGL/GLSL is the proof backend, not final Apple Silicon GPU strategy.
- Production ImGui node canvas with actual drag/drop/collapse/enter/exit compound editing.
- Runtime compilation of compound child graph into independent `RuntimeOp` cook order.
- Full 0519-style 13-patch analyzer expansion: attack, density, sustain, silence, residue, aggregate pressure, and MIDI/shader routing.
- Storage-backed compound/module publication.
- AI worker command loop with proof/repair evidence.

## Recommended Next Line

Do C1.1 before growing the analyzer:

```text
fixtures/compounds/loudness.compound.json
-> parse/validate CompoundPatchSpec
-> dump same proof shape as debug/loudness_compound.json
```

Reason: the loudness mother patch currently exists as C++ contract and debug JSON. The next line should make it reloadable as a saved module/compound before adding more analyzer patches.

## Skills To Use Next Session

- `native-canvas-spine` for native canvas direction.
- `cpp-patch-runtime` for graph/runtime/debug snapshot contract.
- `analyzer-patch-architect` when expanding loudness into attack/density/sustain/silence modules.
- `low-latency-audio-spine` when touching audio callback, device routing, buffer size, MIDI timing, or realtime swaps.
