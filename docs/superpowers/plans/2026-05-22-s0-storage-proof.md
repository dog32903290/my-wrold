# S0 Storage Proof Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Prove that a work can be saved as reloadable patch documents, published into module/work libraries, and committed locally by `Command+S`.

**Architecture:** Storage is the graph source-of-truth boundary, not a late export feature. A work project owns patch documents and proof outputs; module packages are reusable saved mother patches or compounds; library indexes list available works and modules. `Command+S` writes an atomic project snapshot, validates it can be read back, then makes a local git commit in the artwork project repository only.

**Tech Stack:** C++20, standard filesystem, JUCE app shell for keyboard command later, git CLI for local commit proof, existing graph contract.

---

## Storage Contract

### Concepts

```text
WorkProject
  A full artwork project. Owns one main patch, optional subpatches, assets, debug/proof outputs, and local git history.

PatchDocument
  A serializable graph document. Can be a whole work patch, a mother patch, or a compound patcher body. Stores graph structure and parameter binding state.

ModulePackage
  A reusable patch/compound published from a PatchDocument into a module repository. Exposes public ports and docs.

WorkLibrary
  A repository/index of larger works.

ModuleLibrary
  A repository/index of reusable module packages.
```

### First Directory Shape

```text
<work-root>/
  myworld.work.json
  patches/
    main.patch.json
  assets/
  debug/
  .myworld/
    save_log.jsonl
  .git/

<library-root>/
  works/
    <work-id>/
      myworld.work.json
      patches/
        main.patch.json
  modules/
    <module-id>/
      module.myworld.json
      patch.patch.json
      docs/
        manual.md
```

### `Command+S` Contract

```text
trigger:
  user presses Command+S in the app

input:
  active WorkProject root
  current editorGraph / runtimeGraph / commandGraph snapshot
  dirty flag

success:
  write files atomically
  reload/validate saved files
  git add only project files
  git commit if there are changes
  append save_log.jsonl with commit id

failure:
  validation failure -> do not replace existing project files
  save ok but git commit failed -> keep saved files, report save-ok commit-failed
  no changes -> no commit, report clean

observability:
  save_log.jsonl records timestamp, changed files, validation result, commit id or failure reason
```

`Command+S` must never commit the app source repository unless the app source repository is explicitly opened as the active work project. Auto commit is local-only; pushing to any remote is a separate command.

---

## File Map

- Create: `source/storage/StorageContract.h`
  - Manifest structs and path constants for work projects, patch documents, module packages, and library indexes.

- Create: `source/storage/StorageContract.cpp`
  - Helpers that create first proof manifests and serialize deterministic JSON strings.

- Create: `tests/StorageContractTests.cpp`
  - Tests first manifest shape, module package shape, and `Command+S` save status vocabulary.

- Create: `fixtures/storage/minimal-work/myworld.work.json`
  - First reloadable work manifest fixture.

- Create: `fixtures/storage/minimal-work/patches/main.patch.json`
  - First reloadable patch document fixture.

- Modify: `CMakeLists.txt`
  - Add `my_world_storage` and `my_world_storage_tests`.

- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`
  - Mark storage state as proven/blocked after each task.

---

### Task 1: Storage Schema Contract

**Files:**
- Create: `source/storage/StorageContract.h`
- Create: `source/storage/StorageContract.cpp`
- Create: `tests/StorageContractTests.cpp`
- Modify: `CMakeLists.txt`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Write failing storage contract tests**

Add `tests/StorageContractTests.cpp`:

```cpp
#include "StorageContract.h"

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

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}
}

int main()
{
    const auto work = myworld::makeMinimalWorkProject ("work.city-0001", "City Study");
    expect (work.id == "work.city-0001", "work id");
    expect (work.mainPatchPath == "patches/main.patch.json", "main patch path");
    expect (work.savePolicy == "commandS.localCommit", "save policy");

    const auto workJson = myworld::toJson (work);
    expectContains (workJson, "\"kind\": \"workProject\"", "work json");
    expectContains (workJson, "\"mainPatchPath\": \"patches/main.patch.json\"", "work json");
    expectContains (workJson, "\"moduleLibraries\"", "work json");

    const auto patch = myworld::makeMinimalPatchDocument ("patch.main", "Main Patch");
    const auto patchJson = myworld::toJson (patch);
    expectContains (patchJson, "\"kind\": \"patchDocument\"", "patch json");
    expectContains (patchJson, "\"editorGraph\"", "patch json");
    expectContains (patchJson, "\"runtimeGraph\"", "patch json");
    expectContains (patchJson, "\"portBindings\"", "patch json");

    const auto module = myworld::makeModulePackage ("module.loudness", "Loudness Module", "patches/loudness.patch.json");
    const auto moduleJson = myworld::toJson (module);
    expectContains (moduleJson, "\"kind\": \"modulePackage\"", "module json");
    expectContains (moduleJson, "\"publicPorts\"", "module json");
    expectContains (moduleJson, "\"humanDocPath\"", "module json");

    expect (myworld::isKnownSaveStatus ("saved-and-committed"), "saved-and-committed status");
    expect (myworld::isKnownSaveStatus ("save-ok commit-failed"), "commit-failed status");
    expect (! myworld::isKnownSaveStatus ("probably-saved"), "unknown status rejected");

    std::cout << "storage contract ok\n";
    return 0;
}
```

- [ ] **Step 2: Wire test target and verify RED**

Modify `CMakeLists.txt`:

```cmake
add_library(my_world_storage
    source/storage/StorageContract.cpp
)

target_include_directories(my_world_storage
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

add_executable(my_world_storage_tests
    tests/StorageContractTests.cpp
)

target_link_libraries(my_world_storage_tests
    PRIVATE
        my_world_storage
)

add_test(NAME storage_contract COMMAND my_world_storage_tests)
```

Run:

```bash
cmake --build build --target my_world_storage_tests
```

Expected: FAIL because `StorageContract.h` does not exist yet.

- [ ] **Step 3: Implement minimal storage contract**

Create `source/storage/StorageContract.h`:

```cpp
#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct WorkProjectManifest
{
    std::string id;
    std::string title;
    std::string mainPatchPath;
    std::string savePolicy;
    std::vector<std::string> moduleLibraries;
    std::vector<std::string> workLibraries;
};

struct PatchDocumentManifest
{
    std::string id;
    std::string title;
    std::string editorGraphKind;
    std::string runtimeGraphKind;
    std::string portBindingsKind;
};

struct ModulePackageManifest
{
    std::string id;
    std::string title;
    std::string patchPath;
    std::string humanDocPath;
    std::vector<std::string> publicPorts;
};

WorkProjectManifest makeMinimalWorkProject (const std::string& id, const std::string& title);
PatchDocumentManifest makeMinimalPatchDocument (const std::string& id, const std::string& title);
ModulePackageManifest makeModulePackage (const std::string& id, const std::string& title, const std::string& patchPath);

std::string toJson (const WorkProjectManifest& manifest);
std::string toJson (const PatchDocumentManifest& manifest);
std::string toJson (const ModulePackageManifest& manifest);
bool isKnownSaveStatus (const std::string& status);
}
```

Create `source/storage/StorageContract.cpp`:

```cpp
#include "StorageContract.h"

#include <algorithm>
#include <array>
#include <sstream>

namespace myworld
{
namespace
{
std::string quote (const std::string& text)
{
    std::ostringstream out;
    out << "\"";

    for (const auto character : text)
    {
        if (character == '"' || character == '\\')
            out << '\\';

        out << character;
    }

    out << "\"";
    return out.str();
}

void appendStringArray (std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";

    for (size_t index = 0; index < values.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << quote (values[index]);
    }

    out << "]";
}
}

WorkProjectManifest makeMinimalWorkProject (const std::string& id, const std::string& title)
{
    return {
        id,
        title,
        "patches/main.patch.json",
        "commandS.localCommit",
        { "modules" },
        { "works" }
    };
}

PatchDocumentManifest makeMinimalPatchDocument (const std::string& id, const std::string& title)
{
    return { id, title, "editorGraph", "runtimeGraph", "portBindings" };
}

ModulePackageManifest makeModulePackage (const std::string& id, const std::string& title, const std::string& patchPath)
{
    return {
        id,
        title,
        patchPath,
        "docs/manual.md",
        { "in", "out" }
    };
}

std::string toJson (const WorkProjectManifest& manifest)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"workProject\",\n";
    out << "  \"id\": " << quote (manifest.id) << ",\n";
    out << "  \"title\": " << quote (manifest.title) << ",\n";
    out << "  \"mainPatchPath\": " << quote (manifest.mainPatchPath) << ",\n";
    out << "  \"savePolicy\": " << quote (manifest.savePolicy) << ",\n";
    out << "  \"moduleLibraries\": ";
    appendStringArray (out, manifest.moduleLibraries);
    out << ",\n";
    out << "  \"workLibraries\": ";
    appendStringArray (out, manifest.workLibraries);
    out << "\n";
    out << "}\n";
    return out.str();
}

std::string toJson (const PatchDocumentManifest& manifest)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"patchDocument\",\n";
    out << "  \"id\": " << quote (manifest.id) << ",\n";
    out << "  \"title\": " << quote (manifest.title) << ",\n";
    out << "  \"editorGraph\": { \"kind\": " << quote (manifest.editorGraphKind) << " },\n";
    out << "  \"runtimeGraph\": { \"kind\": " << quote (manifest.runtimeGraphKind) << " },\n";
    out << "  \"portBindings\": { \"kind\": " << quote (manifest.portBindingsKind) << " }\n";
    out << "}\n";
    return out.str();
}

std::string toJson (const ModulePackageManifest& manifest)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"modulePackage\",\n";
    out << "  \"id\": " << quote (manifest.id) << ",\n";
    out << "  \"title\": " << quote (manifest.title) << ",\n";
    out << "  \"patchPath\": " << quote (manifest.patchPath) << ",\n";
    out << "  \"humanDocPath\": " << quote (manifest.humanDocPath) << ",\n";
    out << "  \"publicPorts\": ";
    appendStringArray (out, manifest.publicPorts);
    out << "\n";
    out << "}\n";
    return out.str();
}

bool isKnownSaveStatus (const std::string& status)
{
    static constexpr std::array<const char*, 5> statuses {
        "clean",
        "saved-and-committed",
        "save-ok commit-failed",
        "validation-failed",
        "write-failed"
    };

    return std::find (statuses.begin(), statuses.end(), status) != statuses.end();
}
}
```

- [ ] **Step 4: Verify GREEN**

Run:

```bash
cmake --build build --target my_world_storage_tests
ctest --test-dir build --output-on-failure
```

Expected: storage contract and existing tests pass.

- [ ] **Step 5: Update spec and commit**

Update `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`:

```text
- Proven: minimal storage contract names WorkProject, PatchDocument, ModulePackage, WorkLibrary, and ModuleLibrary.
- Proven: Command+S save statuses are explicit: clean, saved-and-committed, save-ok commit-failed, validation-failed, write-failed.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add CMakeLists.txt source/storage/StorageContract.h source/storage/StorageContract.cpp tests/StorageContractTests.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Add storage contract proof"
```

---

### Task 2: Atomic Work Save Fixture

**Files:**
- Create: `fixtures/storage/minimal-work/myworld.work.json`
- Create: `fixtures/storage/minimal-work/patches/main.patch.json`
- Modify: `tests/StorageContractTests.cpp`
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`

- [ ] **Step 1: Add fixture files**

Create `fixtures/storage/minimal-work/myworld.work.json`:

```json
{
  "kind": "workProject",
  "id": "work.minimal",
  "title": "Minimal Work",
  "mainPatchPath": "patches/main.patch.json",
  "savePolicy": "commandS.localCommit",
  "moduleLibraries": ["modules"],
  "workLibraries": ["works"]
}
```

Create `fixtures/storage/minimal-work/patches/main.patch.json`:

```json
{
  "kind": "patchDocument",
  "id": "patch.main",
  "title": "Main Patch",
  "editorGraph": {
    "nodes": [
      { "id": "shader1", "type": "shader.fragment" },
      { "id": "out1", "type": "output.preview" }
    ]
  },
  "runtimeGraph": {
    "nodes": [
      { "id": "shader1", "type": "shader.fragment" },
      { "id": "out1", "type": "output.preview" }
    ],
    "edges": [
      { "from": "shader1.output", "to": "out1.input" }
    ]
  },
  "portBindings": {
    "items": [
      {
        "id": "shader1.brightness",
        "dataType": "signal.float",
        "bindingMode": "manual",
        "storedValue": 1.0
      }
    ]
  }
}
```

- [ ] **Step 2: Extend storage test to check fixtures exist**

Add to `tests/StorageContractTests.cpp`:

```cpp
#include <filesystem>
```

Add in `main()`:

```cpp
expect (std::filesystem::exists ("fixtures/storage/minimal-work/myworld.work.json"), "minimal work fixture");
expect (std::filesystem::exists ("fixtures/storage/minimal-work/patches/main.patch.json"), "minimal patch fixture");
```

- [ ] **Step 3: Verify**

Run:

```bash
cmake --build build --target my_world_storage_tests
ctest --test-dir build --output-on-failure
```

Expected: storage test passes and fixture files are tracked.

- [ ] **Step 4: Update spec and commit**

Update S0:

```text
- Proven: minimal work fixture can store a main patch with shader1 -> out1 as reloadable project data.
```

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
git add fixtures/storage/minimal-work/myworld.work.json fixtures/storage/minimal-work/patches/main.patch.json tests/StorageContractTests.cpp docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md
git commit -m "Add minimal work storage fixture"
```

---

### Task 3: Command+S Local Commit Contract

**Files:**
- Modify: `docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md`
- Modify: `docs/superpowers/plans/2026-05-22-s0-storage-proof.md`

- [ ] **Step 1: Lock the first behavior**

For first implementation, `Command+S` behavior is:

```text
if no active work project:
  report validation-failed: no active work project

if active work project is dirty:
  write atomic files
  reload/validate
  git add project files only
  git commit -m "Save <work title>"

if active work project has no changes:
  report clean

if write succeeds and git commit fails:
  report save-ok commit-failed
```

- [ ] **Step 2: Update spec**

Add this to S0:

```text
- Locked: `Command+S` creates at most one local commit per explicit save gesture.
- Locked: no remote push happens on save.
- Locked: no commit happens when there are no file changes.
- Locked: save log records both save result and commit result.
```

- [ ] **Step 3: Commit the clarified contract**

Run:

```bash
git diff --check
git add docs/superpowers/specs/2026-05-22-native-canvas-skeleton-design.md docs/superpowers/plans/2026-05-22-s0-storage-proof.md
git commit -m "Clarify command save commit contract"
```

---

## Stop Conditions

- Stop before A0/A1 if S0 has no committed storage contract or is not explicitly deferred.
- Stop before implementing `Command+S` if it is unclear which repository is the active work project.
- Stop if save/commit code could stage files outside the active work root.
- Stop if `Command+S` would push to remote or require credentials.
- Stop if a module package cannot name its public ports and source patch.

## Final Verification

Run:

```bash
git diff --check
cmake --build build
ctest --test-dir build --output-on-failure
```

Expected:
- Storage contract tests pass.
- Existing graph/V1 tests still pass.
- The plan/spec explicitly state whether S0 is proven, blocked, or deferred before A0/A1 begins.
