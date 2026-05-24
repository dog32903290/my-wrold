# Repo Relocation Note - 2026-05-24

This file is a path/debug breadcrumb, not a new progress dashboard.

## Current Verified Repo

- Current working directory: `/Users/chenbaiwei/Projects/my-world`
- Display name remains `我的世界`.
- Branch at note time: `codex/tooll3-interaction-t0-t7`
- Local git state at note time: branch is ahead of origin by 3 commits; `AGENTS.md` was already modified before this note.
- CodeGraph was checked in the current path and reported an up-to-date index for `/Users/chenbaiwei/Projects/my-world`.
- `build/CMakeCache.txt` was checked after the move. `CMAKE_HOME_DIRECTORY` and `CMAKE_CACHEFILE_DIR` point to `/Users/chenbaiwei/Projects/my-world`, so no stale CMake source/build root was observed at note time.

## Historical Path Breadcrumbs

- `docs/superpowers/handoffs/2026-05-22-native-canvas-session-handoff.md` recorded the earlier repo path as `/Users/chenbaiwei/Desktop/我的世界`.
- R1 docs recorded an intermediate post-iCloud-move path, `/Users/chenbaiwei/Projects/我的世界`, and verified that proof output did not recreate the Desktop shadow project.
- Current repo path is the ASCII path `/Users/chenbaiwei/Projects/my-world`.

Do not rewrite older handoffs/specs just to normalize their paths. Those old paths are evidence of when a note was written. Treat this file and the master progress plan as the current path override.

## If Path Bugs Appear

Suspect a stale working directory, cached build root, old flow-runner workspace argument, app proof output root, or CodeGraph project path if future work:

- writes `debug/` or proof artifacts under `/Users/chenbaiwei/Desktop/我的世界`;
- writes artifacts under `/Users/chenbaiwei/Projects/我的世界` instead of `/Users/chenbaiwei/Projects/my-world`;
- fails CMake with impossible source/build paths;
- reports CodeGraph status for a path other than `/Users/chenbaiwei/Projects/my-world`;
- launches an old app bundle from a previous build location.

First checks:

```bash
pwd
git status --short --branch
rg -n "/Users/chenbaiwei/Desktop/我的世界|/Users/chenbaiwei/Projects/我的世界|/Users/chenbaiwei/Projects/my-world" docs build/CMakeCache.txt
rg -n "Desktop/我的世界|Projects/我的世界" scripts docs .myworld -g '*'
rg -n "CMAKE_HOME_DIRECTORY|CMAKE_CACHEFILE_DIR" build/CMakeCache.txt
codegraph status /Users/chenbaiwei/Projects/my-world
```

If `build/CMakeCache.txt` points to an old path, delete/reconfigure the build directory before treating source code as broken.
