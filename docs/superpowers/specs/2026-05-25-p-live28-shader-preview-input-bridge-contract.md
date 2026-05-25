# P-LIVE28 Shader Preview Input Bridge Contract

## Status

Closed on 2026-05-25.

## Acceptance

- A JUCE-free `ShaderPreviewInputBridge` defines the uniform snapshot shape the shader preview can consume later.
- The bridge can build a snapshot from shader uniform evidence.
- Invalid evidence fails with an explicit status/message.
- The bridge can dump a stable JSON snapshot.
- No OpenGL backend wiring, shader preview smoke-read, live binding, graph node, mapping editor, or realtime callback work is added.

## Target Line

```text
live_io_shader_uniform_report.json shape
-> ShaderPreviewInputBridge
-> ShaderPreviewInputSnapshot JSON
-> focused bridge test
```

## Evidence

- `source/render/ShaderPreviewInputBridge.h`
- `source/render/ShaderPreviewInputBridge.cpp`
- `tests/ShaderPreviewInputBridgeTests.cpp`
- `CMakeLists.txt`

## Contract Notes

The bridge is intentionally not connected to `OpenGLShaderPreview` yet. It defines a renderer-facing data object:

```text
bindingId + uniformName + value + sampleCounter
-> ShaderPreviewInputSnapshot
```

P-LIVE29 can smoke-read this shape without pretending the native preview is already live-bound.

## Verification

- RED: `cmake -S . -B build` failed first because `source/render/ShaderPreviewInputBridge.cpp` did not exist.
- GREEN: `cmake -S . -B build && cmake --build build --target my_world_shader_preview_input_bridge_tests && ./build/my_world_shader_preview_input_bridge_tests`
- App build: `cmake --build build --target my-world`
- Full suite: `ctest --test-dir build --output-on-failure` passed 76/76.
- Diff check: `git diff --check` passed.

## Parked

- Shader preview smoke-read.
- OpenGL backend wiring.
- Shader preview live binding.
- Full mapping editor.
- Graph IO node.
