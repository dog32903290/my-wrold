# C5 Module Publish/Reuse Path

Date: 2026-05-24 11:57 Asia/Taipei

## C5 Target

```text
selected reusable compound source
-> publish_module command
-> saved module package manifest + compound patch file
-> module library entry
-> reload through ModuleLibrary into visible NodeSpec registry
-> reload through RuntimeRegistry with coverage diagnostics
-> create published module node through InteractionContract
-> proof dump reads back publish/reuse evidence
```

## C5.1 First Slice

```text
C2 work fixture PatchDocument + selected compound node library_loud1
-> PublishModuleRequest
-> publishModule()
-> modules/published-loudness/module.json
-> modules/published-loudness/compound.compound.json
-> module-libraries/published.module-library.json
-> loadCompoundModuleNodeSpecsFromLibrary()
-> loadRuntimeRegistryFromModuleLibrary()
-> createNode(visibleRegistry, published node type)
-> debug/c5-module-publish-proof/module_publish_report.json
```

This first slice uses an explicit command/test/app proof trigger. Visible UI affordances and AI worker `publish_module` are parked until the command/storage/runtime proof is closed.

## C5.1 Closed Slice

```text
C2 work fixture PatchDocument + selected compound node library_loud1
-> PublishModuleRequest
-> publishModule()
-> modules/published-loudness/module.json
-> modules/published-loudness/compound.compound.json
-> module-libraries/published.module-library.json
-> loadCompoundModuleNodeSpecsFromLibrary()
-> loadRuntimeRegistryFromModuleLibrary()
-> RuntimeOp coverage diagnostics
-> createNode(visibleRegistry, compound.published-loudness)
-> debug/c5-module-publish-proof/module_publish_report.json
```

## C5.1 Evidence

- `source/storage/StorageCommand.*` defines `PublishModuleRequest`, `PublishModuleResult`, and `publishModule()`.
- `publishModule()` validates the active `GraphSession` source node, rejects non-compound sources, resolves the source compound through the work manifest's module libraries, writes a package-local `module.json` and `compound.compound.json`, updates/creates the target `ModuleLibrary`, reloads the package/library/compound, and records `publish_module:published` in `GraphSession.commandLog`.
- The source compound lookup reuses the repo's parent-searching `PathResolution` behavior so publishing follows the same module path law as existing module loading.
- `tests/ModulePublishTests.cpp` proves the published package reloads through `loadCompoundModuleNodeSpecsFromLibrary()`, reloads through `loadRuntimeRegistryFromModuleLibrary()`, passes RuntimeOp diagnostics as `runtime-op-ready` / `create-enabled`, and creates `compound.published-loudness` through `InteractionContract::createNode()`.
- `--dump-c5-module-publish-proof-and-exit` writes `debug/c5-module-publish-proof/module_publish_report.json` plus the published package/library artifacts.

## Contract Check

| Boundary | Trigger | Input | Success | Failure | Log / proof |
| --- | --- | --- | --- | --- | --- |
| Publish command | `publishModule()` from test/app proof | work manifest path, source node id, module id/title/node type, output package directory, target library path, overwrite flag | writes package files, updates/creates target library, returns publish status | missing work/patch, source node missing, source is not compound, invalid public ports, output exists without overwrite, write/reload failure | `publish_module:<status>` command log plus publish report |
| Module package storage | publish command | `ModulePackageManifest` + `CompoundPatchSpec` derived from the selected compound source | manifest-local compound path loads through existing storage APIs | path resolution failure, invalid compound patch JSON, public port mismatch | report includes package path, compound path, public ports, reload status |
| ModuleLibrary reuse | publish command proof | target library manifest path with published package ref | `loadCompoundModuleNodeSpecsFromLibrary()` exposes published `NodeSpec` | malformed library, unresolved package path, duplicate/conflicting node type | report includes visible registry node type and create-enabled status |
| Runtime reuse | publish command proof | same library manifest path | `loadRuntimeRegistryFromModuleLibrary()` returns runtime entry and coverage diagnostics | missing RuntimeOp coverage, invalid child cook order, no public outputs | report includes runtime entry, coverage status, missing ops |
| Interaction reuse | proof command after reload | visible registry + published node type | `createNode()` creates the published compound through the command path | node spec not found, creation gate blocks, graph mutation not logged | report includes `create_node` command log and created node id |

## C5 Proof Report Must Say

```text
ok: true
operation: publish_module
source: PatchDocument
sourceNodeId: library_loud1
sourceNodeType: compound.loudness
publishedModuleId: module.published-loudness
publishedNodeType: compound.published-loudness
packageReloaded: true
libraryReloaded: true
visibleRegistryContainsPublishedNode: true
runtimeRegistryContainsPublishedNode: true
runtimeCoverageStatus: ready
createdPublishedNode: true
graphCommandLogStatus: create_node
usesInteractionState: false
```

Latest proof report says:

```text
ok: true
operation: publish_module
source: PatchDocument
sourceNodeId: library_loud1
sourceNodeType: compound.loudness
publishedModuleId: module.published-loudness
publishedNodeType: compound.published-loudness
packageReloaded: true
libraryReloaded: true
visibleRegistryContainsPublishedNode: true
runtimeRegistryContainsPublishedNode: true
runtimeCoverageStatus: ready
createdPublishedNode: true
graphCommandLogStatus: create_node
usesInteractionState: false
```

## Verification Gate

```text
cmake --build build --target my_world_module_publish_tests my-world
./build/my_world_module_publish_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-module-publish-proof-and-exit
git diff --check
```

Verification run:

```text
cmake --build build --target my_world_module_publish_tests my_world_save_work_command_tests my_world_compound_module_tests my_world_runtime_registry_tests my-world
./build/my_world_module_publish_tests
./build/my_world_save_work_command_tests
./build/my_world_compound_module_tests
./build/my_world_runtime_registry_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-module-publish-proof-and-exit
git diff --check
ctest --test-dir build --output-on-failure
```

Latest accepted result:

```text
module publish/reuse proof ok
save_work command contract ok
compound module fixture ok
runtime registry ok
debug/c5-module-publish-proof/module_publish_report.json ok: true
29/29 tests passed
git diff --check passed
```

## Parked Outside C5.1

```text
publishing arbitrary root work graphs as modules
multi-node selection and group-to-compound extraction
user-facing publish dialog / browser polish
AI worker publish_module command
natural language module authoring
AI repair loop / retry policy
remote sync / shared module registry
new analyzer compound family
human documentation generation beyond a stub path
```

## C5 Closed Target

```text
selected compound source
-> published module package
-> target ModuleLibrary reloads it
-> visible registry can create it
-> runtime registry can execute/diagnose it
-> proof evidence is serializable and read back
```

C5.1 closes this C5 target for one selected compound source. Broader publishing UX and arbitrary graph extraction remain parked outside C5.1.

## Next Line

```text
C6 candidate:
analyzer compound family / AI repair loop closure
```
