# AV3 Real OpenGL Capture Pair

## Status

Closed on 2026-05-25.

## Acceptance

- Shader preview proof dump captures the current OpenGL-rendered frame before writing V1 artifacts.
- In the same active OpenGL context, proof dump renders a quiet frame with loudness `0.0` and a loud frame with loudness `1.0`.
- The quiet/loud OpenGL frames are passed into `V1ShaderProofArtifacts`, which writes `visual_reaction.json`.
- The preview restores the active loudness render after the proof pair capture.
- No offscreen renderer, graph node mapping, mapping editor, direct realtime send, or Metal backend work is added.

## Target Line

```text
OpenGLShaderPreview proof dump
-> render loudness 0.0 / 1.0 in the active GL context
-> V1ShaderProofArtifacts quiet/loud frames
-> visual_reaction.json from real preview capture path
```

## Evidence

- `source/render/OpenGLShaderPreview.cpp`
- `docs/superpowers/specs/2026-05-25-av2-visual-reaction-proof.md`

## Contract Notes

AV3 closes the gap left by AV2: the visual reaction frame pair now comes from the shader preview's real OpenGL render backend when an app proof dump is requested.

This is not a standalone headless/offscreen GL test. It deliberately reuses the app preview context because the current JUCE/OpenGL boundary is component-owned.

## Verification

- App build: `cmake --build build --target my-world`
- Full suite: `ctest --test-dir build --output-on-failure`
- Diff check: `git diff --check`

## Parked

- Headless/offscreen OpenGL capture harness.
- Graph IO node mapping.
- Full mapping editor.
- Direct realtime callback send.
- Metal backend.
