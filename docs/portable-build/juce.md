# Portable JUCE Build

## Goal

The repo must configure without hardcoded local JUCE paths. A developer or CI worker can either point CMake at an existing JUCE checkout or let CMake fetch the pinned JUCE tag.

## Resolution Order

CMake uses the first available JUCE source:

1. `MY_WORLD_JUCE_DIR` CMake cache value.
2. `MY_WORLD_JUCE_DIR` environment variable.
3. `JUCE_DIR` environment variable.
4. `../JUCE` next to this repository.
5. `FetchContent` when `MY_WORLD_FETCH_JUCE=ON`.

If none is available, CMake still configures core targets and prints a warning. JUCE app targets are skipped.

## Local Existing JUCE

```sh
cmake -S . -B build -DMY_WORLD_JUCE_DIR=/path/to/JUCE
cmake --build build
ctest --test-dir build --output-on-failure
```

Environment variable form:

```sh
MY_WORLD_JUCE_DIR=/path/to/JUCE cmake -S . -B build
cmake --build build --target my-world
```

## Fetch JUCE

```sh
cmake -S . -B build -DMY_WORLD_FETCH_JUCE=ON -DMY_WORLD_JUCE_TAG=8.0.12
cmake --build build
ctest --test-dir build --output-on-failure
```

## CI

GitHub Actions uses the fetch path:

```sh
cmake -S . -B build -DMY_WORLD_FETCH_JUCE=ON -DMY_WORLD_JUCE_TAG=8.0.12
cmake --build build
ctest --test-dir build --output-on-failure
```

## Parked

- Dependency cache tuning.
- Multi-platform matrix beyond macOS.
- Package manager integration.
- Automatic JUCE version bumps.
