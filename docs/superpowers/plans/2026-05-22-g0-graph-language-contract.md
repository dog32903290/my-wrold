# G0 Graph Language Contract Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add the first graph-language contract for nodes, regions, typed ports, stream kinds, and AI-safe commands before building production node UI or audio graphs.

**Architecture:** Treat the patch as a typed IR / AST that can later compile to runtime graphs, GLSL, C++, validation reports, or migration output. Keep the first implementation pure C++ and testable; park future C# compiler workers behind file/process boundaries so they never enter realtime audio/render paths.

**Tech Stack:** C++20, existing graph contract, deterministic JSON strings for proof, future external workers via files/processes.

---

## Contract

### First-Class Graph Objects

```text
Node
  Ordinary operation or value source.

Region
  Visual control-flow block such as if / for_each / repeat / while.

Edge
  Typed connection with dataType and streamKind.

Command
  Validated mutation request used by UI, AI worker, imports, and scripts.

GraphIR
  Checked graph form that can become runtimeGraph or compiler worker input.
```

### First Type Vocabulary

```text
audio.mono
signal.float
texture.rgba
geometry.mesh
event.midi
event.osc
command.graph
resource.file
generic<T>
```

### First Stream Kinds

```text
continuous
event
command
resource
```

### First Commands

```text
create_node
create_region
connect
set_param
publish_module
save_work
```

### C# Boundary

Allowed later:

```text
C# external compiler worker
  reads graphIR.json
  writes validation_report.json / generated artifacts
```

Forbidden in first stage:

```text
C# inside audio callback
C# inside render loop
C# embedded in native app lifecycle
C# nodes inside runtimeGraph hot path
```

---

## File Map

- Create: `source/core/GraphLanguage.h`
  - Region, type, stream, edge, command, and graph IR structs.

- Create: `source/core/GraphLanguage.cpp`
  - Seed type/stream/command registries and minimal validation helpers.

- Create: `tests/GraphLanguageTests.cpp`
  - Tests region types, typed edges, stream kinds, command vocabulary, and C# boundary labels.

- Modify: `CMakeLists.txt`
  - Add `my_world_graph_language` and `my_world_graph_language_tests`.

- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`
  - Mark G0 status accurately after each task.

- Modify: `docs/superpowers/plans/2026-05-22-a1-audio-proof.md`
  - Keep S0/G0 as gates before A0/A1 execution.

---

### Task 1: Graph Language Types

**Files:**
- Create: `source/core/GraphLanguage.h`
- Create: `source/core/GraphLanguage.cpp`
- Create: `tests/GraphLanguageTests.cpp`
- Modify: `CMakeLists.txt`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Write failing graph language tests**

Add `tests/GraphLanguageTests.cpp`:

```cpp
#include "GraphLanguage.h"

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
    expect (myworld::isKnownRegionType ("if"), "if region");
    expect (myworld::isKnownRegionType ("for_each"), "for_each region");
    expect (! myworld::isKnownRegionType ("maze"), "unknown region");

    expect (myworld::isKnownTypeSpec ("audio.mono"), "audio.mono type");
    expect (myworld::isKnownTypeSpec ("signal.float"), "signal.float type");
    expect (myworld::isKnownTypeSpec ("generic<T>"), "generic type slot");
    expect (! myworld::isKnownTypeSpec ("mystery.blob"), "unknown type");

    expect (myworld::isKnownStreamKind ("continuous"), "continuous stream");
    expect (myworld::isKnownStreamKind ("event"), "event stream");
    expect (myworld::isKnownStreamKind ("command"), "command stream");
    expect (myworld::isKnownStreamKind ("resource"), "resource stream");

    const myworld::TypedEdge edge { "edge1", "audio1.mono", "loudness1.input", "audio.mono", "continuous" };
    expect (myworld::isValidTypedEdge (edge), "typed edge validates");

    const myworld::TypedEdge badEdge { "edge2", "midi1.note", "shader1.input", "event.midi", "continuous" };
    expect (! myworld::isValidTypedEdge (badEdge), "event data cannot use continuous stream kind");

    expect (myworld::isKnownCommandType ("create_node"), "create_node command");
    expect (myworld::isKnownCommandType ("create_region"), "create_region command");
    expect (myworld::isKnownCommandType ("save_work"), "save_work command");
    expect (! myworld::isKnownCommandType ("edit_json_directly"), "direct json mutation forbidden");

    expect (myworld::isAllowedCompilerWorkerLanguage ("c++"), "c++ compiler worker language");
    expect (myworld::isAllowedCompilerWorkerLanguage ("c#"), "c# external compiler worker language");
    expect (! myworld::isAllowedRealtimeRuntimeLanguage ("c#"), "c# not allowed in realtime runtime");

    std::cout << "graph language contract ok\n";
    return 0;
}
```

- [ ] **Step 2: Wire test target and verify RED**

Modify `CMakeLists.txt`:

```cmake
add_library(my_world_graph_language
    source/core/GraphLanguage.cpp
)

target_include_directories(my_world_graph_language
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

add_executable(my_world_graph_language_tests
    tests/GraphLanguageTests.cpp
)

target_link_libraries(my_world_graph_language_tests
    PRIVATE
        my_world_graph_language
)

add_test(NAME graph_language COMMAND my_world_graph_language_tests)
```

Run:

```bash
cmake --build build --target my_world_graph_language_tests
```

Expected: FAIL because `GraphLanguage.h` does not exist yet.

- [ ] **Step 3: Implement minimal graph language contract**

Create `source/core/GraphLanguage.h`:

```cpp
#pragma once

#include <string>

namespace myworld
{
struct TypedEdge
{
    std::string id;
    std::string from;
    std::string to;
    std::string dataType;
    std::string streamKind;
};

bool isKnownRegionType (const std::string& type);
bool isKnownTypeSpec (const std::string& type);
bool isKnownStreamKind (const std::string& kind);
bool isKnownCommandType (const std::string& type);
bool isValidTypedEdge (const TypedEdge& edge);
bool isAllowedCompilerWorkerLanguage (const std::string& language);
bool isAllowedRealtimeRuntimeLanguage (const std::string& language);
}
```

Create `source/core/GraphLanguage.cpp`:

```cpp
#include "GraphLanguage.h"

#include <algorithm>
#include <array>

namespace myworld
{
namespace
{
template <size_t size>
bool contains (const std::array<const char*, size>& values, const std::string& value)
{
    return std::find (values.begin(), values.end(), value) != values.end();
}
}

bool isKnownRegionType (const std::string& type)
{
    static constexpr std::array<const char*, 4> values { "if", "for_each", "repeat", "while" };
    return contains (values, type);
}

bool isKnownTypeSpec (const std::string& type)
{
    static constexpr std::array<const char*, 9> values {
        "audio.mono",
        "signal.float",
        "texture.rgba",
        "geometry.mesh",
        "event.midi",
        "event.osc",
        "command.graph",
        "resource.file",
        "generic<T>"
    };
    return contains (values, type);
}

bool isKnownStreamKind (const std::string& kind)
{
    static constexpr std::array<const char*, 4> values { "continuous", "event", "command", "resource" };
    return contains (values, kind);
}

bool isKnownCommandType (const std::string& type)
{
    static constexpr std::array<const char*, 6> values {
        "create_node",
        "create_region",
        "connect",
        "set_param",
        "publish_module",
        "save_work"
    };
    return contains (values, type);
}

bool isValidTypedEdge (const TypedEdge& edge)
{
    if (! isKnownTypeSpec (edge.dataType) || ! isKnownStreamKind (edge.streamKind))
        return false;

    if (edge.dataType.rfind ("event.", 0) == 0)
        return edge.streamKind == "event";

    if (edge.dataType.rfind ("command.", 0) == 0)
        return edge.streamKind == "command";

    if (edge.dataType.rfind ("resource.", 0) == 0)
        return edge.streamKind == "resource";

    return edge.streamKind == "continuous";
}

bool isAllowedCompilerWorkerLanguage (const std::string& language)
{
    static constexpr std::array<const char*, 2> values { "c++", "c#" };
    return contains (values, language);
}

bool isAllowedRealtimeRuntimeLanguage (const std::string& language)
{
    static constexpr std::array<const char*, 1> values { "c++" };
    return contains (values, language);
}
}
```

- [ ] **Step 4: Verify GREEN**

Run:

```bash
cmake --build build --target my_world_graph_language_tests
ctest --test-dir build --output-on-failure
```

Expected: graph language and existing tests pass.

- [ ] **Step 5: Update spec and commit**

Update `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`:

```text
- Proven: G0 first graph language contract has region types, TypeSpec, StreamKind, typed edges, AI-safe command names, and C# external compiler worker boundary.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add CMakeLists.txt source/core/GraphLanguage.h source/core/GraphLanguage.cpp tests/GraphLanguageTests.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Add graph language contract"
```

---

### Task 2: Graph IR / Compiler Worker Manifest

**Files:**
- Create: `fixtures/graph-language/minimal-graph-ir.json`
- Create: `fixtures/graph-language/compiler-worker.manifest.json`
- Modify: `tests/GraphLanguageTests.cpp`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Add graph IR fixture**

Create `fixtures/graph-language/minimal-graph-ir.json`:

```json
{
  "kind": "graphIR",
  "version": 1,
  "nodes": [
    { "id": "audio1", "type": "audio.input" },
    { "id": "loudness1", "type": "analyzer.loudness" }
  ],
  "regions": [
    {
      "id": "region1",
      "type": "if",
      "bodyPatchId": "patch.if-body",
      "inputBorderPorts": ["condition"],
      "outputBorderPorts": ["result"]
    }
  ],
  "edges": [
    {
      "id": "edge1",
      "from": "audio1.mono",
      "to": "loudness1.input",
      "dataType": "audio.mono",
      "streamKind": "continuous"
    }
  ]
}
```

- [ ] **Step 2: Add compiler worker manifest fixture**

Create `fixtures/graph-language/compiler-worker.manifest.json`:

```json
{
  "kind": "externalCompilerWorker",
  "version": 1,
  "allowedLanguages": ["c++", "c#"],
  "forbiddenRuntimeTargets": ["audioCallback", "renderLoop", "appLifecycle"],
  "inputs": ["graphIR.json", "commandGraph.json"],
  "outputs": ["validation_report.json", "generated_artifacts"]
}
```

- [ ] **Step 3: Extend test to require fixtures**

Add to `tests/GraphLanguageTests.cpp`:

```cpp
#include <filesystem>
```

Add in `main()`:

```cpp
expect (std::filesystem::exists ("fixtures/graph-language/minimal-graph-ir.json"), "minimal graph IR fixture");
expect (std::filesystem::exists ("fixtures/graph-language/compiler-worker.manifest.json"), "compiler worker manifest fixture");
```

- [ ] **Step 4: Verify and commit**

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add fixtures/graph-language/minimal-graph-ir.json fixtures/graph-language/compiler-worker.manifest.json tests/GraphLanguageTests.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Add graph IR compiler worker fixtures"
```

---

## Stop Conditions

- Stop before A0/A1 if G0 has no committed graph language contract or is not explicitly deferred.
- Stop before region UI if `RegionSpec` cannot name type, body patch, input border ports, and output border ports.
- Stop before AI worker graph edits if commands can bypass validation or mutate JSON directly.
- Stop before using C# if it would run in the realtime audio callback, render loop, or native app lifecycle.
- Stop before type inference work if the first `TypeSpec` and `StreamKind` registries are not committed.

## Final Verification

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
```

Expected:
- Graph language tests pass.
- Storage and V1 tests still pass.
- Spec states whether G0 is proven, blocked, or deferred before A0/A1 begins.
