# Tooll3 Skin Parity Spec

Date: 2026-05-22
Status: P0-P7 first pass implemented: source packet recorded, testable skin contract added, old global shader/prefs panels hidden, output now reads as workspace background, nodes float over it, left/bottom Tooll3-style rails exist, node/port/connection skin grammar is contract-backed, selected shader source lives in the node inspector with real compile handoff, workspace tabs/context-menu/transport grammar exist, Delete follows selected-object command behavior, and the visible browser consumes the loaded module-library registry. C1.10 loaded internal-edge value-bus proof exists for loaded modules. S8 full timeline editing, output pinning, live thumbnails, RuntimeOp dispatch extraction, and deeper node browser polish are still pending.

Source witness: `jithinraj/t3` cloned for inspection at upstream commit `61d254c3e3107eaa1f64239dd0e399150a68436b`.

License stance: Tooll3/T3 is MIT licensed in the inspected repository. If this project copies source code, style constants, icon data, or substantial implementation structure, preserve the MIT notice and attribution. This spec's preferred route is to borrow the interface grammar and reimplement it in `我的世界`'s C++/JUCE/ImGui body.

## Current Progress Snapshot

Date: 2026-05-23 10:46 Asia/Taipei.

已鎖定:

- The app no longer reads as `shader editor + separate preview`; the main read is now `live output workspace + graph overlay + edge panels`.
- P0-P7 skin parity first pass is implemented and proof-backed by tests and proof dump.
- Delete is now part of Tooll3-style selection behavior: selected edge lowers to `disconnect`, selected node lowers to `delete_node`.
- C1.3 visible module registry proof exists, so the skin now consumes loaded module specs instead of only seed specs when creating nodes.
- C1.4 module-library index proof exists, so the browser registry is fed through the default saved `ModuleLibrary` index rather than an app-side loudness module path.
- C1.5 runtime registry proof exists, so app proof dump can show loaded compounds as runtime entries instead of only browser/editor entries.
- C1.6 dry-run proof exists, so app proof dump can show per-child loaded-compound runtime status without claiming full computation.
- C1.7 first executable child proof exists, so app proof dump can show `analyzer.rms` as a computed loaded child with `rms=0.707107` and `peak=1.000000`, while unimplemented siblings remain explicit.
- C1.8 mini-chain proof exists, so app proof dump can show `audio.mono_mix`, `analyzer.rms`, and `analyzer.analysis_gain` as computed children with explicit `inputs` and `outputs`.
- C1.9 public-output proof exists, so app proof dump can show all seven loaded loudness children as computed and entry-level `publicOutputs`.
- C1.10 internal-edge value-bus proof exists, so app proof dump can show runtime values sourced from loaded `internalEdges` and `publicOutputMappings`.

正在試壓:

- Whether loaded library modules can move node execution into named RuntimeOp dispatch units without turning UI status into fake execution.

還沒承重:

- S8 timeline editing has only a visual/status strip, not animation commandGraph contracts.
- S9 browser works for right-click and drag-to-empty creation, but keyboard-first browser polish and insert-between-edge behavior remain parked.
- Module-library discovery, runtime registry proof, dry-run status, computed runtime chain, public output map, and source-route evidence are wired, but the browser still needs deeper category/search ergonomics and dispatch-backed runtime output status once more modules exist.
- Live node thumbnails and output pin/multi-output workflow are not proven until `RenderBackend` is extracted.

下一根線:

- Keep new skin surface area parked until loaded modules execute through named RuntimeOps. The proof dump records route sources now; next pressure is execution shape, not more panel polish.

## Purpose

The previous T0-T7 work proved a Tooll3-like interaction spine:

```text
pan / zoom / select / move / connect / delete / undo / save / trace
```

That is not the same as Tooll3's skin.

Tooll3's visual identity comes from a different layer:

```text
live output as the workspace
-> nodes floating over that output
-> dark translucent panels around the work
-> compact typed nodes
-> parameter inspector and timeline woven into the same surface
```

This spec defines the skin parity target before more UI code is written. "Parity" here means maximum practical UI/operation specification parity with Tooll3, not code forking.

## Core Diagnosis

Current native UI still reads as:

```text
shader editor + framed node canvas + preview/panels
```

Tooll3 reads as:

```text
live artwork/output surface + node controls floating on top + editor panels attached to the edges
```

The main difference is not button color. It is the compositing model.

In Tooll3, the graph canvas can use the rendered output as its background. The node graph is an overlay on the result, not a separate box next to it. In the inspected code this is represented by `GraphWindow.ImageBackground.cs` rendering an output into the graph window background, and `GraphWindow.cs` suppressing the grid when that image background is active.

## Source Witness Files

Primary files to inspect again during implementation:

```text
Editor/Gui/Styling/T3Style.cs
Editor/Gui/Styling/Color.cs
Editor/Gui/Styling/ColorVariations.cs
Editor/Gui/Styling/CustomComponents.cs
Editor/Gui/InputUi/TypeUiRegistry.cs
Editor/Gui/Windows/Window.cs
Editor/Gui/Graph/GraphWindow.cs
Editor/Gui/Graph/GraphWindow.ImageBackground.cs
Editor/Gui/Graph/GraphCanvas.cs
Editor/Gui/Graph/GraphNode.cs
Editor/Gui/Graph/InputNode.cs
Editor/Gui/Graph/OutputNode.cs
Editor/Gui/Graph/Interaction/ConnectionMaker.cs
Editor/Gui/Windows/ParameterWindow.cs
Editor/Gui/Windows/Output/OutputWindow.cs
Editor/Gui/Windows/TimeLine/TimeLineCanvas.cs
Editor/Gui/Windows/Variations/VariationBaseCanvas.cs
```

Borrow these as witnesses for visual grammar and interaction state. Do not import their C# ownership model as our native law.

## Non-Negotiable Boundary

Allowed to borrow:

- Full-bleed output-as-background composition.
- Dark, low-padding ImGui style.
- Flat, square, non-rounded window/panel grammar.
- Type-colored nodes and ports.
- Compact node labels, tiny indicators, inline port/value rows.
- Parameter popup / inspector rhythm.
- Presets / snapshots / variations panel rhythm.
- Bottom transport / timeline strip.
- Output pinning and selected-output preview behavior.
- Context menu and node browser placement logic.

Not allowed as native law:

- C# class structure.
- SharpDX / Direct3D / HLSL assumptions.
- Tooll3 `Symbol` / `Instance` runtime as our graph schema.
- Tooll3 branding, exact title strings, or identity.
- UI-only graph mutation.
- A skin layer that bypasses `InteractionContract` / commandGraph.

## Skin Layers

### S0 Attribution And Source Packet

Every implementation pass that copies non-trivial code or constants from Tooll3 must list:

```text
source repo
source commit
source files
what was copied vs reimplemented
license notice location
```

Preferred default: reimplement behavior and style tokens from observation instead of copying code.

### S1 Global Style Tokens

Tooll3 witness: `T3Style.cs`, `Color.cs`, `ColorVariations.cs`.

Target grammar:

- Window background nearly black, around `0.05-0.10` luminance.
- Panels are dark translucent overlays, not cards.
- Window padding is close to zero.
- Frame padding is tight.
- Rounding is zero or almost zero.
- Separators are dark structural lines.
- Active accents are blue/cyan, but not the whole palette.
- Text is mostly low-contrast white/gray, with stronger white only for active labels.

Acceptance:

- No rounded card aesthetic.
- No large explanatory text on the workspace.
- No grid panel that looks like a separate app inside the app.

### S2 Workspace Composition

Tooll3 witness: `GraphWindow.cs`.

Target layout:

```text
top menu / status strip
left edge panels: presets / snapshots / selected node inspector
center: full-bleed live output background
center overlay: node graph
bottom: transport / time / timeline strip
```

The center workspace owns the visual weight. Side panels must feel attached to the workspace, not like independent forms.

Acceptance:

- On launch, at least 70% of the app's usable area is the live output/workspace surface.
- Shader source editor is not globally visible by default.
- Selecting a Shader node may reveal shader source in the inspector.
- The node canvas is not drawn inside a bordered rectangular sub-panel.

### S3 Output-As-Background

Tooll3 witness: `GraphWindow.ImageBackground.cs`, `OutputWindow.cs`.

Target behavior:

```text
out1 / pinned output
-> renders into the workspace background
-> nodes and connections draw over it
-> grid becomes hidden or extremely muted while output background is active
```

This is the skin's main load-bearing line.

Acceptance:

- `shader1 -> out1` renders as the workspace background, not as a small preview panel.
- `out1` still exists as a node and can be selected/pinned.
- The background survives pan/zoom/selection without breaking hit-tests.
- If shader compilation fails, last valid output remains visible with status in the inspector/status strip.

### S4 Node Visual Grammar

Tooll3 witness: `GraphNode.cs`, `TypeUiRegistry.cs`, `ColorVariations.cs`.

Target node shape:

```text
flat rectangle
typed color wash
compact label
left input strip
right output strip
inline input labels / values when zoom allows
tiny status indicators
selection outline
optional cached preview above/inside visual nodes later
```

Node color follows data/runtime type, not arbitrary decoration:

```text
values      gray
points      muted rose
strings     green
textures    magenta
commands    cyan
audio       project-specific blue/teal
shader      project-specific violet/cyan
compound    muted structural color
output      emphasized but not neon
```

Acceptance:

- Nodes can be scanned by type before reading text.
- Text does not carry the whole node identity.
- Selected state is an outline/contrast change, not layout change.
- Hover and active states change color/opacity, not size.

Current proof:

- `Tooll3SkinContract` now defines typed colors, square node skins, stable hover/selection geometry, and role labels for shader/output/audio/signal/compound/value nodes.
- ImGui node drawing uses that contract for node fills, labels, outlines, and left/right side strips.
- `tooll3_skin_contract` tests verify type-scannable colors and geometry-stable state changes.

### S5 Ports And Connections

Tooll3 witness: `GraphNode.cs`, `ConnectionMaker.cs`.

Target grammar:

- Inputs live on the left side of nodes.
- Outputs live on the right side.
- Connection color follows data type.
- Dragging a connection highlights compatible targets and mutes incompatible targets.
- Dropping on empty workspace opens compatible node browser at the drop position.
- Existing graph mutation still goes through commandGraph.

Acceptance:

- No successful connection exists only as pixels.
- Cancelled drag leaves graph state unchanged.
- Compatible-node popup is spatially attached to the release point.
- Delete acts on the current graph selection: selected edge lowers to `disconnect`, selected node lowers to `delete_node`.

Current proof:

- Connections now use `Tooll3ConnectionSkin`, so edge color follows `GraphEdge.dataType` and selected edges thicken without changing graph state.
- Ports now render as left/right colored strips from `PortSpec.dataType` while hit-tests and mutations continue through `InteractionContract`.
- Dragging from an output still uses commandGraph-backed connect/create behavior; full compatible-target dimming polish remains parked for P5.
- Delete button/key now follows Tooll3-style selected-object behavior instead of the old edge-only proof path.

### S6 Inspector And Parameter Skin

Tooll3 witness: `ParameterWindow.cs`, input UI files under `Editor/Gui/InputUi`.

Target grammar:

```text
selected node
-> compact header
-> parameter rows
-> manual/default/connected/animated state visible
-> source/code editors appear only inside node-specific panels
```

For `shader.fragment`, shader source belongs here:

```text
select shader1
-> Inspector / Shader Source panel opens
-> compile status shown near it
-> preview remains the workspace background
```

Acceptance:

- No global half-screen shader editor.
- Parameter edits create commandGraph evidence.
- Connected parameters visibly differ from manual values.

Current proof:

- `Tooll3SkinContract` now defines inspector policy: shader source is owned by the selected shader node, compile status is shown with that source panel, and source is not global.
- Inspector rows now use visible default/manual/connected/animated state accents from `Tooll3InspectorRowSkin`.
- `shader.fragment` source edits live in the ImGui Inspector; `Apply Source` runs `set_param` on the selected node and hands the source to `OpenGLShaderPreview` for compilation.
- The hidden JUCE shader editor no longer carries the visible editing surface; it remains parked as legacy plumbing until the preview/graph ownership boundary is fully collapsed.

### S7 Left Panels: Presets, Snapshots, Node Context

Tooll3 witness: `VariationBaseCanvas.cs`, variation/snapshot windows.

Target grammar:

- Left rail is dark and attached to the workspace.
- Top tabs can hold `Presets`, `Snapshots`, later `Library`.
- Empty states are quiet.
- Node context/description lives below, tied to current selection.

Acceptance:

- Left panel can be collapsed or narrowed later.
- The first version may show empty presets/snapshots, but the proportions must match the Tooll3 rhythm.
- It must not compete visually with the central output.

Current proof:

- `Tooll3SkinContract` defines left-rail primary tabs as `Presets`, `Snapshots`, and `Library`, with quiet empty states and selected-node context attached below.
- The ImGui left rail now uses a tab bar for Presets/Snapshots/Library while keeping the selected-node Inspector in the same rail.

### S8 Bottom Transport And Timeline

Tooll3 witness: `TimeLineCanvas.cs`, `TimeControls.cs`, `GraphWindow.cs`.

Target grammar:

```text
bottom strip
-> transport controls
-> time display
-> optional timeline/dope view
```

The first native pass can be a nonfunctional visual/contract strip if realtime timeline logic is not ready, but it must not fake graph/runtime state.

Acceptance:

- Bottom strip exists as a workspace boundary.
- Time state shown must come from real app time/frame state if displayed.
- Timeline editing is parked until commandGraph animation contracts exist.

Current proof:

- `Tooll3SkinContract` defines the transport boundary as real app time, command strip, trace status, and parked timeline editing.
- The ImGui bottom rail now renders time, command buttons, trace/status text, and a stable non-editing timeline/playhead strip.

### S9 Node Browser And Context Menu

Tooll3 witness: `GraphCanvas.cs`, `CustomComponents.cs`.

Target grammar:

- Right-click on workspace opens context menu.
- Drag connection to empty workspace opens compatible node browser.
- Browser appears at gesture position.
- Search/filter is dense and keyboard-friendly.

Acceptance:

- Node browser never creates untyped/dangling edges.
- Search results are filtered by `NodeSpec` and port compatibility.

Current proof:

- `Tooll3SkinContract` defines right-click workspace browser policy: empty-canvas only, gesture-anchored, search/filter capable, commandGraph-backed, and no dangling edges.
- Right-click on empty canvas opens a node browser at the gesture position; selecting a result calls `createNode` against the passed visible registry, validates `NodeSpec`, mutates via `InteractionContract`, and selects the created node.
- Drag-to-empty compatible node creation remains filtered by source port type; full keyboard-first browser ergonomics remain parked.
- Inserting a node produces one macro command where appropriate.

## Current UI Gap Table

| Surface | Current Native State | Tooll3 Parity Target | Next Pressure |
| --- | --- | --- | --- |
| Main visual | `out1` output owns the center workspace and nodes float over it | Output is the workspace background | Output pinning and multi-output selection |
| Shader editor | Shader source lives in selected-node ImGui inspector; legacy JUCE plumbing is hidden from the main read | Node-specific inspector panel | Formalize inspector state and compile errors as selected-node data |
| Output preview | Background is authoritative for the first output proof | Background is primary output, preview panel is secondary/pinned | Pinned/secondary output workflow |
| Nodes | Flat typed rectangles with type colors, port strips, selection outline, and connection colors | Flat typed operator rectangles | Live thumbnails and compact inline value affordances after `RenderBackend` |
| Grid | Hidden/muted behind output background | Hidden/muted when output background active | No extra work until multiple background modes exist |
| Panels | Left rail and bottom strip are unified ImGui workspace surfaces | Edge-attached Tooll3 panels | Collapse/resize persistence after storage path matures |
| Timeline | Bottom strip shows real time/status plus non-editing playhead | Bottom strip with transport/time | Animation commandGraph contract before editing |
| Module browser | Browser consumes a merged visible registry sourced from seed specs plus the default saved `ModuleLibrary` index | Saved module libraries feed search/create without app hardcoding | Runtime status and deeper browser ergonomics after more modules exist |

## First Implementation Roadmap

### P0 Skin Packet

Create a tiny `Tooll3SkinNotes` doc or source comment packet listing witness files and MIT attribution decision.

Gate:

```text
source commit recorded
license note recorded
no copied code without attribution
```

### P1 Style Tokens

Add native style tokens for:

```text
workspace background
panel background
node type colors
text muted/active
connection colors
separator / splitter
selection outline
```

Gate:

```text
proof frame shows global Tooll3-like dark flat style
no layout changes yet
```

### P2 Workspace Shell

Replace current workspace shell with:

```text
top strip
left rail
full-bleed center surface
bottom strip
floating nodes
```

Gate:

```text
node canvas no longer appears inside a bordered sub-panel
shader editor is hidden unless shader node selected
```

### P3 Output Background

Render the shader output as the center workspace background.

Gate:

```text
proof frame: shader output fills the center workspace
nodes draw on top
hit-tests still pass through CanvasHands / InteractionContract
```

### P4 Node Skin

Implement Tooll3-like node surfaces over existing `GraphSession`.

Gate:

```text
typed node colors visible
port strips visible
connection color follows data type
selection/hover states do not shift layout
```

### P5 Inspector Skin

Move selected-node details into a Tooll3-like left/side inspector.

Gate:

```text
select shader1 -> shader params/source visible
select out1 -> output preview/pin status visible
no selected node -> quiet empty inspector
```

### P6 Bottom Strip

Add transport/time strip using real app frame/time state.

Gate:

```text
bottom strip displays real time/frame
no fake timeline editing
```

### P7 Skin Parity Review

Run visual pressure:

```text
desktop screenshot
proof dump
CanvasHands click/drag/connect trace
current UI vs Tooll3 reference comparison
```

Gate:

```text
the app reads as output-first visual workspace, not shader editor + node panel
```

## Acceptance Checklist

- [x] Full-bleed output owns the center workspace.
- [x] Nodes float over output, not inside a framed canvas panel.
- [x] Shader source is selected-node detail, not global layout.
- [x] `out1` output is visible as live background or pinned output.
- [x] Left panel carries presets/snapshots/inspector rhythm.
- [x] Bottom strip carries transport/time rhythm.
- [x] Node color follows data/runtime type.
- [x] Ports and connections follow type color.
- [x] Compatible drag/drop opens spatial node browser.
- [x] Every persistent UI mutation still routes through commandGraph.
- [x] Proof dump shows the current skin direction.
- [x] MIT attribution is preserved if any Tooll3 code/constants are copied.

## Parking Lot

- Exact icon font parity.
- Dope sheet editing.
- Multiple graph/output windows.
- Tooll3-style variations blending.
- Live thumbnail rendering on nodes.
- Full Metal output texture embedded into ImGui.
- Animation commandGraph contract.

These are real Tooll3 skin organs, but they are not first-slice requirements.
