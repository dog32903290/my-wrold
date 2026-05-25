# 我的世界

Native canvas prototype for shader preview, live audio analysis, compound patching, and proof-driven interaction work.

## Build

Use an existing JUCE checkout:

```sh
cmake -S . -B build -DMY_WORLD_JUCE_DIR=/path/to/JUCE
cmake --build build
ctest --test-dir build --output-on-failure
```

Or let CMake fetch the pinned JUCE tag:

```sh
cmake -S . -B build -DMY_WORLD_FETCH_JUCE=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

More detail: [docs/portable-build/juce.md](docs/portable-build/juce.md).
