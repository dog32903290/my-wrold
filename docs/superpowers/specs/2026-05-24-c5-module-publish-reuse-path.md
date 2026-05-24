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

## C5.2 Target

```text
AIWorkerCommandRequest(publish_module)
-> allowedAIWorkerOperations includes publish_module
-> executeAIWorkerCommand()
-> StorageCommand::publishModule()
-> published package + library reload evidence
-> GraphSession commandLog + collaborationLog proof evidence
-> app proof dump can read back evidence
```

## C5.2 Closed Slice

```text
AIWorkerCommandRequest(publish_module, workManifestPath, source node, module package fields)
-> allowedAIWorkerOperations includes publish_module
-> executeAIWorkerCommand()
-> StorageCommand::publishModule()
-> package/library reload evidence
-> commandLog records ai_worker:publish_module + publish_module:published
-> collaborationLog records publish proof evidence
-> debug/c5-ai-worker-module-publish-proof/ai_worker_module_publish_report.json
```

## C5.2 Evidence

- `source/ai/AIWorkerCommand.*` now carries the minimal `publish_module` payload: source node id, module id/title, published node type, package directory, target library path, and overwrite flag.
- `allowedAIWorkerOperations()` exposes `publish_module`.
- `executeAIWorkerCommand()` handles `publish_module` only by calling `StorageCommand::publishModule()`.
- `AIWorkerCommandEvidence` records `publishCommandLogStatus`, module manifest path, compound patch path, target library path, `packageReloaded`, and `libraryReloaded`.
- `tests/AIWorkerCommandTests.cpp` proves AI `publish_module` writes the package/library, reloads the module manifest, records the shared `publish_module:published` command status, and writes collaboration proof evidence.
- `--dump-c5-ai-worker-module-publish-proof-and-exit` writes `debug/c5-ai-worker-module-publish-proof/ai_worker_module_publish_report.json`.

## C5.2 Proof Report Must Say

```text
ok: true
source: PatchDocument
allowedPublishModule: true
operation: publish_module
status: published
publishCommandLogStatus: publish_module:published
aiCommandLogStatus: ai_worker:publish_module:published
packageReloaded: true
libraryReloaded: true
collaborationProofEvidence includes publishCommandLogStatus=publish_module:published
collaborationProofEvidence includes packageReloaded=true
collaborationProofEvidence includes libraryReloaded=true
usesInteractionState: false
```

Latest C5.2 proof report says:

```text
ok: true
allowedPublishModule: true
operation: publish_module
status: published
publishCommandLogStatus: publish_module:published
aiCommandLogStatus: ai_worker:publish_module:published
packageReloaded: true
libraryReloaded: true
usesInteractionState: false
```

## C5.2 Verification Run

```text
cmake --build build --target my_world_ai_worker_command_tests my-world
./build/my_world_ai_worker_command_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-ai-worker-module-publish-proof-and-exit
git diff --check
ctest --test-dir build --output-on-failure
```

Latest accepted result:

```text
AI worker command contract ok
debug/c5-ai-worker-module-publish-proof/ai_worker_module_publish_report.json ok: true
29/29 tests passed
git diff --check passed
```

## C5.3 Target

```text
visible selected compound node
-> ImGui Publish Module button / OpenGLShaderPreview callback
-> MainComponent publishSelectedModule()
-> StorageCommand::publishModule()
-> package/library reload evidence
-> visible registry creates published node
-> app proof dump can read back evidence
```

## C5.3 Closed Slice

```text
GraphSession with selected visible compound node loud1
-> publishSelectedModuleResult()
-> publishModule()
-> debug/c5-visible-module-publish/modules/loud1/module.json
-> debug/c5-visible-module-publish/modules/loud1/compound.compound.json
-> debug/c5-visible-module-publish/module-libraries/visible.module-library.json
-> loadCompoundModuleNodeSpecsFromLibrary()
-> createNode(visibleRegistry, compound.visible-loud1)
-> debug/c5-visible-module-publish-proof/visible_module_publish_report.json
```

## C5.3 Evidence

- `ImGuiSmokeOverlay` exposes a `Publish Module` control that only fires when a compound node is selected.
- `OpenGLShaderPreview` forwards the visible publish request through `onPublishModuleRequested`.
- `MainComponent::publishSelectedModule()` calls `StorageCommand::publishModule()` and returns command status to the visible overlay.
- Default visible publish proof uses the repo C2 work fixture as the source-law anchor; explicit `MY_WORLD_ACTIVE_WORK_MANIFEST` still routes to the caller's active work manifest.
- `--dump-c5-visible-module-publish-proof-and-exit` writes `debug/c5-visible-module-publish-proof/visible_module_publish_report.json`.

## C5.3 Proof Report Must Say

```text
ok: true
operation: publish_module
source: PatchDocument
sourceNodeId: loud1
sourceNodeType: compound.loudness
publishedModuleId: module.visible-loud1
publishedNodeType: compound.visible-loud1
status: published
packageReloaded: true
libraryReloaded: true
visibleRegistryContainsPublishedNode: true
createdPublishedNode: true
graphCommandLogStatus: create_node
usesInteractionState: false
```

Latest C5.3 proof report says:

```text
ok: true
operation: publish_module
sourceNodeId: loud1
sourceNodeType: compound.loudness
publishedNodeType: compound.visible-loud1
status: published
packageReloaded: true
libraryReloaded: true
visibleRegistryContainsPublishedNode: true
createdPublishedNode: true
graphCommandLogStatus: create_node
usesInteractionState: false
```

## C5.3 Verification Run

```text
cmake --build build --target my-world
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-visible-module-publish-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
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

Final C5 closure is split across:

```text
C5.1 command/storage/runtime publish proof
C5.2 AI worker publish_module command path
C5.3 visible Publish Module hand
```

Together these prove the same selected compound source can be published through the shared storage command path from command tests, AI worker, and visible UI, then reloaded into visible/runtime registries as evidence instead of a separate JSON-writing side path.

## Next Line

```text
C6 candidate:
analyzer compound family / AI repair loop closure
```
