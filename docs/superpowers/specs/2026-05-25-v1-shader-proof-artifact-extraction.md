# V1 Shader Proof Artifact Extraction

Date: 2026-05-25
Status: closed

## Load-Bearing Goal

Move V1 shader proof artifact orchestration out of `OpenGLShaderPreview` while preserving the existing CLI flag, live OpenGL frame capture, artifact names, runtime registry evidence, and proof status behavior.

This is not a headless V1 rewrite. The preview component still owns:

- live OpenGL repaint trigger
- active frame capture through `RenderBackend::captureFrame()`
- conversion from captured frame bytes to `juce::Image`
- UI status callback

The artifact writer owns:

- output directory creation
- PNG writing
- V1 `cook_order.json` and `node_stats.json` writing
- loudness compound artifact writing
- runtime registry / op catalog / coverage / UI diagnostics artifacts
- runtime dry-run and synthetic execution artifacts
- missing-runtime-op negative fixture artifacts
- artifact success/failure aggregation

## Preserved External Contract

CLI:

```text
--dump-proof-and-exit
```

Artifacts:

```text
debug/v1-shader-proof/cook_order.json
debug/v1-shader-proof/frame.png
debug/v1-shader-proof/loudness_compound.json
debug/v1-shader-proof/node_stats.json
debug/v1-shader-proof/runtime_dry_run.json
debug/v1-shader-proof/runtime_execution.json
debug/v1-shader-proof/runtime_missing_runtimeop_coverage.json
debug/v1-shader-proof/runtime_missing_runtimeop_dry_run.json
debug/v1-shader-proof/runtime_missing_runtimeop_execution.json
debug/v1-shader-proof/runtime_missing_runtimeop_registry.json
debug/v1-shader-proof/runtime_op_catalog.json
debug/v1-shader-proof/runtime_op_coverage.json
debug/v1-shader-proof/runtime_registry.json
debug/v1-shader-proof/runtime_ui_diagnostics.json
```

Stable evidence:

```text
frame.png is nonempty
node_stats.json has version = 1 and renderer = OpenGL
runtime_execution.json has kind = runtimeExecution
runtime_execution.json has nodeType = compound.loudness
runtime_execution.json has status = computed
runtime_missing_runtimeop_execution.json has status = missing-runtime-op
runtime_ui_diagnostics.json keeps ready analyzer/PV compounds and missing-runtime-op diagnostic
```

## Implementation

- `source/render/V1ShaderProofArtifacts.h`
- `source/render/V1ShaderProofArtifacts.cpp`
- `tests/V1ShaderProofArtifactsTests.cpp`

`OpenGLShaderPreview::handlePendingProofDump()` now captures the frame, builds `V1ShaderProofArtifactRequest`, calls `writeV1ShaderProofArtifacts()`, and maps the result back to the existing proof status callback.

## Verification

Red:

```text
cmake -S . -B build
```

Expected failure before artifact writer implementation:

```text
Cannot find source file:
  source/render/V1ShaderProofArtifacts.cpp
```

Green:

```text
cmake -S . -B build
cmake --build build --target my_world_v1_shader_proof_artifacts_tests
ctest --test-dir build --output-on-failure -R "v1_shader_proof_artifacts|runtime_registry|opengl_render_backend|render_backend_contract"
cmake --build build --target my-world
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted result:

- `v1_shader_proof_artifacts` passed.
- `runtime_registry` passed.
- `opengl_render_backend` passed.
- `render_backend_contract` passed.
- app target built.
- CLI proof exited 0.
- `frame.png` was nonempty.
- all 14 V1 proof artifacts were written.
- V1 runtime evidence retained `runtimeExecution`, `compound.loudness`, `computed`, and missing-runtime-op diagnostics.
- Full `ctest` passed 50/50.
- `git diff --check` passed.

## Parked

- V1 still depends on live OpenGL for `frame.png`.
- A future headless visual proof should use `HeadlessRenderRuntime` / `RenderBackend`, not this extraction as a substitute.
