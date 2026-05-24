# C4 AI Worker Command Contract

Date: 2026-05-24 11:24 Asia/Taipei

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

## C4.1 Evidence

- `source/ai/AIWorkerCommand.*` defines the minimal AI worker request/result shape.
- `allowedAIWorkerOperations()` exposes `save_work` as the first formal allowed operation.
- `executeAIWorkerCommand()` records AI intent, calls `StorageCommand::saveWork()` for the actual save, then records result/proof evidence.
- The AI path does not call `savePatchDocument()` directly and does not call `serializeInteractionState()`.
- `GraphSession::collaborationLog` records actor, command id, operation, intent, status, result, proof evidence, and error.
- `tests/AIWorkerCommandTests.cpp` proves dirty C2 compound work saves through the shared command path, reloads as `PatchDocument`, preserves public compound ports and expanded child layout, and reads back `.myworld/save_log.jsonl`.
- `--dump-c4-ai-worker-save-work-proof-and-exit` writes `debug/c4-ai-worker-save-work-proof/ai_worker_save_work_report.json`.

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

## Verification Gate

```text
cmake --build build --target my-world my_world_ai_worker_command_tests
./build/my_world_ai_worker_command_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c4-ai-worker-save-work-proof-and-exit
```

## Parked Outside C4.1

```text
natural language task parsing
AI graph mutation commands beyond save_work
repair loop / retry policy
remote push/sync
persisted collaboration_log.jsonl file
user-facing AI worker UI
```

## Next Line

```text
C4.2 candidate:
AI worker graph mutation command
-> allowed commandGraph operation
-> InteractionContract mutation path
-> save_work proof still reloads PatchDocument
```
