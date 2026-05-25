# G1 Graph IO Mapping Foundation

## Status

Closed on 2026-05-25.

## Trigger

The AV lane proved that analyzer loudness can reach the native shader preview and produce visible reaction evidence. The remaining weak line is that the A1 to V1 mapping still lives mostly as app/controller wiring. G1 makes the first graph-readable mapping contract for one saved control line.

## Acceptance

- A graph-level mapping can name one `signal.float` source and one `shader.uniform` target.
- The mapping validates source id, target kind, target uniform, type, and stream.
- The mapping can produce a `LiveIOBinding` for the existing `LiveIOBus`.
- Missing source or invalid target is reported as diagnostics and emits no event.
- The proof report is JSON-readable and includes source, target, input value, normalized/clamped value, and diagnostics.
- No full mapping editor, UI binding surface, extra MIDI/OSC operator, direct realtime send, shader preview live binding expansion, Metal, or analyzer DSP is added.

## Target Line

```text
fixtures/graphs/g1_loudness_to_shader_uniform.graph.json
-> GraphIOMapping validation
-> LiveIOBus binding generation
-> graph_io_mapping_report.json
```

## Contract

```text
Mapping answers:
which saved graph source port drives which external/control target?

Conversion:
graph endpoint + target descriptor -> LiveIOBinding -> proof event

Source:
- id: compound.loudness.out
- dataType: signal.float
- streamKind: continuous

Target:
- kind: shader.uniform
- id: shader.preview.main
- uniformName: u_loudness
- dataType: signal.float

Policy:
- clamp normalized value to 0..1 through the existing LiveIOBus range policy.
- missing source: diagnostic, blocked report, no event.
- invalid target: diagnostic, blocked report, no event.
```

## Verification

- `cmake -S . -B build`
- `cmake --build build --target my_world_graph_io_mapping_tests`
- `./build/my_world_graph_io_mapping_tests`
- `cmake --build build --target my_world_live_io_bus_tests`
- `./build/my_world_live_io_bus_tests`
- `ctest --test-dir build --output-on-failure` passed 78/78.
- `git diff --check`
- `cmake --build build --target my-world`

## Evidence

- `fixtures/graphs/g1_loudness_to_shader_uniform.graph.json`
- `source/core/GraphIOMapping.h`
- `source/core/GraphIOMapping.cpp`
- `source/storage/GraphIOMappingStorage.h`
- `source/storage/GraphIOMappingStorage.cpp`
- `tests/GraphIOMappingTests.cpp`
- `CMakeLists.txt`

## Parked

- Full user-facing mapping editor.
- Multiple simultaneous user-authored mappings.
- Dynamic OSC scanning or broader MIDI operators.
- Direct realtime MIDI/OSC send from the audio callback.
- Shader preview live binding beyond the existing AV handoff.
- Metal, headless/offscreen GL expansion, and visual polish.
