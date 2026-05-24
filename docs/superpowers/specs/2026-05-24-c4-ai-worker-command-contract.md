# C4 AI Worker Command Contract

Date: 2026-05-24 11:36 Asia/Taipei

## C4 Target

```text
AI worker command
-> formal commandGraph / allowed operation
-> mutates or saves through same command path as UI
-> save_work caller uses StorageCommand::saveWork()
-> command/collaboration log records intent, result, proof evidence
-> app proof dump can read back evidence
```

## C4.1 Closed Slice

```text
AIWorkerCommandRequest(save_work, workManifestPath, dirty GraphSession)
-> allowedAIWorkerOperations includes save_work
-> executeAIWorkerCommand()
-> StorageCommand::saveWork()
-> PatchDocument reload + save log readback
-> GraphSession commandLog + collaborationLog evidence
-> debug/c4-ai-worker-save-work-proof/ai_worker_save_work_report.json
```

## C4.2 Closed Slice

```text
AIWorkerCommandRequest(move_node, nodeId, deltaX, deltaY)
-> allowedAIWorkerOperations includes move_node
-> executeAIWorkerCommand()
-> InteractionContract::moveNode()
-> dirty GraphSession + move_node command log
-> GraphSession collaborationLog evidence
```

## C4.3 Closed Slice

```text
AIWorkerCommandRequest(move_node)
-> InteractionContract::moveNode()
-> AIWorkerCommandRequest(save_work)
-> StorageCommand::saveWork()
-> PatchDocument reload preserves moved node
-> app proof reads command log, save log, collaboration log, and reload evidence
```

## C4.1 Evidence

- `source/ai/AIWorkerCommand.*` defines the minimal AI worker request/result shape.
- `allowedAIWorkerOperations()` exposes `save_work` as the first formal allowed operation.
- `executeAIWorkerCommand()` records AI intent, calls `StorageCommand::saveWork()` for the actual save, then records result/proof evidence.
- The AI path does not call `savePatchDocument()` directly and does not call `serializeInteractionState()`.
- `GraphSession::collaborationLog` records actor, command id, operation, intent, status, result, proof evidence, and error.
- `tests/AIWorkerCommandTests.cpp` proves dirty C2 compound work saves through the shared command path, reloads as `PatchDocument`, preserves public compound ports and expanded child layout, and reads back `.myworld/save_log.jsonl`.
- `--dump-c4-ai-worker-save-work-proof-and-exit` writes `debug/c4-ai-worker-save-work-proof/ai_worker_save_work_report.json`.

## C4.2 Evidence

- `AIWorkerCommandRequest` now carries the minimal mutation payload: `nodeId`, `deltaX`, and `deltaY`.
- `AIWorkerCommandEvidence` now records graph-command evidence: `graphCommandLogStatus` and `graphMutationApplied`.
- `allowedAIWorkerOperations()` exposes `move_node` as the first AI graph mutation operation.
- `executeAIWorkerCommand()` handles `move_node` only through `InteractionContract::moveNode()`.
- `tests/AIWorkerCommandTests.cpp` proves AI `move_node` changes `library_loud1` position, leaves the session dirty, records `move_node` in `commandLog`, and records collaboration proof evidence.

## C4.3 Evidence

- `tests/AIWorkerCommandTests.cpp` now runs `move_node -> save_work` on the same AI-mutated session and proves the reloaded `PatchDocument` preserves the moved `library_loud1` position.
- `--dump-c4-ai-worker-save-work-proof-and-exit` now runs the same two-command AI sequence instead of mutating the graph directly before save.
- `debug/c4-ai-worker-save-work-proof/ai_worker_save_work_report.json` records `allowedMoveNode`, `graphCommandLogStatus`, `graphMutationApplied`, `moveCollaborationProofEvidence`, `storageCommandLogStatus`, `saveLogStatus`, `savedMove.matches`, public-port edges, and expanded child layout.

## C4.1 Proof Report Must Say

```text
ok: true
source: PatchDocument
usesInteractionState: false
allowedSaveWork: true
operation: save_work
status: save-ok commit-pending
storageCommandLogStatus: save_work:save-ok commit-pending
aiCommandLogStatus: ai_worker:save_work:save-ok commit-pending
patchReloaded: true
saveLogOk: true
saveLogStatus: save-ok commit-pending
collaborationLogEntries: 2
collaborationIntentStatus: requested
collaborationResultStatus: save-ok commit-pending
collaborationProofEvidence includes patchReloaded=true and saveLogStatus=save-ok commit-pending
publicInputEdge: true
publicOutputEdge: true
expandedLayout.matches: true
```

## C4.3 Proof Report Must Say

```text
ok: true
source: PatchDocument
usesInteractionState: false
allowedSaveWork: true
allowedMoveNode: true
moveOperation: move_node
moveStatus: ok
graphCommandLogStatus: move_node
graphMutationApplied: true
operation: save_work
status: save-ok commit-pending
storageCommandLogStatus: save_work:save-ok commit-pending
aiCommandLogStatus: ai_worker:save_work:save-ok commit-pending
patchReloaded: true
saveLogOk: true
saveLogStatus: save-ok commit-pending
collaborationLogEntries: 4
moveCollaborationProofEvidence includes graphCommandLogStatus=move_node
collaborationProofEvidence includes patchReloaded=true and saveLogStatus=save-ok commit-pending
savedMove.matches: true
publicInputEdge: true
publicOutputEdge: true
expandedLayout.matches: true
```

## Verification Gate

```text
cmake --build build --target my-world my_world_ai_worker_command_tests
./build/my_world_ai_worker_command_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c4-ai-worker-save-work-proof-and-exit
```

## C4 Closed Target

```text
AI worker command
-> formal allowed operation boundary
-> move_node mutates through InteractionContract
-> save_work saves through StorageCommand::saveWork()
-> no direct JSON surgery
-> no interaction-state-v1 save path
-> command/collaboration log records intent, result, proof evidence
-> app proof dump reads back evidence
```

## Parked Outside C4

```text
natural language task parsing
AI graph mutation commands beyond move_node
repair loop / retry policy
remote push/sync
persisted collaboration_log.jsonl file
user-facing AI worker UI
```

## Next Line

```text
C5 candidate:
module publish/reuse path
-> selected compound/work graph source
-> saved module package
-> reload through ModuleLibrary
```
