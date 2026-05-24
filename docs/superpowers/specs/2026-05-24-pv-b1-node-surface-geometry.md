# PV-B1.1 Node Surface Geometry

Status: closed as of 2026-05-24 22:33 Asia/Taipei

## Question

Can analyzer compound nodes with long labels and multiple public ports be drawn and hit-tested from one TiXL-style row geometry contract, without changing analyzer DSP, runtime execution, MIDI, shader mapping, or render export?

## Contract

```text
GraphNode + NodeSpec
-> CanvasNodeSurfaceGeometry
-> ImGui node drawing
-> InteractionContract hit-test / portCenter
```

The node surface geometry owns:

- node bounds
- title bounds
- input row bounds
- output row bounds
- port strip bounds
- port label bounds
- port centers

Drawing and hit-test must read this geometry instead of maintaining separate node-size and port-row math.

## Fixture

Minimum pressure fixture:

```text
compound.attack
input:
- audio.onset_event

outputs:
- onset_event
- attack_value
- attack_envelope
- confidence
```

The fixture represents the visible PV analyzer nodes that broke the old fixed `140 x 60` surface.

## Proof

Targeted proof:

```text
CanvasGeometryContractTests
-> compound.attack layout width grows from long labels
-> height grows to contain four output rows
-> output rows, label bounds, and final port center remain inside node bounds

NodeHitTestTests
-> final compound.attack output port is hit-testable
-> body hit works on the final output row
```

Build proof:

```text
cmake --build build --target my_world_canvas_geometry_contract_tests my_world_node_hit_test_tests
ctest --test-dir build --output-on-failure -R 'canvas_geometry_contract|node_hit_tests'
cmake --build build --target my-world
```

Latest accepted verification:

```text
cmake --build build --target my_world_canvas_geometry_contract_tests my_world_node_hit_test_tests
ctest --test-dir build --output-on-failure -R 'canvas_geometry_contract|node_hit_tests'
cmake --build build --target my-world
MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-pv-b1-analyzer-environment-proof-and-exit
ctest --test-dir build --output-on-failure
git diff --check
```

Accepted result:

- focused geometry and hit-test tests passed
- app target built
- PV-B1 analyzer environment proof exited 0 and `analyzer_environment_report.json` has `ok: true`
- full `ctest` passed 40/40
- `git diff --check` passed

## Boundary

This lane does not touch:

- analyzer DSP semantics
- MIDI mapping
- shader uniform mapping
- live callback runtime
- Metal
- image.blur
- node thumbnails
- SOP/MAT/POINT
- render export
- TiXL runtime body or C# class structure
