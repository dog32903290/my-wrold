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
| R1 OpenGL backend extraction | closed | default shader graph -> OpenGLRenderBackend -> V1 proof artifacts unchanged | `source/render/RenderBackend.h`, new `source/render/OpenGLRenderBackend.*`, `source/render/OpenGLShaderPreview.*`, `source/app/MainComponent.cpp`, `CMakeLists.txt` | backend test, build app, `--dump-proof-and-exit`, check `frame.png`, `cook_order.json`, `node_stats.json`, `ctest`, `git diff --check` |
| R2 headless texture summary runtime | closed | `top_constant_to_output.graph.json` -> `texture_summary.json` | `source/render/HeadlessRenderRuntime.*`, `tests/HeadlessRenderRuntimeTests.cpp`, `CMakeLists.txt` | `my_world_headless_render_runtime_tests`, `ctest` 33/33, `git diff --check` |
| R3 runtime/render proof unification | closed | runtimeGraph render status -> shared proof JSON fields | `source/render/HeadlessRenderRuntime.cpp`, `tests/HeadlessRenderRuntimeTests.cpp`, R roadmap, master progress, implementation plan | V1 and R2 both expose `version`, `cookOrder`, node `cookDomain`, node `status`, and renderer evidence |
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
cmake --build build --target my_world_opengl_render_backend_tests my-world
./build/my_world_opengl_render_backend_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
test -s debug/v1-shader-proof/frame.png
test -s debug/v1-shader-proof/cook_order.json
test -s debug/v1-shader-proof/node_stats.json
rg -n "\"renderer\": \"OpenGL\"" debug/v1-shader-proof/node_stats.json
cmake --build build
ctest --test-dir build --output-on-failure
git diff --check
```

R1 closed evidence on 2026-05-24: `my_world_opengl_render_backend_tests` printed `opengl render backend ok`; `node_stats.json` kept `"renderer": "OpenGL"`; the app proof stayed inside `/Users/chenbaiwei/Projects/我的世界` and did not recreate `/Users/chenbaiwei/Desktop/我的世界`; full `ctest` passed 32/32.

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
cmake --build build --target my_world_headless_render_runtime_tests
./build/my_world_headless_render_runtime_tests
ctest --test-dir build --output-on-failure
git diff --check
```

R2 closed evidence on 2026-05-24: the RED build failed on missing `HeadlessRenderRuntime.h`; the GREEN target printed `headless render runtime ok`; `debug/r2-top-constant/texture_summary.json` reports 1280x720 rgba8; `cook_order.json` orders `const1` before `out1`; `node_stats.json` marks render-domain nodes; `errors.json` has `"ok": true`; invalid resolution and unsupported node type write failure evidence; full `ctest` passed 33/33.

## R3 Closure Contract

R3 is closed when:

```text
V1 `debug/v1-shader-proof/cook_order.json` and R2 `debug/r2-top-constant/cook_order.json` both expose versioned cook order evidence.
V1 and R2 `node_stats.json` both expose versioned node stats, renderer identity, node cookDomain, and node status.
The R segment still has no Metal, image.blur, node thumbnails, SOP/MAT/POINT, render export, storage schema work, or reopened C4/C5/C6 behavior.
The master progress plan marks R closed and leaves no next lane active by assumption.
```

R3 closed evidence on 2026-05-24: focused R tests printed `render backend contract ok`, `opengl render backend ok`, and `headless render runtime ok`; `--dump-proof-and-exit` refreshed V1 proof; V1 and R2 artifacts both expose `"version": 1`, cook order, renderer identity (`OpenGL` / `headless`), and render-domain node stats; full `ctest` passed 33/33; `git diff --check` passed.

## Non-Goals

```text
do not implement Metal in R1, R2, or R3
do not add WGPU, bgfx, MoltenVK, or Vulkan
do not add production node thumbnails
do not add image.blur before R closes
do not add SOP, MAT, POINT, timeline, export window, or render queue
do not change storage JSON schema
do not reopen C4/C5/C6 command or AI worker behavior
do not move graph mutation out of the command path
```

## Conflict Resolution

The older TiXL node-function spec names `top_constant_to_output.graph.json` as the first runtime slice. This roadmap keeps that as the first new runtime-node slice, but places OpenGL backend extraction before it because the current V1 preview still hides backend ownership inside `OpenGLShaderPreview`.

This is not a license to expand R. R1 only moves the existing OpenGL proof behind a backend boundary. R2 starts new runtime node execution. R3 only aligns proof evidence and closes the segment.

## Next Handoff Sentence

Open the master progress plan first. R runtime/render backbone is closed through R3; select the next lane explicitly before editing. Metal and new visual node vocabulary remain parked.
