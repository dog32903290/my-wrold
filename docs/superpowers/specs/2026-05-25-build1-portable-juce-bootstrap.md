# BUILD1 Portable JUCE Bootstrap

## Status

Closed on 2026-05-25.

## Trigger

The previous CMake setup depended on a developer-local JUCE checkout. This made app builds portable only when the same path or cache value existed.

## Acceptance

- CMake has no hardcoded `/Users/...` JUCE default.
- Existing local JUCE checkouts still work through `MY_WORLD_JUCE_DIR`, the `MY_WORLD_JUCE_DIR` environment variable, `JUCE_DIR`, or `../JUCE`.
- A clean worker can configure with `-DMY_WORLD_FETCH_JUCE=ON`.
- The fetched JUCE version is pinned by `MY_WORLD_JUCE_TAG`.
- GitHub Actions has a macOS CMake workflow that uses the fetch path.
- The workflow builds the default target set before running `ctest`, so test executables exist.
- Build instructions exist in repo docs.

## Target Line

```text
clean checkout
-> cmake -DMY_WORLD_FETCH_JUCE=ON
-> pinned JUCE FetchContent
-> my-world target
-> ctest
```

## Evidence

- `CMakeLists.txt`
- `.github/workflows/cmake.yml`
- `README.md`
- `docs/portable-build/juce.md`

## Verification

- Existing local JUCE path:
  `cmake -S . -B build`
- App build:
  `cmake --build build`
- Full suite:
  `ctest --test-dir build --output-on-failure`
- Fetch path configure smoke:
  `cmake -S . -B cmake-build-juce-fetch -DMY_WORLD_FETCH_JUCE=ON -DMY_WORLD_JUCE_TAG=8.0.12`
- Fetch path build smoke:
  `cmake --build cmake-build-juce-fetch`
- Fetch path full suite:
  `ctest --test-dir cmake-build-juce-fetch --output-on-failure` passed 77/77.
- Diff check:
  `git diff --check`

## Parked

- CI dependency cache tuning.
- Linux/Windows matrix.
- Package manager integration.
- Automatic JUCE version update workflow.
