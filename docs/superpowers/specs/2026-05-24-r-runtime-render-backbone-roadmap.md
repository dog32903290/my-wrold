# R Runtime/Render Backbone Roadmap

Date: 2026-05-24 15:54 Asia/Taipei

## Status

This spec routes the next lane after H1 closure.

It does not implement Metal, new visual nodes, node thumbnails, or a production render graph. It defines the order in which the current proof-only OpenGL shader preview becomes a runtime/render backbone that can later carry Metal and visual node vocabulary without hiding state inside UI code.

## One Line First

The first implementation line after this roadmap is:

```text
makeDefaultShaderOutputGraph()
-> OpenGL proof backend behind RenderBackend
-> debug/v1-shader-proof/frame.png + cook_order.json + node_stats.json still pass
```

The first new runtime-node line after the backend extraction is:

```text
fixtures/runtime/top_constant_to_output.graph.json
-> headless image.constant runtime
-> debug/r2-top-constant/texture_summary.json + cook_order.json + node_stats.json + errors.json
```

These are separate lines. R1 changes render ownership for the already-proven V1 shader proof. R2 proves the first headless texture runtime node. Do not combine them into a Metal spike or a broad node-library pass.

## Current Evidence

Already proven:

```text
V1 native shader preview:
native app -> hand-written shader.fragment -> output.preview -> frame.png

A1 audio bridge:
audio analyzer snapshot -> u_loudness -> shader preview proof metadata

C lanes:
command/storage/runtime registry/module publish/repair loop are closed

H1 cleanup:
UI overlay, RuntimeRegistry, and StorageContract splits are closed
```

Current pressure point:

```text
RenderBackend exists as a thin interface.
OpenGLShaderPreview still owns shader compile, GL resources, render loop, framebuffer readback, proof file writing, ImGui input, and UI callbacks.
```

That shape was acceptable for V1. It cannot carry Metal, live node thumbnails, high-resolution previews, output view modes, or texture runtime fixtures without becoming the next oversized surface.

## Ownership Contract

### Graph

Source of truth:

```text
GraphContract.runtimeGraph
fixtures/runtime/*.graph.json
```

The UI can edit or display the graph, but render/runtime proof must be reproducible from graph state and debug inputs. No R slice may introduce render state that exists only in an ImGui widget or a JUCE component member.

### Preview Component

`OpenGLShaderPreview` remains the UI hand:

```text
JUCE Component
OpenGL context attachment
ImGui overlay frame/input bridge
visible callbacks for save_work and publish_module
proof dump request scheduling
```

It should stop being the owner of backend semantics:

```text
shader program lifecycle
uniform binding
render target/readback
backend name/status
backend proof artifact data
```

### Render Backend

`RenderBackend` must become a real boundary before Metal:

```text
compileShader
resize
renderFrame
readPixels or captureFrame
release resources
report backend name and last status
```

The first concrete backend is OpenGL because it preserves the existing proof. Metal remains the production direction but enters only after OpenGL bears the interface.

### Runtime

Headless runtime nodes must use text fixtures and write artifacts:

```text
graph fixture
-> parser/validator
-> cook order
-> node stats
-> output summary
-> errors
```

For the first R runtime fixture, `image.constant` only needs texture metadata and a color summary. It does not need a GPU texture or `frame.png`.

## Roadmap

| Slice | Status | One-line proof | Files likely touched | Verification |
| --- | --- | --- | --- | --- |
| R0 roadmap | closed by this spec | master plan routes R lane | `docs/superpowers/specs/2026-05-24-r-runtime-render-backbone-roadmap.md`, master progress | `git diff --check` |
| R1 OpenGL backend extraction | next | default shader graph -> OpenGLRenderBackend -> V1 proof artifacts unchanged | `source/render/RenderBackend.h`, new `source/render/OpenGLRenderBackend.*`, `source/render/OpenGLShaderPreview.*`, `CMakeLists.txt` | build app, `--dump-proof-and-exit`, check `frame.png`, `cook_order.json`, `node_stats.json`, `git diff --check` |
| R2 headless texture summary runtime | queued | `top_constant_to_output.graph.json` -> `texture_summary.json` | new render/runtime core files, fixture if current one needs tightening, focused tests | new runtime test target, headless dump command or test fixture, `ctest`, `git diff --check` |
| R3 runtime/render proof unification | queued | runtimeGraph render status -> shared proof JSON fields | `GraphContract.*`, render proof helpers, app proof command code | V1 proof plus R2 headless proof both write compatible `cook_order` and `node_stats` evidence |
| R4 Metal readiness gate | parked | OpenGL backend interface audit -> Metal first-slice decision | spec only until R1-R3 close | no Metal code before the audit names the smallest frame-producing slice |

## R1 Acceptance Contract

R1 is closed only when:

```text
OpenGLShaderPreview no longer directly owns shader compile/link objects as its main render abstraction.
The existing V1 proof dump still produces frame.png, cook_order.json, and node_stats.json.
Compile success still replaces the live program.
Compile failure still keeps the last valid frame and reports the error.
u_time, u_resolution, u_frame, and u_loudness still bind for the default shader.
The app still builds as the same JUCE target.
No Metal, node thumbnails, analyzer detector semantics, or storage schema work enters the slice.
```

Suggested R1 verification:

```text
cmake --build build --target my-world
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
test -s debug/v1-shader-proof/frame.png
test -s debug/v1-shader-proof/cook_order.json
test -s debug/v1-shader-proof/node_stats.json
rg -n "\"renderer\": \"OpenGL\"" debug/v1-shader-proof/node_stats.json
ctest --test-dir build --output-on-failure
git diff --check
```

## R2 Acceptance Contract

R2 is closed only when:

```text
fixtures/runtime/top_constant_to_output.graph.json is the source fixture.
The runtime can validate image.constant -> output.texture_summary without UI.
The output summary records resolution, format, color, node id, and status.
cook_order.json names image.constant before output.texture_summary.
node_stats.json records cook domain, cook status, and output summary.
errors.json exists and is empty or has an explicit ok status for the valid fixture.
Invalid resolution or unknown node type produces errors.json without pretending success.
```

Suggested R2 verification:

```text
cmake --build build --target my_world_render_runtime_tests
./build/my_world_render_runtime_tests
ctest --test-dir build --output-on-failure
git diff --check
```

## Non-Goals

```text
do not implement Metal in R1 or R2
do not add WGPU, bgfx, MoltenVK, or Vulkan
do not add production node thumbnails
do not add image.blur before image.constant proof closes
do not add SOP, MAT, POINT, timeline, export window, or render queue
do not change storage JSON schema
do not reopen C4/C5/C6 command or AI worker behavior
do not move graph mutation out of the command path
```

## Conflict Resolution

The older TiXL node-function spec names `top_constant_to_output.graph.json` as the first runtime slice. This roadmap keeps that as the first new runtime-node slice, but places OpenGL backend extraction before it because the current V1 preview still hides backend ownership inside `OpenGLShaderPreview`.

This is not a license to expand R1. R1 only moves the existing OpenGL proof behind a backend boundary. R2 starts new runtime node execution.

## Next Handoff Sentence

Open the master progress plan first, then this spec. The next implementation lane is R1 OpenGL proof backend extraction behind `RenderBackend`; keep Metal and new visual node vocabulary parked until R1 closes.
