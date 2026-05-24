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

## C3.2 Closed Slice

```text
save_work save log
-> structured save log readback API
-> app proof command
-> debug/c3-save-work-proof/save_work_report.json
-> report proves command log, save log, PatchDocument reload, public ports, expanded child layout
```

## C3.2 Evidence

- `loadSaveLog()` reads `.myworld/save_log.jsonl` back into `SaveLogEntry` records with `command`, `status`, `commitStatus`, work manifest path, and patch path.
- `--dump-c3-save-work-proof-and-exit` copies the C2 compound work fixture into `debug/c3-save-work-proof/work`, dirties a fresh `GraphSession`, runs the same `saveWork()` boundary, reloads `patches/main.patch.json`, reads `.myworld/save_log.jsonl`, and writes `debug/c3-save-work-proof/save_work_report.json`.
- The C3.2 app proof reports `ok: true`, `source: PatchDocument`, `usesInteractionState: false`, `commandLogStatus: save_work:save-ok commit-pending`, `saveLogStatus: save-ok commit-pending`, `commitStatus: not-started`, public-port edges, and `library_loud1/mono_mix` layout at `358,146`.

## C3.2 Verification Gate

```text
cmake --build build --target my-world my_world_save_work_command_tests
./build/my_world_save_work_command_tests
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-c3-save-work-proof-and-exit
```

## C3.3 Closed Slice

```text
visible Save Work button / Command+S
-> OpenGLShaderPreview save callback
-> MainComponent active work manifest
-> StorageCommand::saveWork()
-> GraphSession command log + PatchDocument write path
```

## C3.3 Evidence

- The ImGui bottom command strip now exposes `Save Work` instead of the temporary `Save State` / `Reload State` interaction serializer buttons.
- `OpenGLShaderPreview` catches `Command+S` and calls the same `ImGuiSmokeOverlay::requestSaveWork()` path as the visible button.
- `MainComponent::saveActiveWork()` prepares an ignored default active work project at `debug/c3-active-work/myworld.work.json` when `MY_WORLD_ACTIVE_WORK_MANIFEST` is not set, then calls `saveWork()` on the active `GraphSession`.
- The visible save hand no longer writes `interaction-state-v1`; `serializeInteractionState()` remains only for older interaction tests and trace fixtures.

## C3.3 Verification Gate

```text
cmake --build build --target my-world
rg -n "Save State|Reload State|savedInteractionState" source/ui source/render source/app
rg -n "Save Work|requestSaveWork|onSaveWorkRequested|commandModifier" source/ui source/render source/app
```

## Parked Outside C3.3

```text
background local git add/commit worker
saved-and-committed / save-ok commit-failed transition
AI worker save_work caller
remote push/sync
```

## Next Line

```text
C3.4 local git worker:
save-ok commit-pending
-> background local git add/commit in active work repo
-> saved-and-committed or save-ok commit-failed
-> final save log status readable
```
