# C5 Proof Harness Extraction

Date: 2026-05-25
Status: closed

## Load-Bearing Goal

Move the C5 module publish proof family out of `MainComponent` while preserving the existing CLI flags, output directories, report files, and publish/reuse evidence.

The app shell may keep:

- startup trigger
- status label text
- quit-after-dump scheduling

The runner must own:

- C2 work fixture candidate lookup
- output directory clearing/creation
- publish package and module-library paths
- direct `publish_module` proof
- AI worker `publish_module` proof
- visible module publish/reuse proof
- C5 report writing through the shared `ProofReports` serializer

## Preserved External Contract

CLI:

```text
--dump-c5-module-publish-proof-and-exit
--dump-c5-ai-worker-module-publish-proof-and-exit
--dump-c5-visible-module-publish-proof-and-exit
```

Artifacts:

```text
debug/c5-module-publish-proof/module_publish_report.json
debug/c5-ai-worker-module-publish-proof/ai_worker_module_publish_report.json
debug/c5-visible-module-publish-proof/visible_module_publish_report.json
```

Stable JSON fields:

```text
kind
ok
operation = publish_module
publishedNodeType
packageReloaded
libraryReloaded
createdPublishedNode
error
```

Additional stable fields:

```text
c5ModulePublishProof.runtimeCoverageStatus = ready
c5AIWorkerModulePublishProof.allowedPublishModule = true
c5AIWorkerModulePublishProof.publishCommandLogStatus = publish_module:published
c5AIWorkerModulePublishProof.aiCommandLogStatus = ai_worker:publish_module:published
c5AIWorkerModulePublishProof.collaborationLogEntries = 2
c5VisibleModulePublishProof.visibleRegistryContainsPublishedNode = true
```

## Implementation

- `source/app/C5ModulePublishProofRunner.h`
- `source/app/C5ModulePublishProofRunner.cpp`
- `tests/C5ModulePublishProofRunnerTests.cpp`

`MainComponent::dumpC5ModulePublishProof()`, `dumpC5AIWorkerModulePublishProof()`, and `dumpC5VisibleModulePublishProof()` now build small requests, call `runC5ModulePublishProof()`, and map the result to UI status text.

## Verification

Red:

```text
cmake -S . -B build
```

Expected failure before runner implementation:

```text
Cannot find source file:
  source/app/C5ModulePublishProofRunner.cpp
```

Green:

```text
cmake -S . -B build
cmake --build build --target my_world_c5_module_publish_proof_runner_tests
cmake --build build --target my-world
ctest --test-dir build --output-on-failure -R "c5_module_publish_proof_runner|module_publish|ai_worker_command"
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-module-publish-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-ai-worker-module-publish-proof-and-exit
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c5-visible-module-publish-proof-and-exit
```

Accepted result:

- `module_publish` passed.
- `ai_worker_command` passed.
- `c5_module_publish_proof_runner` passed.
- app target built.
- all three CLI proofs exited 0.
- all three reports retained `ok: true`.

## Parked

- C2-C4 proof orchestration remains in `MainComponent`.
- C6 AI repair-loop proof remains in `MainComponent`.
- V1 shader proof remains tied to `OpenGLShaderPreview`.
