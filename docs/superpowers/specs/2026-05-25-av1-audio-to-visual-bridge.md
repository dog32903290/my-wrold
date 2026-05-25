# AV1 Audio-to-Visual Bridge

## Status

Closed on 2026-05-25.

## Acceptance

- App/control-rate `shader.uniform` evidence can feed the native preview path.
- `MainComponent::tickLiveIOControl()` converts latest `u_loudness` evidence into a `ShaderPreviewInputSnapshot`.
- `OpenGLShaderPreview` exposes `setInputSnapshot()` and converts it through `ShaderPreviewInputBridge`.
- Preview loudness input updates from the bridge snapshot while preserving the existing direct analyzer loudness fallback.
- No pixel-diff visual proof, graph node mapping, OpenGL uniform expansion, Metal work, full mapping editor, or realtime callback send is added.

## Target Line

```text
LiveIOStatusIndicatorState shader uniform evidence
-> MainComponent::tickLiveIOControl()
-> ShaderPreviewInputSnapshot
-> OpenGLShaderPreview::setInputSnapshot()
-> existing preview loudness render input
```

## Evidence

- `source/app/MainComponent.cpp`
- `source/render/OpenGLShaderPreview.h`
- `source/render/OpenGLShaderPreview.cpp`
- `CMakeLists.txt`

## Contract Notes

Before AV1, `MainComponent::updateAudioMeters()` already called `preview.setLoudness(snapshot.loudness)`. That path remains as fallback. AV1 adds the real bridge path from Live IO `shader.uniform` evidence to preview input:

```text
indicator.hasShaderUniform
-> makeShaderPreviewInputFromUniformEvidence(...)
-> preview.setInputSnapshot(...)
-> makeRenderFrameInputFromShaderPreviewInput(...)
-> setLoudness(input.loudness)
```

This proves the app has a load-bearing audio-to-visual bridge at the native preview boundary. It does not yet prove pixels changed.

## Verification

- RED: `cmake --build build --target my-world` failed first because `OpenGLShaderPreview::setInputSnapshot()` did not exist.
- GREEN: `cmake --build build --target my-world`
- Full suite: `ctest --test-dir build --output-on-failure` passed 77/77.
- Diff check: `git diff --check` passed.

## Parked

- AV2 pixel-diff visual reaction proof.
- OpenGL backend uniform list expansion beyond existing `u_loudness`.
- Graph node mapping for audio-to-visual routes.
- Full mapping editor.
- Metal backend.
