# C2 Compound Work Closure

Date: 2026-05-24 10:26 Asia/Taipei

## Closure Target

```text
C2 compound work
-> active GraphSession saved as PatchDocument file
-> work manifest main patch reload
-> fresh GraphSession/editorGraph/runtimeGraph
-> collapsed compound public ports and expanded child layout still match
-> app proof dump records reload evidence
-> C2 closed
```

## Closed Evidence

```text
C2.1 formal PatchDocument storage boundary
C2.2 active GraphSession -> PatchDocument file -> GraphSession reload proof
C2.3 WorkProject manifest -> main patch -> PatchDocument reload proof
C2.4 app-level C2 storage proof dump
```

## Proof Artifacts

```text
fixtures/storage/c2-compound-work/myworld.work.json
fixtures/storage/c2-compound-work/patches/main.patch.json
debug/c2-storage-proof/saved_main.patch.json
debug/c2-storage-proof/reload_report.json
```

The C2 report must say:

```text
ok: true
source: PatchDocument
usesInteractionState: false
publicInputEdge: true
publicOutputEdge: true
expandedLayout.matches: true
expandedLayout.nodeId: library_loud1/mono_mix
expandedLayout.x: 358
expandedLayout.y: 146
```

## Verification Gate

```text
cmake --build build
./build/my_world_patch_document_tests
ctest --test-dir build --output-on-failure
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-audio-proof-and-exit
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c2-storage-proof-and-exit
git diff --check
```

## Parked Outside C2

```text
Command+S keyboard path and background local git commit
AI worker save_work command loop
raw callback-buffer runtime execution
module publishing from a selected compound
multi-patch work libraries and remote sync
RenderBackend extraction and Metal backend
```

## Next Line

```text
C3 storage command path:
UI/AI save_work command
-> atomic PatchDocument write
-> save-ok commit-pending
-> background local git commit result
-> visible save log evidence
```
