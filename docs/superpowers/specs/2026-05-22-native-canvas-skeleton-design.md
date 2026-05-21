# Native Canvas Skeleton Design

Date: 2026-05-22
Status: V1 shader preview proof dump implemented; S0 storage and G0 graph language proofs planned before A0/A1; A0/A1/C1 not yet built

## Purpose

`我的世界` is the new native body for 柏為的畫布.

This is not a Web wrapper and not a generic C++ port. The first skeleton must prove that a local app can host live shader editing, native audio analysis, compound patchers, and eventually AI worker collaboration without inheriting LiteGraph as the law.

## Locked Decisions

- The repo lives at `/Users/chenbaiwei/Desktop/我的世界`.
- Use JUCE + CMake for the first native app skeleton.
- Build a standalone app first, not a plugin.
- Use display name `我的世界`; keep internal target names ASCII, starting with `my-world`.
- Use OpenGL/GLSL for the first shader backend to preserve GLSL vibe-coding speed.
- Do not hardwire the app to OpenGL. Create a `RenderBackend` boundary so Metal can replace or sit beside it later.
- Old Web canvas works are migration fixtures, not the native graph law.

## First Stage Proofs

### S0 Storage Proof

Goal:

```text
work project -> atomic save -> local git commit -> module/work library export
```

Contract:

- Planned: a work is a project folder with a manifest, one or more patch documents, assets, debug/proof outputs, and a local git history.
- Planned: a patch can be a mother patch containing smaller nodes, compound patchers, and references to saved modules.
- Planned: a selected patch or compound can be published into a module repository as a reusable module package.
- Planned: large works can be stored in a work repository/library separately from the module repository.
- Planned: `Command+S` performs an atomic save of the active work project and then creates a local git commit when files changed.
- Planned: `Command+S` commits the artwork/project repository only; it must never commit this app source repository unless the user is editing this repo directly.
- Planned: auto commit is local-only. Push/sync to a remote repository must be an explicit later command.
- Planned: if save succeeds but commit fails, the UI reports `save-ok commit-failed` and leaves evidence; it must not silently claim a full save.
- Forbidden: graph state that only exists inside UI widgets, ImGui ids, or in-memory node objects.

Current storage execution plan:

```text
docs/superpowers/plans/2026-05-22-s0-storage-proof.md
```

### G0 Graph Language Contract

Goal:

```text
NodeSpec + RegionSpec + TypeSpec + StreamKind + Command schema -> typed graph IR
```

Contract:

- Planned: graph objects include nodes and regions; `NodeSpec` is not the only visible or serializable graph object.
- Planned: `RegionSpec` supports future `if`, `for_each`, `repeat`, and `while` control blocks without spaghetti node wiring.
- Planned: `TypeSpec` distinguishes values such as `audio.mono`, `signal.float`, `texture.rgba`, `geometry.mesh`, `event.midi`, `command.graph`, and future generics.
- Planned: `StreamKind` distinguishes continuous data, event streams, command streams, and resources.
- Planned: edges carry `dataType` and `streamKind`, not only `from` / `to`.
- Planned: graph validation produces a typed graph IR / AST before runtime execution or code generation.
- Planned: AI worker mutates graph state only through typed commands such as `create_node`, `create_region`, `connect`, `set_param`, `publish_module`, and `save_work`.
- Planned: future compiler workers may consume graph IR and generate GLSL, C++, validation reports, migration output, or documentation.
- Forbidden: direct AI JSON surgery.
- Forbidden: C# / .NET code in realtime audio callbacks, render hot paths, or native app lifecycle for the first stage.

Current graph language execution plan:

```text
docs/superpowers/plans/2026-05-22-g0-graph-language-contract.md
```

### A0 UI / Node Contract Guard

Goal:

```text
NodeSpec taxonomy -> Dear ImGui smoke overlay -> no JUCE Component NodeView trap
```

Contract:

- Planned: first node taxonomy registry separates `type`, `category`, `runtimeDomain`, and port `dataType`.
- Planned: seed categories include `audio`, `analyzer`, `signal`, `midi`, `shader`, `top`, `sop`, `mat`, `output`, and `compound`.
- Planned: `type` remains stable saved graph identity; `category` is registry metadata and can be extended or migrated later.
- Planned: each node spec points to a human manual; machine-readable behavior lives in `NodeSpec`, not prose.
- Planned: Dear ImGui mounts in the OpenGL render loop before any production node editor work.
- Planned: A0 may show a simple ImGui smoke panel / slider, but it must not own graph truth or graph mutation.
- Planned: production node surfaces follow vvvv-like minimal patching: compact name, pins, tiny status, doc/patch icons; values live in IOBox / Pad / Meter / Scope nodes or inspector.
- Forbidden: implementing a temporary JUCE `Component` node editor or `NodeView` that would later be replaced wholesale by ImGui.
- Forbidden: writing long explanatory sentences directly on node surfaces.

### V1 Visual Proof

Goal:

```text
native app -> hand-written Shader node -> Output preview
```

Contract:

- Proven: the app opens as a native desktop app.
- Proven: a default graph contains one Shader node connected to Output.
- Proven: the user can edit fragment shader text with the current JUCE `TextEditor` proof UI.
- Proven: compile success updates the preview.
- Proven: compile failure keeps the last valid frame and shows an error.
- Proven: the runtime can dump `frame.png`, `cook_order.json`, and `node_stats.json` for proof.

Current proof command:

```bash
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
```

Current proof output:

```text
debug/v1-shader-proof/frame.png
debug/v1-shader-proof/cook_order.json
debug/v1-shader-proof/node_stats.json
```

The V1 dump currently reads back the OpenGL framebuffer from `OpenGLShaderPreview`. This proves native render evidence, but the render backend boundary is not fully extracted yet.

### A1 Audio Proof

Goal:

```text
native preferences -> audio input -> native analyzer meter rows
```

Contract:

- Not started: preferences include audio input device, channel/mono mix, sample rate, buffer size, analysis gain, and analyzer profile.
- Not started: the realtime audio path only measures or writes bounded realtime-safe state.
- Not started: UI reads analyzer values outside the realtime callback.
- Not started: first values include at least `rms`, `peak`, and `loudness`.
- Not started: MIDI preferences are present early: output device, channel, CC map, stream on/off, and map mode.

### C1 Compound Proof

Goal:

```text
loudness compound patcher -> expanded child patchers -> collapsed public ports
```

Contract:

- Not started: `loudness` can appear as one collapsed node.
- Not started: it can expand to show child patchers such as `AudioIn`, `MonoMix`, `RMS`, `AnalysisGain`, `PreGate`, `OutputSmoother`, and `LoudnessOut`.
- Not started: the compound exposes public ports such as `loudness.out`.
- Not started: it can also expose selected inner ports such as `loudness.rms`, `loudness.peak`, `loudness.gate`, and `loudness.confidence`.

## Architecture

### Data Layers

Keep four separate truths:

```text
editorGraph        node positions, selection, expanded/collapsed state, UI metadata
runtimeGraph       executable graph for render/audio/control runtime
commandGraph       formal mutation operations used by UI, AI, importers, and scripts
collaborationLog   AI tasks, commands, diagnostics, repair attempts, and proof evidence
```

Add storage truths before the editor grows:

```text
workManifest       project identity, main patch, library refs, save policy
patchDocuments     serializable graph documents that can be reloaded headlessly
modulePackages     reusable saved mother patches / compounds with public ports
libraryIndexes     known work repositories and module repositories
saveLog            save attempts, commit ids, failures, validation evidence
```

Storage is not an export layer. It is the graph source-of-truth boundary that lets UI, runtime, AI worker, and future module libraries read the same work back.

### Graph Language

The graph is closer to a visual programming language than to a canvas of UI widgets.

First-class graph objects:

```text
Node       ordinary operation or value source
Region     visual control-flow block such as if / for_each / repeat
Edge       typed connection with dataType and streamKind
Command    validated graph mutation
Module     saved patch/compound with public ports
```

Graph flow:

```text
editorGraph
-> command validation
-> type validation
-> region boundary validation
-> graphIR / AST
-> runtimeGraph or compiler worker output
```

This allows the first runtime to remain direct C++/OpenGL while preserving a path to future compilation.

### Language / Runtime Boundary

C++ owns the first-stage app body:

```text
JUCE app shell
CoreAudio / audio callback
OpenGL / future Metal render loop
shader preview
realtime analyzer
work save and local commit
runtimeGraph hot path
```

External compiler workers are parked behind files/process boundaries:

```text
graphIR.json
commandGraph.json
validation_report.json
generated shader/source artifacts
```

C# may be useful later for graph compiler tooling, type inference experiments, module indexing, migration, documentation generation, or AI worker-side validation. It must not be embedded into realtime audio/render paths in the first stage.

### C++ Boundary

Avoid one giant class per node. Use this split:

```text
NodeSpec      static description: ports, params, widgets, compound children
NodeInstance  per-work state: values, position, exposed ports, editor flags
NodeView      generic drawing and interaction
RuntimeOp     execution logic for render/audio/control domains
Command       single mutation entrypoint
```

`NodeView` means a generic view adapter over `NodeSpec` / `NodeInstance`; it must not mean one JUCE `Component` subclass per node. The first graphics-side adapter should be Dear ImGui, proved by A0 before building production node editor interactions.

### Node Taxonomy

Use four separate labels:

```text
type           stable saved graph identity, e.g. analyzer.loudness
category       UI / browser grouping, e.g. analyzer
runtimeDomain  cook owner, e.g. audioAnalysis
dataType       port compatibility, e.g. signal.float
```

First category registry:

```text
audio
analyzer
signal
midi
shader
top
sop
mat
output
compound
```

New categories can be added later through the registry. Existing saved graph identity should move through aliases or migration, not by casually renaming `type`.

### Node Surface

The patch surface should lean closer to vvvv than to card-heavy dashboard UI.

Default visible node:

```text
name
input pins
output pins
tiny status mark
doc icon
patch-behind / expand icon when available
```

Hidden until hover, selection, inspector, or a dedicated display node:

```text
long description
usage examples
failure modes
large values
meters
previews
full parameter explanations
```

Values should be exposed through explicit nodes:

```text
IOBox
Pad
Meter
Scope
Preview
```

Human documentation and machine documentation are separate:

```text
human manual   docs/nodes/<node-type>.md
machine spec   NodeSpec registry / serialized graph schema
```

The node surface may show a `?` or small book icon that opens the human manual in an inspector. The machine spec is used by validation, node browser filtering, graph migration, AI worker commands, and runtime dispatch.

### Render Backend

Initial implementation:

```text
RenderBackend interface
OpenGLShaderPreview direct implementation
```

Minimum backend methods:

```text
compileShader
createRenderTarget
renderNode
readPixels
resize
destroy
```

Metal is parked, not rejected. `RenderBackend` exists as an interface, but `OpenGLShaderPreview` still owns compile/render/readback directly. Extracting the OpenGL implementation behind `RenderBackend` remains pending.

## Audio Analyzer Migration

Current execution plan:

```text
docs/superpowers/plans/2026-05-22-a1-audio-proof.md
```

The analyzer should borrow from:

- `/Users/chenbaiwei/Documents/GitHub/sound-in-area-analyzer-plugin`
- `/Users/chenbaiwei/Desktop/vibe coding/0512＿現場表演系統01/sound_in_area/src/analyzer`

Do not copy it as one black-box node. Split analyzer work into:

```text
Raw Facts
rms, peak, diffMean, zeroCrossHz

Feature
loudness, highMotion, zcrShape, registerEstimate, active

Detector / State
attackRaw, attackEnvelope, onsetEvent, densityEnvelope, sustainEnvelope, silenceTimer, residueEnvelope

Aggregate
globalPressure, breathiness, instability, sourceLegibility, rupture, residue

Output Shaping
gate, curve, smooth, outMin/outMax, MIDI CC, shader uniform mapping
```

First audio-visual closure:

```text
audio input -> loudness meter -> shader uniform u_loudness -> Output preview reacts
```

## AI Worker Position

AI worker is a first-class future client of the graph, not a chat box.

Initial command-loop shape:

```text
natural language task
-> command list
-> graph mutation
-> compile/render/analyze proof
-> repair loop
-> final state + evidence
```

The first skeleton does not need full AI UI, but it must not design graph mutation paths that AI cannot use later.

## Non-Goals For First Skeleton

- No full old Web project importer.
- No black velvet material demo as the first proof.
- No Metal backend implementation yet.
- No plugin build target yet.
- No complete 13-patch analyzer UI yet.
- No full AI worker UI yet.
- No production node editor UI yet; however Dear ImGui smoke proof is now required before A1 meter UI grows further.
- No temporary JUCE `Component` node editor.

## Open Questions

- Exact JUCE version / dependency acquisition method for portable builds. Current local proof references `/Users/chenbaiwei/Documents/GitHub/sound-in-area-analyzer-plugin/JUCE`.
- Whether to vendor JUCE, use CPM/FetchContent, or keep a local checkout path for early work.
- Exact first graph serialization format once `NodeSpec` is drafted.
- Exact Dear ImGui node editor library choice (`imnodes`, `imgui-node-editor`, or custom layer) after the command graph contract is less soft.

These are implementation questions, not design blockers.
