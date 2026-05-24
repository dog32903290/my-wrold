# C3 Storage Command Path

Date: 2026-05-24 10:50 Asia/Taipei

## C3 Target

```text
UI / AI save_work command
-> validate active GraphSession
-> atomic PatchDocument write
-> save-ok commit-pending
-> background local git commit
-> saved-and-committed / save-ok commit-failed
-> save log can be read back
-> reload proof still passes
```

## C3.1 Closed Slice

```text
dirty GraphSession + WorkProject manifest path
-> save_work
-> formal PatchDocument save API writes work main patch
-> PatchDocument reload validates saved file
-> command log records save_work status
-> .myworld/save_log.jsonl records save_work status
```

## C3.1 Evidence

- `source/storage/StorageCommand.*` defines the first `saveWork()` command boundary.
- `saveWork()` loads the active `WorkProject` manifest, resolves `patches/main.patch.json`, preserves the current patch document id/title, calls `savePatchDocument()`, reloads the saved `PatchDocument`, appends `.myworld/save_log.jsonl`, and records `save_work:<status>` in `GraphSession.commandLog`.
- The C3.1 path does not call `serializeInteractionState()` and does not write `interaction-state-v1`.
- A successful dirty save returns `save-ok commit-pending`, clears `GraphSession.dirty`, and leaves git commit work unstarted.
- `tests/SaveWorkCommandTests.cpp` copies the C2 compound work fixture to a temp work project, dirties the active session, runs `saveWork()`, reloads `main.patch.json`, and checks public-port edges plus `library_loud1/mono_mix` layout at `358,146`.

## C3.1 Verification Gate

```text
cmake --build build --target my_world_save_work_command_tests
./build/my_world_save_work_command_tests
```

## Parked Outside C3.1

```text
visible Command+S wiring
app-level save_work proof dump
background local git add/commit worker
saved-and-committed / save-ok commit-failed transition
AI worker save_work caller
remote push/sync
```

## Next Line

```text
C3.2 visible/app save_work path:
active work project in app
-> user-visible save_work trigger or proof command
-> same StorageCommand boundary
-> proof dump includes save log evidence
```
