# Node Function Spec From TiXL Witness

Date: 2026-05-23
Status: TiXL `Operators/Lib` witness catalog exists; only five seed node function specs are promoted here. No TOP/SOP/POINT/RENDER runtime implementation is implied by this file.

## Purpose

This file defines how `我的世界` borrows TiXL / Tooll3 operator vocabulary without inheriting TiXL's C# class shape, Direct3D runtime, HLSL backend, or Symbol database as native law.

The borrowing path is:

```text
TiXL operator witness
-> My World node function spec
-> graph fixture
-> C++ runtime proof
-> UI reads proof-backed state
```

The witness catalog is:

```text
fixtures/tixl-witness/operator-catalog.json
```

That catalog is a searchable map, not a backlog commitment. Parked entries do not become planned nodes until a small proof slice promotes them.

User-facing browser taxonomy parity is governed separately by:

```text
docs/superpowers/specs/2026-05-24-tixl-taxonomy-parity-spec.md
```

That spec decides which TiXL paths appear in the default browser, which paths are hidden/import-only, and which visible entry points need acceptance traces before implementation.

## Borrowing Boundary

Allowed to borrow:

- Operator intent and user-facing vocabulary.
- Input/output slot naming patterns.
- Parameter defaults, ranges, enum shape, and grouping hints.
- Nested patch structure as a clue for cook/runtime decomposition.
- Human descriptions as search and manual-writing source material.

Not allowed as native law:

- C# `Instance<T>` class structure.
- TiXL GUIDs as our graph identity.
- SharpDX / Direct3D / HLSL backend assumptions.
- `.t3` child graph as our runtime graph schema.
- `.t3ui` layout positions as our UI layout contract.
- Any node whose only proof is that it appears in a TiXL menu.

## Catalog Rule

The first catalog contains all discovered TiXL `Operators/Lib` operators. Examples, skills, plugins, and legacy folders stay out of this first witness surface until a later pass explicitly needs them. The complete node function spec must remain small.

```text
All TiXL Operators/Lib operators -> witness catalog -> status parked
Only selected seed nodes -> complete node function spec -> proof fixtures
Only passing proof fixtures -> implementation
```

Current catalog scan:

```text
source repo: https://github.com/tixl3d/tixl.git
source commit: 26dc80c7e3a85389f6619ffcd76b1e75e03547aa
scanned root: Operators/Lib
entries: 923
complete .cs + .t3 + .t3ui triples: 922
seed candidates with direct TiXL witnesses: 4
```

`image.constant` is a My World seed even though TiXL does not expose a single exact one-to-one operator under that name. It borrows the texture-generation role from TiXL image generators and render target clear behavior, but the first implementation should be native and minimal.

## Canonical Types And Browser Aliases

Saved graph node types use My World categories. TOP/SOP-style names can appear in the node browser, search, tutorials, and imported-language hints, but they are aliases, not storage law.

| Saved node type | Browser alias | Rule |
| --- | --- | --- |
| `image.constant` | `top.constant` | TOP means "texture operator" to the user; saved graph keeps `image.*`. |
| `image.blur` | `top.blur` | Borrowed from TiXL image/blur witness; saved graph keeps `image.*`. |
| `mesh.cube` | `sop.cube` | SOP means "geometry operator" to the user; saved graph keeps `mesh.*`. |
| `point.grid` | `point.grid` | No alias needed yet; output is `point.cloud`. |
| `render.screen_quad` | `render.screen_quad` | Render nodes emit `command.graph` or framebuffer contributions. |

Do not implement duplicate native nodes named both `top.blur` and `image.blur`. One runtime node may have many search aliases, but only one saved type.

## Type Mapping

| TiXL / C# witness | My World TypeSpec | Status |
| --- | --- | --- |
| `Texture2D` | `texture.rgba` | existing |
| `MeshBuffers` | `geometry.mesh` | existing |
| `BufferWithViews` for point operators | `point.cloud` | existing |
| `Command` | `command.graph` | existing |
| `float` / `System.Single` | `signal.float` | existing |
| `string` | `text.string` | existing |
| `DataSet` / object-like payload | `data.object` | existing |
| `ShaderGraphNode` / shader material payload | `material.shader` | existing |
| `int` | `signal.int` | missing |
| `bool` | `signal.bool` | missing |
| `Int2` / integer resolution pair | `vector.int2` | missing |
| `Int3` / integer segment triplet | `vector.int3` | missing |
| `Vector2` | `vector.float2` | missing |
| `Vector3` | `vector.float3` | missing |
| `Vector4` | `vector.float4` or `color.rgba` | missing / needs decision |
| GPU buffer not known to be points | `data.buffer` | missing / parked |
| `ParticleSystem` | `particle.system` | missing / parked |

Do not add all missing types at once. Add a type only when a promoted seed node needs it. Current C++ seed specs still use some raw widget strings like `float`, `int`, and `string`; do not copy those raw widget names into new patch graph TypeSpecs.

## Seed Node Function Specs

### `image.constant` (browser alias `top.constant`)

TiXL witness:

- No exact one-to-one TiXL operator selected.
- Related witness: `Operators/Lib/image/generate/basic/RenderTarget.cs` for clear-color texture behavior.
- Related witness: `Operators/Lib/image/generate/basic/CheckerBoard.cs` for simple generated texture output.

Intent:

```text
params -> texture.rgba
```

Ports:

```text
outputs:
- out: texture.rgba
```

Params:

```text
color: color.rgba, default [0.02, 0.02, 0.02, 1.0]
resolution: vector.int2, default [1280, 720]
format: enum.texture_format, default rgba8, parked until RenderBackend extraction
```

Cook behavior:

- Produces a texture filled with one color.
- Re-cooks when `color`, `resolution`, or frame format changes.
- Does not depend on input graph order.

Failure behavior:

- Invalid resolution clamps or rejects before GPU allocation.
- If texture allocation fails, keep previous valid output and write an error.

Debug fields:

```text
resolution
format
color
cookTimeMs
outputSummary
lastError
```

First proof:

```text
fixtures/runtime/top_constant_to_output.graph.json
-> core validator / proof dumper
-> debug/top-constant/texture_summary.json
-> debug/top-constant/cook_order.json
-> debug/top-constant/node_stats.json
-> debug/top-constant/errors.json
```

No `frame.png` is required for this first proof. The first line only proves graph parsing, node contract, cook order, and texture metadata. Visual frame dumping comes after the RenderBackend/runtime boundary is extracted.

### `image.blur` (browser alias `top.blur`)

TiXL witness:

- `Operators/Lib/image/fx/blur/Blur.cs`
- `Operators/Lib/image/fx/blur/Blur.t3`
- `Operators/Lib/image/fx/blur/Blur.t3ui`

Intent:

```text
texture.rgba -> texture.rgba
```

Ports:

```text
inputs:
- image: texture.rgba, required

outputs:
- out: texture.rgba
```

Params:

```text
size: signal.float, default 1.0, range 0.0..20.0
samples: signal.float, default 8.0, range 0.0..100.0
offset: signal.float, default 0.0
opacity: signal.float, default 1.0
resolution: vector.int2, default [0, 0] meaning inherit input
wrap: enum.texture_address, default MirrorOnce
```

Cook behavior:

- Reads input texture and writes a new texture.
- Inherits input resolution unless `resolution` is non-zero.
- Re-cooks when upstream texture, blur params, or resolution changes.

Failure behavior:

- Missing input produces no output and writes `missing required input: image`.
- Invalid sample count is clamped by validation before cook.
- Shader compile/runtime failure keeps previous valid output where possible.

Debug fields:

```text
inputResolution
outputResolution
size
samples
wrap
cookTimeMs
lastError
```

First proof:

```text
fixtures/runtime/top_constant_blur_to_output.graph.json
-> core validator / proof dumper
-> debug/top-blur/texture_summary.json
-> debug/top-blur/cook_order.json
-> debug/top-blur/node_stats.json
-> debug/top-blur/errors.json
```

No `frame.png` is required for this first proof. It proves that a real upstream `image.constant` summary can feed `image.blur` without UI-only state.

### `mesh.cube` (browser alias `sop.cube`)

TiXL witness:

- `Operators/Lib/mesh/generate/CubeMesh.cs`
- `Operators/Lib/mesh/generate/CubeMesh.t3`
- `Operators/Lib/mesh/generate/CubeMesh.t3ui`

Intent:

```text
params -> geometry.mesh
```

Ports:

```text
outputs:
- mesh: geometry.mesh
```

Params:

```text
scale: signal.float, default 1.0
stretch: vector.float3, default [1.0, 1.0, 1.0]
center: vector.float3, default [0.0, 0.0, 0.0]
rotation: vector.float3, default [0.0, 0.0, 0.0]
pivot: vector.float3, default [0.0, 0.0, 0.0]
segments: vector.int3, default [1, 1, 1]
texCoord: enum.uv_mode, default 0
texCoord2: enum.uv_mode, default 0
margin: signal.float, default 0.0
margin2: signal.float, default 0.0
```

Cook behavior:

- Generates vertex and index buffers for a cube mesh.
- Re-cooks when geometry params change.
- Does not render by itself.

Failure behavior:

- Segment counts must be clamped or rejected before allocation.
- Invalid mesh allocation writes an error and keeps previous valid mesh if available.

Debug fields:

```text
vertexCount
indexCount
faceCount
bounds
segments
cookTimeMs
lastError
```

First proof:

```text
fixtures/runtime/sop_cube_to_mesh_summary.graph.json
-> core validator / proof dumper
-> debug/sop-cube/mesh_summary.json
-> debug/sop-cube/cook_order.json
-> debug/sop-cube/node_stats.json
-> debug/sop-cube/errors.json
```

Rendering this cube is a later proof, not part of the first `mesh.cube` acceptance.

### `point.grid`

TiXL witness:

- `Operators/Lib/point/generate/GridPoints.cs`
- `Operators/Lib/point/generate/GridPoints.t3`
- `Operators/Lib/point/generate/GridPoints.t3ui`

Intent:

```text
params -> point.cloud
```

Ports:

```text
outputs:
- points: point.cloud
```

Params:

```text
countX: signal.int, default 1
countY: signal.int, default 1
countZ: signal.int, default 1
sizeMode: enum.point_size_mode, default Cell
size: vector.float3, default [1.0, 1.0, 1.0]
scale: signal.float, default 1.0
center: vector.float3, default [0.0, 0.0, 0.0]
pivot: vector.float3, default [0.0, 0.0, 0.0]
tiling: enum.grid_tiling, default Cartesian
pointScale: signal.float, default 1.0
color: color.rgba, default [1.0, 1.0, 1.0, 1.0]
orientationAxis: vector.float3, default [0.0, 0.0, 1.0]
orientationAngle: signal.float, default 0.0
f1: signal.float, default 0.0
f2: signal.float, default 0.0
w: signal.float, default 0.0
```

Cook behavior:

- Generates a point cloud with deterministic ordering.
- Re-cooks when counts, transform, tiling, or point attributes change.
- Does not draw by itself.

Failure behavior:

- Count product must have a hard maximum before allocation.
- Invalid tiling enum falls back to Cartesian or rejects at validation.

Debug fields:

```text
pointCount
bounds
tiling
attributeSummary
cookTimeMs
lastError
```

Parked proof name:

```text
fixtures/runtime/point_grid_to_point_summary.graph.json
-> core validator / proof dumper
-> debug/point-grid/point_summary.json
-> debug/point-grid/cook_order.json
-> debug/point-grid/node_stats.json
-> debug/point-grid/errors.json
```

This proof stays parked until `point.cloud` output summaries exist. Do not use a visual point-cloud demo as a substitute for the summary artifact.

### `render.screen_quad`

TiXL witness:

- `Operators/Lib/render/basic/DrawScreenQuad.cs`
- `Operators/Lib/render/basic/DrawScreenQuad.t3`
- `Operators/Lib/render/basic/DrawScreenQuad.t3ui`

Intent:

```text
texture.rgba -> command.graph / framebuffer contribution
```

Ports:

```text
inputs:
- texture: texture.rgba, optional

outputs:
- command: command.graph
```

Params:

```text
color: color.rgba, default [1.0, 1.0, 1.0, 1.0]
width: signal.float, default 1.0
height: signal.float, default 1.0
blendMode: enum.blend_mode, default 0
enableDepthTest: signal.bool, default false
enableDepthWrite: signal.bool, default false
position: vector.float2, default [0.0, 0.0]
filter: enum.texture_filter, default linear
```

Cook behavior:

- Emits a render command that draws a texture as a screen-space quad.
- If no texture is connected, emits a solid quad using `color`.
- Does not own final framebuffer output by itself.
- Requires a render/output context to become visible.

Failure behavior:

- Invalid connected texture produces no command and writes an input error.
- Unsupported blend/filter modes are rejected by validation before render.

Debug fields:

```text
inputResolution
blendMode
position
size
renderCommandCount
cookTimeMs
lastError
```

Parked proof name:

```text
fixtures/runtime/top_constant_screen_quad_to_output.graph.json
-> render command runtime
-> debug/screen-quad/frame_001.png
-> debug/screen-quad/cook_order.json
-> debug/screen-quad/node_stats.json
-> debug/screen-quad/errors.json
```

This proof stays parked behind the first texture-summary proof and the RenderBackend/runtime extraction. Do not connect `mesh.cube` directly into `render.screen_quad`; the honest future mesh visual line is `mesh.cube -> render.draw_mesh -> output.preview`.

## Promotion Gates

A parked catalog entry can be promoted only when all answers exist:

```text
1. What data conversion does it perform?
2. What are the exact input/output TypeSpecs?
3. What are the required params and safe defaults?
4. What does cook do, and when does it re-cook?
5. What are the failure modes?
6. What debug fields prove it ran?
7. What headless fixture proves it without UI?
```

If a node cannot answer these, it remains witness material.

## Next Slice

After C1.3/C1.4 module-library work is clean, the first runtime slice should be:

```text
fixtures/runtime/top_constant_to_output.graph.json
-> image.constant
-> output.texture_summary
-> texture_summary.json + cook_order.json + node_stats.json + errors.json
```

This slice is intentionally headless: no UI, no OpenGL requirement, and no `frame.png` yet. Only after that should `image.blur` enter. SOP and POINT proofs can define contracts now, but their runtime should not outrun the first image cook proof.
