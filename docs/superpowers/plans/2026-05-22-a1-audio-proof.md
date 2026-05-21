# A0/A1 ImGui And Audio Proof Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Prevent the JUCE `Component` NodeView trap, define the first node taxonomy contract, then build the first native audio proof: audio input enters a realtime-safe analyzer, UI shows `rms` / `peak` / `loudness`, and the app can dump audio proof evidence.

**Architecture:** Keep graph/node identity independent from the drawing library. `NodeSpec` / `NodeInstance` / `PortSpec` / `ParamSpec` are stable data contracts; human manuals live in Markdown and machine-readable behavior lives in the registry. Dear ImGui is the first graphics-side UI adapter, and JUCE `Component` remains shell/status UI only. Node surfaces follow a vvvv-like minimal patch style: compact names, pins, tiny state marks, and doc/patch icons instead of card-heavy explanations. Parameter controls follow the Tooll3-inspired rule that manual values remain stored while connected or animated values can override live output; this is graph `PortBinding` state, not ImGui widget state. The audio callback writes bounded atomic analyzer state only; the message thread reads snapshots and updates meter rows. Proof dumps record whether live input was actually observed, so microphone permission blocks are explicit instead of hidden.

**Tech Stack:** C++20, JUCE `AudioDeviceManager` / `AudioIODeviceCallback`, JUCE OpenGL, Dear ImGui `v1.92.8`, CMake, existing native app shell.

---

## Overnight Gates

- Do not execute A0/A1 until `docs/superpowers/plans/2026-05-22-s0-storage-proof.md` and `docs/superpowers/plans/2026-05-22-g0-graph-language-contract.md` are complete or explicitly deferred by the user. Storage decides where graph truth lives; G0 decides what graph truth means.
- Safe to run unattended: pure analyzer tests, CMake wiring, UI build, static proof dump shape.
- May require user presence: first live microphone access. macOS can show a Microphone permission prompt for `我的世界`; an agent cannot safely click or pre-approve this without the user.
- If microphone permission blocks live input, commit code and plan updates with A1 marked as "implemented but live proof pending permission"; do not mark live A1 as proven.
- Use `caffeinate` during execution if the user explicitly starts an overnight run so the machine does not sleep mid-build.
- Do not ask for or store the user's password. If a specific `sudo` command becomes necessary, pause and explain the exact command and reason.

## File Map

- Modify: `CMakeLists.txt`
  - Add `my_world_node_specs`.
  - Add `my_world_node_spec_tests`.
  - Fetch Dear ImGui `v1.92.8` for a smoke overlay.
  - Add `my_world_imgui`.
  - Add `my_world_audio`.
  - Add `my_world_audio_analyzer_tests`.
  - Link `my-world` with `my_world_audio`, `my_world_imgui`, and `juce::juce_audio_devices`.

- Create: `source/core/NodeSpec.h`
  - Stable node taxonomy, port, param, human manual path, and runtime domain structs.

- Create: `source/core/NodeSpec.cpp`
  - First seed node specs and lookup helpers.

- Create: `tests/NodeSpecTests.cpp`
  - Tests category/runtime/data-type contracts and extension behavior.

- Create: `docs/nodes/analyzer.loudness.md`
  - First human-readable node manual.

- Create: `docs/nodes/README.md`
  - Node manual convention: human Markdown vs machine `NodeSpec`.

- Create: `source/ui/ImGuiSmokeOverlay.h`
  - Small Dear ImGui overlay wrapper.

- Create: `source/ui/ImGuiSmokeOverlay.cpp`
  - Creates ImGui context, renders a smoke window, and forwards OpenGL draw data.

- Modify: `source/render/OpenGLShaderPreview.h`
  - Own `ImGuiSmokeOverlay` and forward mouse input to ImGui.

- Modify: `source/render/OpenGLShaderPreview.cpp`
  - Initialise/shutdown/render the ImGui overlay inside the OpenGL render loop.

- Create: `source/audio/AudioAnalyzerState.h`
  - Pure C++ realtime-safe analyzer state and snapshot types.
  - No JUCE dependency.

- Create: `source/audio/AudioAnalyzerState.cpp`
  - RMS / peak / loudness calculation.
  - Atomic snapshot storage.

- Create: `source/audio/AudioInputAnalyzer.h`
  - JUCE audio callback bridge.
  - Owns `AudioAnalyzerState`.

- Create: `source/audio/AudioInputAnalyzer.cpp`
  - Starts/stops default input device.
  - Callback calls `AudioAnalyzerState::processBlock`.
  - Clears any output buffers.

- Modify: `source/app/MainComponent.h`
  - Own `juce::AudioDeviceManager`, `AudioInputAnalyzer`, meter labels, and audio proof dump helper.

- Modify: `source/app/MainComponent.cpp`
  - Start audio input.
  - Update meter rows from analyzer snapshots.
  - Dump `debug/a1-audio-proof/audio_stats.json`.

- Modify: `source/app/Main.cpp`
  - Add `--dump-audio-proof-and-exit`.

- Create: `tests/AudioAnalyzerStateTests.cpp`
  - Tests RMS, peak, loudness, silence, and multi-channel mono mix.

- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`
  - Keep A0/A1 status accurate after each task.

---

### Task 0: Node Taxonomy Contract

**Files:**
- Create: `source/core/NodeSpec.h`
- Create: `source/core/NodeSpec.cpp`
- Create: `tests/NodeSpecTests.cpp`
- Create: `docs/nodes/README.md`
- Create: `docs/nodes/analyzer.loudness.md`
- Modify: `CMakeLists.txt`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Write failing node spec tests**

Add `tests/NodeSpecTests.cpp`:

```cpp
#include "NodeSpec.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}
}

int main()
{
    const auto specs = myworld::makeSeedNodeSpecs();

    const auto* shader = myworld::findNodeSpec (specs, "shader.fragment");
    expect (shader != nullptr, "shader.fragment seed spec exists");
    expect (shader->category == "shader", "shader category");
    expect (shader->runtimeDomain == "render", "shader runtime domain");
    expect (shader->outputs.size() == 1, "shader output count");
    expect (shader->outputs[0].dataType == "texture.rgba", "shader output data type");

    const auto* loudness = myworld::findNodeSpec (specs, "analyzer.loudness");
    expect (loudness != nullptr, "analyzer.loudness seed spec exists");
    expect (loudness->category == "analyzer", "loudness category");
    expect (loudness->runtimeDomain == "audioAnalysis", "loudness runtime domain");
    expect (loudness->inputs[0].dataType == "audio.mono", "loudness input data type");
    expect (loudness->outputs[0].dataType == "signal.float", "loudness output data type");
    expect (loudness->humanDocPath == "docs/nodes/analyzer.loudness.md", "loudness human manual path");
    expect (loudness->machineSpecVersion == 1, "machine spec version");

    const auto* midi = myworld::findNodeSpec (specs, "midi.ccOut");
    expect (midi != nullptr, "midi.ccOut seed spec exists");
    expect (midi->category == "midi", "midi category");
    expect (midi->runtimeDomain == "control", "midi runtime domain");

    expect (myworld::isKnownNodeCategory ("audio"), "audio category is known");
    expect (myworld::isKnownNodeCategory ("top"), "top category is known");
    expect (! myworld::isKnownNodeCategory ("weather"), "unknown category stays unknown until registry extension");

    std::cout << "node spec contract ok\n";
    return 0;
}
```

- [ ] **Step 2: Wire the test target and verify RED**

Modify `CMakeLists.txt` near `my_world_core`:

```cmake
add_library(my_world_node_specs
    source/core/NodeSpec.cpp
)

target_include_directories(my_world_node_specs
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

add_executable(my_world_node_spec_tests
    tests/NodeSpecTests.cpp
)

target_link_libraries(my_world_node_spec_tests
    PRIVATE
        my_world_node_specs
)

add_test(NAME node_specs COMMAND my_world_node_spec_tests)
```

Run:

```bash
cmake --build build --target my_world_node_spec_tests
```

Expected: FAIL because `NodeSpec.h` does not exist yet.

- [ ] **Step 3: Implement minimal node taxonomy**

Create `source/core/NodeSpec.h`:

```cpp
#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct PortSpec
{
    std::string id;
    std::string label;
    std::string dataType;
    std::string direction;
};

struct ParamSpec
{
    std::string id;
    std::string label;
    std::string dataType;
    std::string defaultValue;
    std::string range;
};

struct NodeSpec
{
    std::string type;
    std::string displayName;
    std::string category;
    std::string runtimeDomain;
    std::string humanDocPath;
    int machineSpecVersion = 1;
    std::vector<PortSpec> inputs;
    std::vector<PortSpec> outputs;
    std::vector<ParamSpec> params;
};

std::vector<NodeSpec> makeSeedNodeSpecs();
const NodeSpec* findNodeSpec (const std::vector<NodeSpec>& specs, const std::string& type);
bool isKnownNodeCategory (const std::string& category);
}
```

Create `source/core/NodeSpec.cpp`:

```cpp
#include "NodeSpec.h"

#include <algorithm>
#include <array>

namespace myworld
{
std::vector<NodeSpec> makeSeedNodeSpecs()
{
    return {
        {
            "shader.fragment",
            "Fragment Shader",
            "shader",
            "render",
            "docs/nodes/shader.fragment.md",
            1,
            {},
            { { "output", "Output", "texture.rgba", "out" } },
            {
                { "source", "Source", "text.glsl", "", "" }
            }
        },
        {
            "output.preview",
            "Preview Output",
            "output",
            "render",
            "docs/nodes/output.preview.md",
            1,
            { { "input", "Input", "texture.rgba", "in" } },
            {},
            {}
        },
        {
            "audio.input",
            "Audio Input",
            "audio",
            "audio",
            "docs/nodes/audio.input.md",
            1,
            {},
            { { "mono", "Mono", "audio.mono", "out" } },
            {
                { "analysisGain", "Analysis Gain", "float", "1.0", "0.0..4.0" }
            }
        },
        {
            "analyzer.rms",
            "RMS",
            "analyzer",
            "audioAnalysis",
            "docs/nodes/analyzer.rms.md",
            1,
            { { "input", "Input", "audio.mono", "in" } },
            { { "rms", "RMS", "signal.float", "out" } },
            {}
        },
        {
            "analyzer.loudness",
            "Loudness",
            "analyzer",
            "audioAnalysis",
            "docs/nodes/analyzer.loudness.md",
            1,
            { { "input", "Input", "audio.mono", "in" } },
            { { "out", "Loudness", "signal.float", "out" } },
            {
                { "curve", "Curve", "float", "1.0", "0.25..4.0" },
                { "smooth", "Smooth", "float", "0.2", "0.0..1.0" }
            }
        },
        {
            "midi.ccOut",
            "MIDI CC Out",
            "midi",
            "control",
            "docs/nodes/midi.ccOut.md",
            1,
            { { "value", "Value", "signal.float", "in" } },
            {},
            {
                { "channel", "Channel", "int", "1", "1..16" },
                { "cc", "CC", "int", "1", "0..127" }
            }
        },
        {
            "top.texture",
            "Texture",
            "top",
            "render",
            "docs/nodes/top.texture.md",
            1,
            {},
            { { "output", "Output", "texture.rgba", "out" } },
            {}
        },
        {
            "sop.plane",
            "Plane",
            "sop",
            "geometry",
            "docs/nodes/sop.plane.md",
            1,
            {},
            { { "geometry", "Geometry", "geometry.mesh", "out" } },
            {}
        },
        {
            "mat.shader",
            "Shader Material",
            "mat",
            "render",
            "docs/nodes/mat.shader.md",
            1,
            { { "shader", "Shader", "shader.program", "in" } },
            { { "material", "Material", "material", "out" } },
            {}
        }
    };
}

const NodeSpec* findNodeSpec (const std::vector<NodeSpec>& specs, const std::string& type)
{
    const auto found = std::find_if (specs.begin(), specs.end(), [&type] (const NodeSpec& spec)
    {
        return spec.type == type;
    });

    return found == specs.end() ? nullptr : &*found;
}

bool isKnownNodeCategory (const std::string& category)
{
    static constexpr std::array<const char*, 10> categories {
        "audio", "analyzer", "signal", "midi", "shader",
        "top", "sop", "mat", "output", "compound"
    };

    return std::find (categories.begin(), categories.end(), category) != categories.end();
}
}
```

- [ ] **Step 4: Add first human node manuals**

Create `docs/nodes/README.md`:

```markdown
# Node Manuals

Human manuals and machine specs are separate.

- Human manuals live here as Markdown and explain use, examples, and common failure modes.
- Machine specs live in `NodeSpec` and define type, category, runtime domain, ports, params, and doc paths.
- Node surfaces should show a doc icon that opens the human manual; the patch surface should not print long explanations directly on nodes.
```

Create `docs/nodes/analyzer.loudness.md`:

```markdown
# Loudness

Use `analyzer.loudness` when you need a stable 0..1-ish signal representing trusted sound energy.

## Inputs

- `input` (`audio.mono`): mono audio measurement stream.

## Outputs

- `out` (`signal.float`): shaped loudness value.
- `rms` (`signal.float`): raw RMS-style energy when exposed by a compound.
- `peak` (`signal.float`): raw peak when exposed by a compound.
- `confidence` (`signal.float`): trust signal when exposed by a compound.

## Use

- Drive shader uniforms such as `u_loudness`.
- Drive MIDI CC values.
- Gate visuals only when the room or instrument is active.

## Failure Modes

- Threshold too high erases soft breath or quiet performance.
- Smooth too high makes the visual response late.
- Treating loudness as a semantic emotion signal will overclaim what the analyzer knows.
```

- [ ] **Step 5: Verify GREEN**

Run:

```bash
cmake --build build --target my_world_node_spec_tests
ctest --test-dir build --output-on-failure
```

Expected: `node_specs` and existing tests pass.

- [ ] **Step 6: Update spec and commit**

Update `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`:

```text
- Proven: first node taxonomy registry exists with categories `audio`, `analyzer`, `signal`, `midi`, `shader`, `top`, `sop`, `mat`, `output`, and `compound`.
- Proven: node `type` is stable identity; `category` is registry metadata and can move without changing saved graph identity.
- Proven: node specs point to human Markdown manuals while machine-readable behavior stays in `NodeSpec`.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add CMakeLists.txt source/core/NodeSpec.h source/core/NodeSpec.cpp tests/NodeSpecTests.cpp docs/nodes/README.md docs/nodes/analyzer.loudness.md docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Add node taxonomy contract"
```

---

### Task 0.5: Dear ImGui Smoke Overlay

**Files:**
- Modify: `CMakeLists.txt`
- Create: `source/ui/ImGuiSmokeOverlay.h`
- Create: `source/ui/ImGuiSmokeOverlay.cpp`
- Modify: `source/render/OpenGLShaderPreview.h`
- Modify: `source/render/OpenGLShaderPreview.cpp`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Fetch and build Dear ImGui**

Modify the top of `CMakeLists.txt`:

```cmake
include(FetchContent)
```

Inside the existing `if(EXISTS "${MY_WORLD_JUCE_DIR}/CMakeLists.txt")` block, after `add_subdirectory("${MY_WORLD_JUCE_DIR}" ...)`, add:

```cmake
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.8
)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

add_library(my_world_imgui
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    source/ui/ImGuiSmokeOverlay.cpp
)

target_include_directories(my_world_imgui
    PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ui
)

target_link_libraries(my_world_imgui
    PUBLIC
        juce::juce_opengl
)
```

In the `target_link_libraries(my-world PRIVATE ...)` block, add:

```cmake
my_world_imgui
```

Run:

```bash
cmake --build build --target my-world
```

Expected: FAIL because `source/ui/ImGuiSmokeOverlay.cpp` does not exist yet.

- [ ] **Step 2: Add ImGui smoke overlay wrapper**

Create `source/ui/ImGuiSmokeOverlay.h`:

```cpp
#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct NodeSpec;

class ImGuiSmokeOverlay
{
public:
    void initialise();
    void shutdown();
    void beginFrame (int width, int height, float scale, float deltaSeconds);
    void drawSmokePanel (const std::vector<NodeSpec>& nodeSpecs, const std::string& shaderStatus);
    void render();
    bool wantsMouse() const;
    void setMousePosition (float x, float y);
    void setMouseButton (int buttonIndex, bool isDown);
    void addMouseWheel (float deltaY);

private:
    bool initialised = false;
    float smokeValue = 0.35f;
};
}
```

Create `source/ui/ImGuiSmokeOverlay.cpp`:

```cpp
#include "ImGuiSmokeOverlay.h"

#include "NodeSpec.h"

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>

namespace myworld
{
void ImGuiSmokeOverlay::initialise()
{
    if (initialised)
        return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init ("#version 150");
    initialised = true;
}

void ImGuiSmokeOverlay::shutdown()
{
    if (! initialised)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    initialised = false;
}

void ImGuiSmokeOverlay::beginFrame (int width, int height, float scale, float deltaSeconds)
{
    if (! initialised)
        return;

    auto& io = ImGui::GetIO();
    io.DisplaySize = ImVec2 (static_cast<float> (width) / scale, static_cast<float> (height) / scale);
    io.DisplayFramebufferScale = ImVec2 (scale, scale);
    io.DeltaTime = deltaSeconds > 0.0f ? deltaSeconds : 1.0f / 60.0f;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

void ImGuiSmokeOverlay::drawSmokePanel (const std::vector<NodeSpec>& nodeSpecs, const std::string& shaderStatus)
{
    if (! initialised)
        return;

    ImGui::SetNextWindowPos (ImVec2 (16.0f, 16.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize (ImVec2 (360.0f, 220.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin ("A0 ImGui Smoke");
    ImGui::TextUnformatted ("Immediate-mode UI is active.");
    ImGui::Text ("Seed node specs: %d", static_cast<int> (nodeSpecs.size()));
    ImGui::SliderFloat ("smoke value", &smokeValue, 0.0f, 1.0f);
    ImGui::Separator();
    ImGui::TextWrapped ("%s", shaderStatus.c_str());
    ImGui::End();
}

void ImGuiSmokeOverlay::render()
{
    if (! initialised)
        return;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData (ImGui::GetDrawData());
}

bool ImGuiSmokeOverlay::wantsMouse() const
{
    return initialised && ImGui::GetIO().WantCaptureMouse;
}

void ImGuiSmokeOverlay::setMousePosition (float x, float y)
{
    if (initialised)
        ImGui::GetIO().AddMousePosEvent (x, y);
}

void ImGuiSmokeOverlay::setMouseButton (int buttonIndex, bool isDown)
{
    if (initialised)
        ImGui::GetIO().AddMouseButtonEvent (buttonIndex, isDown);
}

void ImGuiSmokeOverlay::addMouseWheel (float deltaY)
{
    if (initialised)
        ImGui::GetIO().AddMouseWheelEvent (0.0f, deltaY);
}
}
```

- [ ] **Step 3: Integrate ImGui into the OpenGL render loop**

In `source/render/OpenGLShaderPreview.h`, include:

```cpp
#include "ImGuiSmokeOverlay.h"
#include "NodeSpec.h"
```

Add to the class:

```cpp
void mouseMove (const juce::MouseEvent& event) override;
void mouseDown (const juce::MouseEvent& event) override;
void mouseDrag (const juce::MouseEvent& event) override;
void mouseUp (const juce::MouseEvent& event) override;
void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

void updateImGuiMousePosition (const juce::MouseEvent& event);

ImGuiSmokeOverlay imguiOverlay;
std::vector<NodeSpec> seedNodeSpecs;
double lastFrameSeconds = 0.0;
```

In `OpenGLShaderPreview::newOpenGLContextCreated()`:

```cpp
imguiOverlay.initialise();
seedNodeSpecs = makeSeedNodeSpecs();
lastFrameSeconds = startTimeSeconds;
```

In `OpenGLShaderPreview::renderOpenGL()`, after shader drawing and before proof dump:

```cpp
const auto deltaSeconds = static_cast<float> (nowSeconds - lastFrameSeconds);
lastFrameSeconds = nowSeconds;
imguiOverlay.beginFrame (juce::jmax (1, width), juce::jmax (1, height), scale, deltaSeconds);
imguiOverlay.drawSmokePanel (seedNodeSpecs, lastStatus.toStdString());
imguiOverlay.render();
```

In `releaseGLObjects()`:

```cpp
imguiOverlay.shutdown();
```

Add mouse forwarding:

```cpp
void OpenGLShaderPreview::updateImGuiMousePosition (const juce::MouseEvent& event)
{
    const auto scale = static_cast<float> (openGLContext.getRenderingScale());
    imguiOverlay.setMousePosition (event.position.x * scale, event.position.y * scale);
}

void OpenGLShaderPreview::mouseMove (const juce::MouseEvent& event)
{
    updateImGuiMousePosition (event);
}

void OpenGLShaderPreview::mouseDown (const juce::MouseEvent& event)
{
    updateImGuiMousePosition (event);
    imguiOverlay.setMouseButton (0, true);
}

void OpenGLShaderPreview::mouseDrag (const juce::MouseEvent& event)
{
    updateImGuiMousePosition (event);
}

void OpenGLShaderPreview::mouseUp (const juce::MouseEvent& event)
{
    updateImGuiMousePosition (event);
    imguiOverlay.setMouseButton (0, false);
}

void OpenGLShaderPreview::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    updateImGuiMousePosition (event);
    imguiOverlay.addMouseWheel (wheel.deltaY);
}
```

- [ ] **Step 4: Verify ImGui smoke proof**

Run:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
rm -rf debug/v1-shader-proof
perl -e 'alarm 12; exec @ARGV' ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
test -s debug/v1-shader-proof/frame.png
```

Expected:
- Build and tests pass.
- `frame.png` includes the shader frame with an `A0 ImGui Smoke` overlay window.
- The `smoke value` slider can be dragged when running the app interactively.
- This smoke window is not the production node surface; it only proves ImGui frame/input/render viability.

- [ ] **Step 5: Update spec and commit**

Update `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`:

```text
- Proven: Dear ImGui is mounted in the OpenGL render loop before production node editor work.
- Proven: first UI adapter is immediate-mode; no JUCE `Component` NodeView exists.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add CMakeLists.txt source/ui/ImGuiSmokeOverlay.h source/ui/ImGuiSmokeOverlay.cpp source/render/OpenGLShaderPreview.h source/render/OpenGLShaderPreview.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Add ImGui smoke overlay"
```

### Task 1: Pure Analyzer State

**Files:**
- Create: `source/audio/AudioAnalyzerState.h`
- Create: `source/audio/AudioAnalyzerState.cpp`
- Create: `tests/AudioAnalyzerStateTests.cpp`
- Modify: `CMakeLists.txt`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Write failing analyzer tests**

Add `tests/AudioAnalyzerStateTests.cpp`:

```cpp
#include "AudioAnalyzerState.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}

void expectNear (float actual, float expected, float tolerance, const std::string& message)
{
    expect (std::abs (actual - expected) <= tolerance, message);
}
}

int main()
{
    myworld::AudioAnalyzerState analyzer;

    const float channel0[] = { 0.0f, 0.5f, -0.5f, 1.0f };
    const float* channels[] = { channel0 };
    analyzer.processBlock (channels, 1, 4, 1.0f);

    auto snapshot = analyzer.getSnapshot();
    expectNear (snapshot.rms, std::sqrt ((0.0f + 0.25f + 0.25f + 1.0f) / 4.0f), 0.0001f, "rms");
    expectNear (snapshot.peak, 1.0f, 0.0001f, "peak");
    expectNear (snapshot.loudness, snapshot.rms, 0.0001f, "loudness follows rms for first proof");
    expect (snapshot.active, "non-silent input should be active");
    expect (snapshot.sampleCounter == 4, "sample counter");

    const float silent[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    const float* silentChannels[] = { silent };
    analyzer.processBlock (silentChannels, 1, 4, 1.0f);

    snapshot = analyzer.getSnapshot();
    expectNear (snapshot.rms, 0.0f, 0.0001f, "silence rms");
    expectNear (snapshot.peak, 0.0f, 0.0001f, "silence peak");
    expect (! snapshot.active, "silence inactive");
    expect (snapshot.sampleCounter == 8, "sample counter accumulates");

    const float left[] = { 1.0f, 1.0f };
    const float right[] = { -1.0f, -1.0f };
    const float* stereo[] = { left, right };
    analyzer.processBlock (stereo, 2, 2, 1.0f);

    snapshot = analyzer.getSnapshot();
    expectNear (snapshot.rms, 0.0f, 0.0001f, "mono mix can cancel opposite phase stereo");
    expectNear (snapshot.peak, 0.0f, 0.0001f, "mono mix peak");

    std::cout << "audio analyzer state ok\n";
    return 0;
}
```

- [ ] **Step 2: Wire the test target and verify RED**

Modify `CMakeLists.txt`:

```cmake
add_library(my_world_audio
    source/audio/AudioAnalyzerState.cpp
)

target_include_directories(my_world_audio
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/audio
)

add_executable(my_world_audio_analyzer_tests
    tests/AudioAnalyzerStateTests.cpp
)

target_link_libraries(my_world_audio_analyzer_tests
    PRIVATE
        my_world_audio
)

add_test(NAME audio_analyzer_state COMMAND my_world_audio_analyzer_tests)
```

Run:

```bash
cmake --build build --target my_world_audio_analyzer_tests
```

Expected: FAIL because `AudioAnalyzerState.h` does not exist yet.

- [ ] **Step 3: Implement minimal analyzer state**

Create `source/audio/AudioAnalyzerState.h`:

```cpp
#pragma once

#include <atomic>
#include <cstdint>

namespace myworld
{
struct AudioAnalyzerSnapshot
{
    float rms = 0.0f;
    float peak = 0.0f;
    float loudness = 0.0f;
    bool active = false;
    std::uint64_t sampleCounter = 0;
};

class AudioAnalyzerState
{
public:
    void processBlock (const float* const* inputChannels,
                       int numInputChannels,
                       int numSamples,
                       float analysisGain) noexcept;

    AudioAnalyzerSnapshot getSnapshot() const noexcept;

private:
    std::atomic<float> rms { 0.0f };
    std::atomic<float> peak { 0.0f };
    std::atomic<float> loudness { 0.0f };
    std::atomic<bool> active { false };
    std::atomic<std::uint64_t> sampleCounter { 0 };
};
}
```

Create `source/audio/AudioAnalyzerState.cpp`:

```cpp
#include "AudioAnalyzerState.h"

#include <algorithm>
#include <cmath>

namespace myworld
{
void AudioAnalyzerState::processBlock (const float* const* inputChannels,
                                       int numInputChannels,
                                       int numSamples,
                                       float analysisGain) noexcept
{
    if (inputChannels == nullptr || numInputChannels <= 0 || numSamples <= 0)
    {
        rms.store (0.0f, std::memory_order_relaxed);
        peak.store (0.0f, std::memory_order_relaxed);
        loudness.store (0.0f, std::memory_order_relaxed);
        active.store (false, std::memory_order_relaxed);
        return;
    }

    double sumSquares = 0.0;
    float blockPeak = 0.0f;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float mono = 0.0f;
        int channelsUsed = 0;

        for (int channel = 0; channel < numInputChannels; ++channel)
        {
            if (inputChannels[channel] != nullptr)
            {
                mono += inputChannels[channel][sample];
                ++channelsUsed;
            }
        }

        if (channelsUsed > 0)
            mono = (mono / static_cast<float> (channelsUsed)) * analysisGain;

        sumSquares += static_cast<double> (mono) * static_cast<double> (mono);
        blockPeak = std::max (blockPeak, std::abs (mono));
    }

    const auto blockRms = static_cast<float> (std::sqrt (sumSquares / static_cast<double> (numSamples)));
    rms.store (blockRms, std::memory_order_relaxed);
    peak.store (blockPeak, std::memory_order_relaxed);
    loudness.store (blockRms, std::memory_order_relaxed);
    active.store (blockPeak > 0.0001f, std::memory_order_relaxed);
    sampleCounter.fetch_add (static_cast<std::uint64_t> (numSamples), std::memory_order_relaxed);
}

AudioAnalyzerSnapshot AudioAnalyzerState::getSnapshot() const noexcept
{
    return {
        rms.load (std::memory_order_relaxed),
        peak.load (std::memory_order_relaxed),
        loudness.load (std::memory_order_relaxed),
        active.load (std::memory_order_relaxed),
        sampleCounter.load (std::memory_order_relaxed)
    };
}
}
```

- [ ] **Step 4: Verify GREEN**

Run:

```bash
cmake --build build --target my_world_audio_analyzer_tests
ctest --test-dir build --output-on-failure
```

Expected: `audio_analyzer_state` and `graph_contract` both pass.

- [ ] **Step 5: Update spec and commit**

Update A1 in `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`:

```text
- Proven: pure realtime-safe analyzer state can calculate rms, peak, loudness, active, and sampleCounter from input buffers.
- Not started: native audio device input bridge.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add CMakeLists.txt source/audio/AudioAnalyzerState.h source/audio/AudioAnalyzerState.cpp tests/AudioAnalyzerStateTests.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Add realtime audio analyzer state"
```

---

### Task 2: JUCE Audio Input Bridge

**Files:**
- Create: `source/audio/AudioInputAnalyzer.h`
- Create: `source/audio/AudioInputAnalyzer.cpp`
- Modify: `CMakeLists.txt`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Add callback bridge header**

Create `source/audio/AudioInputAnalyzer.h`:

```cpp
#pragma once

#include "AudioAnalyzerState.h"

#include <juce_audio_devices/juce_audio_devices.h>

#include <atomic>

namespace myworld
{
class AudioInputAnalyzer final : public juce::AudioIODeviceCallback
{
public:
    void audioDeviceAboutToStart (juce::AudioIODevice* device) override;
    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                           int numInputChannels,
                                           float* const* outputChannelData,
                                           int numOutputChannels,
                                           int numSamples,
                                           const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceStopped() override;

    AudioAnalyzerSnapshot getSnapshot() const noexcept;
    double getSampleRate() const noexcept;
    int getBufferSize() const noexcept;
    void setAnalysisGain (float newGain) noexcept;

private:
    AudioAnalyzerState analyzerState;
    std::atomic<float> analysisGain { 1.0f };
    std::atomic<double> sampleRate { 0.0 };
    std::atomic<int> bufferSize { 0 };
};
}
```

- [ ] **Step 2: Add callback bridge implementation**

Create `source/audio/AudioInputAnalyzer.cpp`:

```cpp
#include "AudioInputAnalyzer.h"

#include <cstring>

namespace myworld
{
void AudioInputAnalyzer::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    if (device != nullptr)
    {
        sampleRate.store (device->getCurrentSampleRate(), std::memory_order_relaxed);
        bufferSize.store (device->getCurrentBufferSizeSamples(), std::memory_order_relaxed);
    }
}

void AudioInputAnalyzer::audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                                           int numInputChannels,
                                                           float* const* outputChannelData,
                                                           int numOutputChannels,
                                                           int numSamples,
                                                           const juce::AudioIODeviceCallbackContext&)
{
    analyzerState.processBlock (inputChannelData,
                                numInputChannels,
                                numSamples,
                                analysisGain.load (std::memory_order_relaxed));

    for (int channel = 0; channel < numOutputChannels; ++channel)
        if (outputChannelData[channel] != nullptr)
            std::memset (outputChannelData[channel], 0, sizeof (float) * static_cast<size_t> (numSamples));
}

void AudioInputAnalyzer::audioDeviceStopped()
{
    sampleRate.store (0.0, std::memory_order_relaxed);
    bufferSize.store (0, std::memory_order_relaxed);
}

AudioAnalyzerSnapshot AudioInputAnalyzer::getSnapshot() const noexcept
{
    return analyzerState.getSnapshot();
}

double AudioInputAnalyzer::getSampleRate() const noexcept
{
    return sampleRate.load (std::memory_order_relaxed);
}

int AudioInputAnalyzer::getBufferSize() const noexcept
{
    return bufferSize.load (std::memory_order_relaxed);
}

void AudioInputAnalyzer::setAnalysisGain (float newGain) noexcept
{
    analysisGain.store (newGain, std::memory_order_relaxed);
}
}
```

- [ ] **Step 3: Link JUCE audio devices**

Modify `CMakeLists.txt`:

```cmake
if(EXISTS "${MY_WORLD_JUCE_DIR}/CMakeLists.txt")
    # after add_subdirectory("${MY_WORLD_JUCE_DIR}" ...)
target_sources(my_world_audio
    PRIVATE
        source/audio/AudioInputAnalyzer.cpp
)

target_link_libraries(my_world_audio
    PUBLIC
        juce::juce_audio_devices
)

target_link_libraries(my-world
    PRIVATE
        my_world_audio
        juce::juce_audio_devices
)
endif()
```

Keep `my_world_audio_analyzer_tests` linked only to the pure `AudioAnalyzerState.cpp` path. It must keep running even if the JUCE checkout is missing and only core tests are configured.

- [ ] **Step 4: Verify build**

Run:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

Expected: full build and tests pass.

- [ ] **Step 5: Update spec and commit**

Update A1 in `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`:

```text
- Proven: JUCE audio callback bridge exists and only writes bounded analyzer state plus output silence.
- Not proven: live microphone input on this Mac until permission is granted and meter values move.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add CMakeLists.txt source/audio/AudioInputAnalyzer.h source/audio/AudioInputAnalyzer.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Add native audio input bridge"
```

---

### Task 3: Meter Rows In Main UI

**Files:**
- Modify: `source/app/MainComponent.h`
- Modify: `source/app/MainComponent.cpp`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Add UI ownership**

Modify `source/app/MainComponent.h`:

```cpp
#include "AudioInputAnalyzer.h"
```

Make `MainComponent` inherit `private juce::Timer` and add:

```cpp
void timerCallback() override;
void startAudioInput();
void updateAudioMeters();

juce::AudioDeviceManager audioDeviceManager;
AudioInputAnalyzer audioInputAnalyzer;
juce::Label audioStatusLabel;
juce::Label rmsLabel;
juce::Label peakLabel;
juce::Label loudnessLabel;
```

- [ ] **Step 2: Start audio input**

In `source/app/MainComponent.cpp`, add:

```cpp
void MainComponent::startAudioInput()
{
    const auto error = audioDeviceManager.initialiseWithDefaultDevices (1, 0);

    if (error.isNotEmpty())
    {
        audioStatusLabel.setText ("audio input error: " + error, juce::dontSendNotification);
        return;
    }

    audioDeviceManager.addAudioCallback (&audioInputAnalyzer);
    audioStatusLabel.setText ("audio input ready", juce::dontSendNotification);
}
```

In the constructor:

```cpp
audioStatusLabel.setText ("audio input starting", juce::dontSendNotification);
addAndMakeVisible (audioStatusLabel);
addAndMakeVisible (rmsLabel);
addAndMakeVisible (peakLabel);
addAndMakeVisible (loudnessLabel);
startAudioInput();
startTimerHz (30);
```

In the destructor:

```cpp
stopTimer();
audioDeviceManager.removeAudioCallback (&audioInputAnalyzer);
```

- [ ] **Step 3: Update meter rows**

Add:

```cpp
void MainComponent::timerCallback()
{
    updateAudioMeters();
}

void MainComponent::updateAudioMeters()
{
    const auto snapshot = audioInputAnalyzer.getSnapshot();
    rmsLabel.setText ("rms " + juce::String (snapshot.rms, 4), juce::dontSendNotification);
    peakLabel.setText ("peak " + juce::String (snapshot.peak, 4), juce::dontSendNotification);
    loudnessLabel.setText ("loudness " + juce::String (snapshot.loudness, 4), juce::dontSendNotification);

    if (audioInputAnalyzer.getSampleRate() > 0.0)
        audioStatusLabel.setText ("audio input "
                                  + juce::String (audioInputAnalyzer.getSampleRate(), 0)
                                  + "Hz / "
                                  + juce::String (audioInputAnalyzer.getBufferSize())
                                  + " samples",
                                  juce::dontSendNotification);
}
```

Place the labels in `resized()` above or below the shader editor without covering the preview.

- [ ] **Step 4: Verify UI build**

Run:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

Then run:

```bash
perl -e 'alarm 8; exec @ARGV' ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界
```

Expected:
- App opens.
- If macOS asks for Microphone permission, stop and ask the user to click Allow.
- If permission is already granted, meter rows appear and build remains stable.

- [ ] **Step 5: Update spec and commit**

Update A1:

```text
- Proven: UI reads analyzer snapshots outside the realtime callback and displays rms / peak / loudness rows.
- Not proven: live microphone values if macOS permission was not granted.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add source/app/MainComponent.h source/app/MainComponent.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Show audio analyzer meter rows"
```

---

### Task 4: A1 Audio Proof Dump

**Files:**
- Modify: `source/app/Main.cpp`
- Modify: `source/app/MainComponent.h`
- Modify: `source/app/MainComponent.cpp`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Add command-line mode**

In `source/app/Main.cpp`, parse:

```cpp
const auto dumpAudioProofAndExit = commandLine.contains ("--dump-audio-proof-and-exit");
```

Pass it into `MainComponent`.

- [ ] **Step 2: Add audio proof dump helper**

In `MainComponent`, add:

```cpp
void dumpAudioProof();
```

Implementation:

```cpp
void MainComponent::dumpAudioProof()
{
    const auto directory = projectDirectory().getChildFile ("debug").getChildFile ("a1-audio-proof");
    directory.createDirectory();

    const auto snapshot = audioInputAnalyzer.getSnapshot();
    const auto json = juce::String()
        + "{\n"
        + "  \"sampleRate\": " + juce::String (audioInputAnalyzer.getSampleRate(), 0) + ",\n"
        + "  \"bufferSize\": " + juce::String (audioInputAnalyzer.getBufferSize()) + ",\n"
        + "  \"rms\": " + juce::String (snapshot.rms, 6) + ",\n"
        + "  \"peak\": " + juce::String (snapshot.peak, 6) + ",\n"
        + "  \"loudness\": " + juce::String (snapshot.loudness, 6) + ",\n"
        + "  \"active\": " + juce::String (snapshot.active ? "true" : "false") + ",\n"
        + "  \"sampleCounter\": " + juce::String (static_cast<juce::int64> (snapshot.sampleCounter)) + "\n"
        + "}\n";

    directory.getChildFile ("audio_stats.json").replaceWithText (json, false, false, "\n");
    statusLabel.setText ("audio proof dumped: " + directory.getFullPathName(), juce::dontSendNotification);
}
```

- [ ] **Step 3: Trigger delayed dump**

When `--dump-audio-proof-and-exit` is set, use:

```cpp
juce::Timer::callAfterDelay (2500, [safe = juce::Component::SafePointer<MainComponent> (this)]
{
    if (safe != nullptr)
        safe->dumpAudioProof();
});
```

After dump, quit the app.

- [ ] **Step 4: Verify proof dump**

Run:

```bash
rm -rf debug/a1-audio-proof
perl -e 'alarm 12; exec @ARGV' ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-audio-proof-and-exit
test -s debug/a1-audio-proof/audio_stats.json
sed -n '1,120p' debug/a1-audio-proof/audio_stats.json
```

Expected:
- `audio_stats.json` exists.
- `sampleCounter` is greater than `0` if audio callback ran.
- `active` may be false in a silent room; that is not a failure.
- If sample rate is `0`, mark live device proof blocked and do not claim A1 live proof.

- [ ] **Step 5: Update spec and commit**

Update A1:

```text
- Proven: A1 can dump `debug/a1-audio-proof/audio_stats.json`.
- Proven or blocked: live input status, based on current `audio_stats.json`.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add source/app/Main.cpp source/app/MainComponent.h source/app/MainComponent.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Add audio analyzer proof dump"
```

---

### Task 5: Loudness Uniform Closure

**Files:**
- Modify: `source/core/GraphContract.cpp`
- Modify: `source/render/OpenGLShaderPreview.h`
- Modify: `source/render/OpenGLShaderPreview.cpp`
- Modify: `source/app/MainComponent.cpp`
- Modify: `tests/GraphContractTests.cpp`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Extend graph contract test**

In `tests/GraphContractTests.cpp`, add:

```cpp
expect (myworld::usesSystemUniform (shader, "u_loudness"), "default shader declares u_loudness");
```

Expected RED: test fails because `u_loudness` is not in the shader node yet.

- [ ] **Step 2: Add `u_loudness` to default shader graph and shader**

In `source/core/GraphContract.cpp`, add `u_loudness` to the Shader node uniforms and default shader source:

```glsl
uniform float u_loudness;
```

Use it subtly in the default shader:

```glsl
float pulse = 1.0 + clamp(u_loudness, 0.0, 1.0) * 0.35;
vec3 color = mix(vec3(0.02, 0.025, 0.035), base * pulse, ring);
```

- [ ] **Step 3: Add preview uniform setter**

In `OpenGLShaderPreview.h`, add:

```cpp
void setLoudness (float newLoudness);
std::atomic<float> loudness { 0.0f };
std::unique_ptr<juce::OpenGLShaderProgram::Uniform> loudnessUniform;
```

In `OpenGLShaderPreview.cpp`, bind `u_loudness` after compile and set it during render.

- [ ] **Step 4: Feed analyzer snapshot into preview**

In `MainComponent::updateAudioMeters()`:

```cpp
preview.setLoudness (snapshot.loudness);
```

- [ ] **Step 5: Verify visual/audio closure**

Run:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
rm -rf debug/v1-shader-proof
perl -e 'alarm 12; exec @ARGV' ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
test -s debug/v1-shader-proof/frame.png
test -s debug/v1-shader-proof/node_stats.json
```

Expected:
- Build and tests pass.
- V1 dump still works.
- `node_stats.json` includes `u_loudness` in `systemUniforms`.

- [ ] **Step 6: Update spec and commit**

Update A1:

```text
- Proven: analyzer loudness snapshot is wired to shader uniform `u_loudness`.
```

If live microphone permission is not granted, write:

```text
- Code path exists: `loudness -> u_loudness`, but live acoustic response remains blocked by microphone permission.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add source/core/GraphContract.cpp source/render/OpenGLShaderPreview.h source/render/OpenGLShaderPreview.cpp source/app/MainComponent.cpp tests/GraphContractTests.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Wire loudness to shader uniform"
```

---

## Stop Conditions

- Stop before A1 if A0 is not committed. Audio meters may use JUCE labels during the proof, but graph/node UI must not grow a JUCE `Component` NodeView.
- Stop before adding any node editor UI if it would store graph state inside ImGui ids or JUCE components instead of `NodeSpec` / `NodeInstance` / commands.
- Stop before adding card-heavy node surfaces. Production nodes should stay vvvv-like: name, pins, tiny status, doc/patch icons. Long descriptions belong in human manuals or inspector.
- Stop before putting usage prose directly on node surfaces; use `humanDocPath` and a doc icon instead.
- Stop before claiming live A1 proof if `audio_stats.json` shows `sampleRate: 0` or `sampleCounter: 0`.
- Stop before claiming realtime safety if any callback code allocates, locks, logs, opens files, parses JSON, or calls UI.
- Stop before C1 compound work if A1 has no committed analyzer state and no updated spec.
- Stop before production node editor work; A0 only proves ImGui loop/input viability, not imnodes or a full graph editor.

## Final Verification

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
rm -rf debug/v1-shader-proof debug/a1-audio-proof
perl -e 'alarm 12; exec @ARGV' ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
test -s debug/v1-shader-proof/frame.png
test -s debug/v1-shader-proof/cook_order.json
test -s debug/v1-shader-proof/node_stats.json
perl -e 'alarm 12; exec @ARGV' ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-audio-proof-and-exit
test -s debug/a1-audio-proof/audio_stats.json
```

Expected:
- Build passes.
- All CTest tests pass.
- V1 shader proof still dumps.
- A0 ImGui smoke overlay remains visible in `debug/v1-shader-proof/frame.png`.
- A1 audio proof dumps. If live microphone permission is missing, plan/spec states that live proof is blocked rather than complete.
