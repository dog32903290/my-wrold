# P-LIVE29 Shader Preview Smoke-Read

## Status

Closed on 2026-05-25.

## Acceptance

- `ShaderPreviewInputBridge` can convert a `ShaderPreviewInputSnapshot` into `RenderFrameInput`.
- `u_loudness` is read from the snapshot and assigned to `RenderFrameInput::loudness`.
- A fake render backend can consume the converted frame input in a smoke test.
- No OpenGL backend wiring, shader preview live binding, visual reaction claim, graph node, mapping editor, or realtime callback work is added.

## Target Line

```text
ShaderPreviewInputSnapshot
-> RenderFrameInput
-> fake RenderBackend.renderFrame()
-> smoke-read test
```

## Evidence

- `source/render/ShaderPreviewInputBridge.h`
- `source/render/ShaderPreviewInputBridge.cpp`
- `tests/ShaderPreviewSmokeReadTests.cpp`
- `CMakeLists.txt`

## Contract Notes

This lane proves the render-side contract can read a uniform snapshot. It does not prove the native OpenGL preview is reacting visually. That belongs after P-LIVE30 under a new A/V bridge lane, not inside Live IO.

## Verification

- RED: `cmake -S . -B build && cmake --build build --target my_world_shader_preview_smoke_read_tests` failed first because `makeRenderFrameInputFromShaderPreviewInput()` did not exist.
- GREEN: `cmake --build build --target my_world_shader_preview_smoke_read_tests && ./build/my_world_shader_preview_smoke_read_tests`
- App build: `cmake --build build --target my-world`
- Full suite: `ctest --test-dir build --output-on-failure` passed 77/77.
- Diff check: `git diff --check` passed.

## Parked

- OpenGL backend wiring.
- Shader preview live binding.
- Visual reaction proof.
- Full mapping editor.
- Graph IO node.
