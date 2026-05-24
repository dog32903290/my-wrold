# Post-C Three-Layer Cleanup

Date: 2026-05-24 15:11 Asia/Taipei

## H1 Target

After the C segment is closed and its post-close hygiene commits are committed, the next engineering lane is:

```text
H1 post-C three-layer cleanup
-> UI overlay responsibility split
-> RuntimeRegistry responsibility split
-> StorageContract serialization split
```

This is a cleanup lane, not a new feature lane. It exists because C4-C6 proved the command/storage/runtime surfaces, and the next feature lanes would otherwise keep adding weight to the same three large files.

## Entry Gate

Start H1 only after:

```text
C4-C6 are closed
C post-close hygiene commits are committed
ctest passes
git diff --check passes
working tree has no unrelated staged files
```

The current known exception is the existing `AGENTS.md` dirty line. Do not include it in H1 unless explicitly requested.

H1 should happen before these lanes unless explicitly overridden:

```text
R runtime/render backbone
PV/analyzer detector expansion
new storage schema work
new visible node-browser / inspector feature work
```

## Layer Order

### H1.1 UI Overlay Split

Repair timing:

```text
first H1 slice, immediately after C hygiene
```

Why now:

```text
source/ui/ImGuiSmokeOverlay.cpp is the current visible interaction surface.
It mixes drawing, browser, inspector, selection/drag affordances, and command callbacks.
Future visible publish/browser/analyzer work would make this harder to split safely.
```

Allowed move:

```text
split drawing/browser/inspector helpers out of ImGuiSmokeOverlay.cpp
keep the existing public overlay API stable
keep command callbacks as callbacks into the same command path
no new UI behavior
no new graph mutation path
```

Verification:

```text
cmake --build build --target my-world
ctest --test-dir build --output-on-failure
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-visible-module-publish-proof-and-exit
git diff --check
```

### H1.2 RuntimeRegistry Split

Repair timing:

```text
second H1 slice, before R runtime/render backbone or PV/analyzer detector expansion
```

Why now:

```text
source/core/RuntimeRegistry.cpp owns loading, runtime-op catalog, coverage diagnostics,
synthetic execution, bridge snapshots, and debug JSON.
R/PV lanes will add runtime pressure, so this split must happen before adding more behavior.
```

Allowed move:

```text
split pure responsibility groups into thin files such as runtime loading,
runtime-op coverage/diagnostics, synthetic execution, and runtime JSON reports
keep RuntimeRegistry value structs and public function signatures stable unless a test requires otherwise
no registry schema change
no new runtime op semantics
```

Verification:

```text
cmake --build build --target my_world_runtime_registry_tests my_world_analyzer_compound_family_tests my-world
./build/my_world_runtime_registry_tests
./build/my_world_analyzer_compound_family_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-analyzer-family-proof-and-exit
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c6-ai-repair-loop-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```

### H1.3 StorageContract Split

Repair timing:

```text
third H1 slice, before new storage schema work or broader module/work library persistence changes
```

Why now:

```text
source/storage/StorageContract.cpp owns manifest structs, patch document serialization,
module library/package serialization, parsing, and file loading.
It should not keep growing before the next schema-bearing task.
```

Allowed move:

```text
split serialization/parsing helpers into storage-focused files
keep JSON field names and parse behavior stable
add focused tests only when a split needs a golden or roundtrip guard
no schema migration
no save_work behavior change
```

Verification:

```text
cmake --build build --target my_world_storage_tests my_world_patch_document_tests my_world_save_work_command_tests my_world_module_publish_tests my-world
./build/my_world_storage_tests
./build/my_world_patch_document_tests
./build/my_world_save_work_command_tests
./build/my_world_module_publish_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c2-storage-proof-and-exit
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c3-save-work-proof-and-exit
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-module-publish-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```

## Non-Goals

```text
do not reopen C4/C5/C6 behavior
do not add analyzer detector semantics
do not add RenderBackend/Metal
do not redesign graph schema
do not change storage JSON schema
do not add a new command path
do not hide behavior behind a broad abstraction
```

## Closure Evidence

H1 is closed only when:

```text
all three slices are committed separately
master progress lists H1 as closed
all slice-specific gates pass
ctest passes after the final slice
all affected proof reports still say ok: true
```

## Next Handoff Sentence

Open the master progress plan first. If C post-close hygiene is committed and no newer lane was explicitly selected, the next lane is H1 post-C three-layer cleanup, starting with H1.1 UI overlay split.
