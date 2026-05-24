# R Segment Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete the R runtime/render backbone segment without implementing Metal or expanding the visual node library beyond the first headless `image.constant` proof.

**Architecture:** R1 moves the existing V1 OpenGL shader proof behind a real `RenderBackend` boundary while preserving current behavior. R2 adds a headless `image.constant -> output.texture_summary` runtime proof from the existing fixture. R3 closes the segment by making V1 and R2 share compatible `cook_order.json` / `node_stats.json` evidence and updating the master plan.

**Tech Stack:** C++20, CMake, JUCE OpenGL for the existing app proof, existing `GraphContract` JSON helpers, focused executable tests, app proof dumps.

---

## Contract Snapshot

R1 module boundary:

| 問題 | 狀態 | 答案 |
| --- | --- | --- |
| Trigger | ok | `OpenGLShaderPreview` calls the backend during GL context creation, shader source edits, render frames, proof capture, and GL context shutdown. |
| Input | ok | GLSL fragment source, viewport width/height/scale, frame index, elapsed time, loudness value. |
| Success | ok | Compile success replaces the live program; render draws the same full-screen shader; capture returns RGBA pixels for `frame.png`. |
| Failure | ok | Compile failure keeps the previous valid program and returns a visible error message; capture with no frame returns an empty artifact/error status instead of crashing. |
| Log | ok | `lastStatus()`, `backendName()`, and existing `node_stats.json` renderer/status fields carry the proof. |

R2 node contract:

```text
Node: image.constant
Question: what constant RGBA texture metadata exists for this runtime proof?
Conversion: color + resolution -> texture.rgba summary
Family: headless texture source
Inputs: none
Outputs: out texture.rgba
Parameters:
  - color: operation, default [0.02, 0.02, 0.02, 1.0], affects outputSummary, saved yes
  - resolution: operation, default [1280, 720], affects outputSummary, saved yes
State: none
Failure: invalid resolution or malformed color rejects the proof and writes errors evidence
Diagnostics: node id, type, cookDomain render, status, resolution, format, color
Evidence: fixtures/runtime/top_constant_to_output.graph.json -> debug/r2-top-constant artifacts
Split / compound decision: single source node; no compound
```

## Task 1: R1 RenderBackend Contract

**Files:**
- Modify: `source/render/RenderBackend.h`
- Create: `tests/RenderBackendContractTests.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Write the failing contract test**

Create `tests/RenderBackendContractTests.cpp` with a fake backend that overrides:

```cpp
const char* backendName() const override;
std::string lastStatus() const override;
ShaderCompileResult compileShader (const std::string& fragmentSource) override;
void resize (int width, int height, float scale) override;
void renderFrame (const RenderFrameInput& input) override;
CapturedFrame captureFrame() const override;
void release() override;
```

The test asserts:

```text
backendName() == "fake"
compileShader("void main(){}").ok == true
renderFrame receives frameIndex 7 and loudness 0.25
captureFrame returns width 2, height 1, and 8 RGBA bytes
release changes lastStatus() to "released"
```

- [x] **Step 2: Run RED**

Run:

```bash
cmake --build build --target my_world_render_backend_contract_tests
```

Expected: fail because `RenderFrameInput`, `CapturedFrame`, and the richer virtual methods do not exist yet.

- [x] **Step 3: Implement minimal contract**

Update `source/render/RenderBackend.h` with:

```cpp
struct RenderFrameInput;
struct CapturedFrame;
class RenderBackend;
```

The interface must include backend name, last status, compile, resize, render, capture, and release. Keep the type free of JUCE so fake/headless tests can use it.

- [x] **Step 4: Run GREEN**

Run:

```bash
cmake --build build --target my_world_render_backend_contract_tests
./build/my_world_render_backend_contract_tests
```

Expected: executable prints `render backend contract ok`.

- [x] **Step 5: Commit**

Commit only the contract test, `RenderBackend.h`, `CMakeLists.txt`, and this plan checkbox update.

## Task 2: R1 OpenGL Backend Extraction

**Files:**
- Create: `source/render/OpenGLRenderBackend.h`
- Create: `source/render/OpenGLRenderBackend.cpp`
- Create: `tests/OpenGLRenderBackendTests.cpp`
- Modify: `source/render/OpenGLShaderPreview.h`
- Modify: `source/render/OpenGLShaderPreview.cpp`
- Modify: `source/app/MainComponent.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Write the failing OpenGL backend smoke test**

Create `tests/OpenGLRenderBackendTests.cpp` that constructs `juce::OpenGLContext`, constructs `OpenGLRenderBackend`, and asserts:

```text
backend.backendName() == "OpenGL"
backend.lastStatus() == "waiting for GL context"
```

- [x] **Step 2: Run RED**

Run:

```bash
cmake --build build --target my_world_opengl_render_backend_tests
```

Expected: fail because `OpenGLRenderBackend.h` and the test target do not exist yet.

- [x] **Step 3: Implement OpenGL backend**

Move these responsibilities from `OpenGLShaderPreview` into `OpenGLRenderBackend`:

```text
shader program ownership
position attribute and system uniforms
full-screen quad VAO/VBO
GL viewport and clear
uniform binding for u_time, u_resolution, u_frame, u_loudness
glReadPixels capture into CapturedFrame
GL resource release
```

`OpenGLShaderPreview` keeps:

```text
JUCE component and OpenGLContext attachment
ImGui overlay/input bridge
shader source submission callback
save_work and publish_module callbacks
proof dump scheduling and file writing
```

`MainComponent` keeps proof artifacts inside the local repo when the app is launched from the repo working directory, so the post-iCloud-move proof run does not create a Desktop shadow project.

- [x] **Step 4: Run GREEN and R1 app proof**

Run:

```bash
cmake --build build --target my_world_opengl_render_backend_tests my-world
./build/my_world_opengl_render_backend_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
test -s debug/v1-shader-proof/frame.png
test -s debug/v1-shader-proof/cook_order.json
test -s debug/v1-shader-proof/node_stats.json
rg -n "\"renderer\": \"OpenGL\"" debug/v1-shader-proof/node_stats.json
```

Expected: test prints `opengl render backend ok`; proof artifacts exist and keep renderer `OpenGL`.

- [x] **Step 5: Nearby regression gate**

Run:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
git diff --check
```

Expected: all configured tests pass and diff check is clean.

- [x] **Step 6: Commit**

Commit only R1 extraction files, this plan checkbox update, and corresponding master/spec status updates.

Evidence:

```text
RED: cmake --build build --target my_world_opengl_render_backend_tests failed before OpenGLRenderBackend existed.
GREEN: my_world_opengl_render_backend_tests printed "opengl render backend ok".
V1 proof: --dump-proof-and-exit wrote debug/v1-shader-proof/{frame.png,cook_order.json,node_stats.json}; node_stats.json keeps "renderer": "OpenGL".
Local repo guard: the same proof run from /Users/chenbaiwei/Projects/我的世界 did not recreate /Users/chenbaiwei/Desktop/我的世界.
Regression: cmake --build build; ctest --test-dir build --output-on-failure => 32/32 passed; git diff --check clean.
```

## Task 3: R2 Headless image.constant Runtime

**Files:**
- Create: `source/render/HeadlessRenderRuntime.h`
- Create: `source/render/HeadlessRenderRuntime.cpp`
- Create: `tests/HeadlessRenderRuntimeTests.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing runtime test**

Create `tests/HeadlessRenderRuntimeTests.cpp` that runs:

```text
fixtures/runtime/top_constant_to_output.graph.json
-> debug/r2-top-constant
```

The test asserts:

```text
result.ok == true
texture_summary.json exists and contains "width": 1280, "height": 720, "format": "rgba8"
cook_order.json names "const1" before "out1"
node_stats.json contains "type": "image.constant" and "cookDomain": "render"
errors.json contains "ok": true
invalid resolution fixture text fails with "invalid resolution"
unknown node type fixture text fails with "unsupported node type"
```

- [ ] **Step 2: Run RED**

Run:

```bash
cmake --build build --target my_world_headless_render_runtime_tests
```

Expected: fail because `HeadlessRenderRuntime` API does not exist yet.

- [ ] **Step 3: Implement minimal headless runtime**

Implement only:

```text
runtimeProofFixture parser for image.constant + output.texture_summary
positive validation for one texture edge const1.out -> out1.input
invalid resolution failure
unknown node type failure
texture_summary.json, cook_order.json, node_stats.json, errors.json writers
```

Do not implement `image.blur`, SOP/MAT/POINT, GPU textures, or UI exposure.

- [ ] **Step 4: Run GREEN**

Run:

```bash
cmake --build build --target my_world_headless_render_runtime_tests
./build/my_world_headless_render_runtime_tests
```

Expected: executable prints `headless render runtime ok`.

- [ ] **Step 5: Nearby regression gate**

Run:

```bash
ctest --test-dir build --output-on-failure
git diff --check
```

Expected: all configured tests pass and diff check is clean.

- [ ] **Step 6: Commit**

Commit only R2 runtime files, test, plan checkbox update, and corresponding master/spec status updates.

## Task 4: R3 Segment Closure

**Files:**
- Modify: `docs/superpowers/specs/2026-05-24-r-runtime-render-backbone-roadmap.md`
- Modify: `docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md`
- Modify: `docs/superpowers/plans/2026-05-24-r-segment-implementation.md`

- [ ] **Step 1: Verify R evidence from fresh commands**

Run:

```bash
cmake --build build --target my_world_render_backend_contract_tests my_world_opengl_render_backend_tests my_world_headless_render_runtime_tests my-world
./build/my_world_render_backend_contract_tests
./build/my_world_opengl_render_backend_tests
./build/my_world_headless_render_runtime_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```

Expected: all commands exit 0.

- [ ] **Step 2: Self-review the R diff**

Review changed files for:

```text
contract bypass
context drift
shallow proof
new dependency drift
manual ownership risk
unrelated refactor
```

Record any issue in the final report or fix it before closure.

- [ ] **Step 3: Close R in docs**

Update the R roadmap and master progress with:

```text
R1 closed evidence
R2 closed evidence
R3 proof compatibility note
R4 Metal readiness remains parked: no Metal code entered
Next candidate lane after R: PV/analyzer detector expansion or explicitly selected lane
```

- [ ] **Step 4: Commit**

Commit closure docs and final plan checkbox update.
