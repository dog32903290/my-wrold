# Tooll3 Interaction Borrowing Design

Date: 2026-05-22

Status: T0-T7 core interaction proof and visible ImGui canvas proof implemented and self-reviewed. Selected-object deletion is now command-backed for both edges and nodes. The visible ImGui node browser now consumes a loaded module registry sourced from the default saved `ModuleLibrary` index when creating nodes, and app proof dump records loaded compounds in runtime registry, dry-run snapshots, first executable child runtime execution snapshots, value-handoff snapshots, public-output snapshots, internal-edge source snapshots, named RuntimeOp execution snapshots, and missing RuntimeOp coverage failure snapshots. The proven layer is C++ graph, command, hit-test, storage, behavior trace logic, runtime-registry/execution evidence, and an ImGui workspace wired to that command path.

## Purpose

`我的世界` borrows Tooll3/T3's node-canvas interaction language without inheriting Tooll3's C# / SharpDX / Direct3D runtime body.

The first production node-canvas proof is:

```text
Tooll3-style gesture
-> native interaction state
-> commandGraph command
-> editorGraph mutation
-> runtimeGraph validation
-> save/debug evidence
```

If a gesture only changes visible UI and does not produce command evidence, it is not implemented.

## Source Boundaries

Allowed to borrow from Tooll3:

- Canvas navigation rhythm.
- Node selection and movement behavior.
- Connection creation, reconnection, cancellation, and deletion behavior.
- Undo/redo command vocabulary.
- Edge-case traces from Tooll3's mature interaction code.
- Visual density and panel rhythm as styling references after T0-T2 pass.

Not allowed as native law:

- C# class shape.
- SharpDX / Direct3D / HLSL assumptions.
- Tooll3 `Symbol` / `Instance` as our graph schema.
- UI-only mutation paths.
- Any copied code without preserving MIT license and copyright notice.

Primary Tooll3 witness files:

```text
Editor/Gui/Graph/GraphCanvas.cs
Editor/Gui/Graph/Interaction/ConnectionMaker.cs
Editor/Gui/Graph/Interaction/NodeOperations.cs
Editor/Gui/SymbolChildUi.cs
Editor/Gui/Commands/UndoRedoStack.cs
Editor/Gui/Commands/Graph/AddConnectionCommand.cs
Editor/Gui/Commands/Graph/DeleteConnectionCommand.cs
Editor/Gui/Commands/Graph/AddSymbolChildCommand.cs
```

## Current Native Anchors

Existing project facts this design must respect:

- `native-canvas-spine` keeps graph mutation on a command path for UI, AI, scripts, and importers.
- `GraphContract` currently separates `editorGraph` and `runtimeGraph`.
- `PatchInteraction` already maps gestures such as `zoom`, `pan`, `select`, `drag_pin_to_pin`, `undo`, and `redo` to command names.
- `NodeSpec` already defines seed node contracts, including shader, analyzer, compound, audio, and MIDI nodes.
- `StorageContract` already names work projects, patch documents, module packages, and save statuses.
- The default proof graph is `shader1.output -> out1.input`.
- C1.2 adds a module-registry `createNode` overload so a loaded module `NodeSpec` can create a compound node without relying only on `makeSeedNodeSpecs()`.
- C1.3 loads module manifests into a visible registry, merges that registry over seed specs by node type, and routes ImGui browser creation through the passed registry instead of a seed-only overload.
- C1.4 loads module manifests through a saved `ModuleLibrary` index before feeding the visible registry, so the UI command path no longer names the loudness module package directly.
- C1.5 adds a runtime registry snapshot for the same loaded module library, so proof evidence can name loaded compounds as `compound.patch` runtime entries even before full `RuntimeOp` execution exists.
- C1.6 adds a dry-run snapshot for that runtime registry, so proof evidence can show per-child cook order and status without pretending computed outputs exist yet.
- C1.7 adds a synthetic runtime execution snapshot for that runtime registry, so proof evidence can show the loaded `analyzer.rms` child as computed with `rms` and `peak` outputs while unimplemented siblings stay explicit.
- C1.8 adds multi-channel synthetic execution and per-child input/output values, so proof evidence can show `audio.mono_mix -> analyzer.rms -> analyzer.analysis_gain` value handoff in the loaded compound cook order.
- C1.9 adds loaded public output publication, so proof evidence can show all seven loudness children as computed and entry-level `publicOutputs` without relying on UI-only status.
- C1.10 adds loaded internal-edge value routing evidence, so proof dumps can show `inputSources` and `publicOutputSources` derived from saved compound routes rather than UI-only graph status.
- C1.11 adds named RuntimeOp dispatch evidence, so proof dumps can show each executed loaded child node type was selected through a RuntimeOp id rather than one branch ladder.
- C1.12 adds missing RuntimeOp coverage failure evidence, so unsupported loaded child node types fail as `missing-runtime-op` and cannot publish public outputs as false partial success.

## First Slice

The first detailed section was T0-T2. The 2026-05-22 implementation pass now proves the T0-T7 core interaction spine:

```text
T0 canvas navigation
T1 node selection / movement
T2 connection create / delete
T3 node create-and-connect
T4 compound enter / exit / collapse
T5 inspector param / port binding
T6 dirty state / storage roundtrip
T7 behavior trace compatibility suite
```

Parked:

- Production-level node editor polish beyond the current Tooll3 skin first pass.
- Insert-node-between-edge menu.
- Rich parameter widgets beyond current command-backed inspector rows.
- AI worker graph edits.
- Timeline / animation.
- Production Metal backend.

## T0-T7 Roadmap And SOP

This roadmap is the long arc. Only the current slice gets implementation-level detail. Later slices stay as contracts until earlier proof creates evidence.

### SOP Loop

Each slice follows the same loop:

```text
1. Borrow
   inspect the relevant Tooll3 behavior and name what we are borrowing

2. Contract
   write trigger, input, success, failure, and evidence

3. Trace
   add a Tooll3-style behavior trace in native terms

4. Tests First
   write failing tests for the deepest relevant validation levels

5. Native Path
   implement the smallest C++ path through commandGraph

6. Verify
   run command tests, graph invariants, save/load, and behavior traces

7. Promote
   update the spec with proven facts, remove false assumptions, and only then detail the next slice
```

No slice is accepted from visuals alone.

### Roadmap

| Slice | Name | One-line proof | Detail status | Gate to next |
| --- | --- | --- | --- | --- |
| T0 | Canvas navigation | pan/zoom -> view state -> stable hit-tests | Proven in core tests | Transform and hit-test tests pass |
| T1 | Node selection / movement | hit node -> select/move command -> persisted position | Proven in core tests | Move undo/redo and save/load pass |
| T2 | Connection create / delete | drag port -> connect/disconnect/delete_node command -> valid graph | Proven in core tests and UI command path | Connect/delete undo/redo, invariants, trace pass |
| T3 | Node search / insert | drag to empty edge/canvas -> create node + connect | Proven as create-and-connect command; UI search parked | T0-T2 command stack and port validation proven |
| T4 | Compound navigation | enter/exit/collapse/expand -> patch path + public ports | Proven as command/storage state; visual compound UI parked | T3 can create/connect nodes without UI-only state |
| T5 | Inspector / parameter binding | edit value/connect override -> param or port binding command | Proven as command/storage state; inspector widgets parked | T4 proves patch path and selected node identity |
| T6 | Global undo / redo / dirty state | user action group -> command stack -> dirty/save status | Proven in command and storage tests | T0-T5 commands have stable Do/Undo/Redo semantics |
| T7 | Behavior trace compatibility suite | Tooll3 traces -> replay runner -> graph and storage evidence | Proven with current 8-trace fixture | Enough T0-T6 traces exist to justify a suite |

### Detail Promotion Rule

A roadmap slice can be promoted to detailed design only when:

```text
all previous slice gates pass
missing graph/storage fields are known from tests
the operation can be expressed as commandGraph commands
the save/load effect is known or explicitly none
```

This prevents T3-T7 from becoming imagined law before T0-T2 reveals the real graph shape.

### Parked Slice Notes

T3 node search / insert:

```text
drag from port to empty canvas
-> open node search filtered by compatible port type
-> create node
-> connect source to new node or new node to target
```

T4 compound navigation:

```text
enter compound
-> current patch path changes
-> inner graph appears
-> exit returns to parent with compound selected
-> collapse/expand preserves public port mapping
```

T5 inspector / parameter binding:

```text
select node
-> inspector shows params from NodeSpec
-> manual edit creates set_param
-> connected input creates set_port_binding
```

T6 global undo / redo / dirty state:

```text
each user gesture creates one undoable action
macro gestures undo as one step
dirty state follows command stack and save result
```

T7 behavior trace compatibility suite:

```text
trace JSON
-> replay simulated gestures
-> assert command log
-> assert editorGraph/runtimeGraph/storage evidence
```

## Interaction Model

The native UI must use this split:

```text
NodeSpec       node type, ports, params, runtime domain
NodeInstance   id, type, persistent editor state, position, collapsed state
NodeView       hit-test and drawing data only
Interaction    transient mouse/keyboard state
Command        only persistent mutation path
GraphValidator graph invariant gate after command
```

`NodeView` must not own graph truth. Any state that survives save/load belongs in `NodeInstance` or a patch-level editor state document.

## T0 Canvas Navigation

Goal:

```text
mouse / trackpad pan + zoom -> CanvasViewState -> hit-tests remain stable
```

Borrowed Tooll3 behavior:

- A graph window has a persistent scope: scale and scroll.
- Jumping between graph scopes preserves understandable focus.
- Fit-view behavior is a view command, not graph data mutation.

Native command:

```text
set_view
```

Contract:

| Question | Answer |
| --- | --- |
| Trigger | Wheel/trackpad zoom, pan gesture, frame selection command, app restoring saved view. |
| Input | Current `CanvasViewState`, pointer delta or zoom delta, viewport size, optional focus point. |
| Success | Updates view scale/scroll; graph data is unchanged; subsequent hit-tests use the new transform. |
| Failure | Invalid scale, invalid viewport, or non-finite coordinates reject the view update and leave view unchanged. |
| Evidence | Command log entry, transform unit tests, optional debug readout of scale/scroll. |

Validation depth:

- L2 required: screen/canvas transform roundtrip, pan stability, zoom-around-focus.
- L4 required for `set_view` if view state becomes undoable or persistent.
- L6 required once view state is saved in patch editor state.

First tests:

```text
screenToCanvas(canvasToScreen(point)) == point
zoom around cursor keeps cursor's canvas point stable
pan changes view but leaves graph nodes/edges unchanged
fit view emits set_view and does not mutate editorGraph
```

## T1 Node Selection And Movement

Goal:

```text
pointer hit-test -> select node -> drag selected nodes -> MoveNodeCommand -> persistent node positions
```

Borrowed Tooll3 behavior:

- Nodes have persistent canvas positions and sizes.
- Selection is separate from graph structure.
- Movement is undoable.
- Multiple selected nodes move together.

Native commands:

```text
select
move_node
```

Command vocabulary status:

```text
select     already exists in GraphLanguage
move_node  must be added to GraphLanguage before T1 implementation
```

Contract:

| Question | Answer |
| --- | --- |
| Trigger | Pointer down on node body, shift-click/additive selection, box selection, drag selected node. |
| Input | Hit-test result, selection modifier state, node ids, start positions, drag delta in canvas coordinates. |
| Success | Selection changes through `select`; movement commits through `move_node`; edge endpoints follow node positions. |
| Failure | Missing node id, non-finite delta, or drag on locked/nonexistent node leaves graph unchanged and logs rejection. |
| Evidence | Command log, before/after editorGraph positions, undo/redo test, save/load roundtrip. |

Selection persistence:

- Selection is editor UI state, not runtimeGraph data.
- A selected path can be promoted to editor restore state in a later slice, but it must not affect cook order or runtime output.

Movement persistence:

- Node position belongs to `editorGraph`.
- Runtime execution may ignore position, but debug output must be able to report the editor position for evidence.

Validation depth:

- L2 required: node body hit-test under pan/zoom, box selection hit-test.
- L3 required: `idle -> selecting` and `idle -> draggingNode -> commitMove / cancelMove`.
- L4 required: `select` and `move_node`.
- L5 required after `move_node`.
- L6 required for node positions.

First tests:

```text
hit-test finds Shader node body at default position
hit-test still finds Shader node after pan/zoom transform
drag Shader by (40, 20) emits move_node
undo move restores original position
save/load preserves moved position
edge endpoint follows moved Shader node
```

## T2 Connection Create And Delete

Goal:

```text
drag output port to input port -> AddConnectionCommand
delete selected edge -> DeleteConnectionCommand
delete selected node -> DeleteNodeCommand
```

Borrowed Tooll3 behavior:

- A temporary connection remembers source/target and value type while dragging.
- Valid targets highlight only when port type and cardinality match.
- Dragging from an already-connected single input detaches the old edge and starts reconnection.
- Cancelled drags leave graph unchanged.
- Connection add/delete and selected node delete go through undoable commands.

Native commands:

```text
connect
delete_node
disconnect
reconnect
```

Command vocabulary status:

```text
connect     proven in GraphLanguage
delete_node proven in GraphLanguage
disconnect  proven in GraphLanguage
reconnect   may lower to disconnect+connect internally, but the user gesture must still be traceable as reconnect
```

Contract:

| Question | Answer |
| --- | --- |
| Trigger | Pointer down on output port, drag to input port, pointer up; pointer down on connected input to detach; delete selected edge; Delete key/button on selected node. |
| Input | Source node/port, target node/port, port data type, stream kind, input cardinality, existing edges. |
| Success | Valid connection mutates editorGraph and regenerated runtimeGraph; edge delete removes edge; node delete removes node plus incident edges; undo/redo restores graph states. |
| Failure | Type mismatch, missing port, duplicate edge, cardinality violation, or cycle rule violation rejects command and leaves graph unchanged. |
| Evidence | Tooll3 behavior trace, command log, graph invariant result, runtimeGraph edge count, save/load roundtrip. |

Type and cardinality:

- `texture.rgba` may connect from `shader.fragment.output` to `output.preview.input`.
- Single input ports accept one incoming edge unless NodeSpec says multi-input.
- Port compatibility must be checked before command commit and again in graph invariants.

Temporary state:

```text
idle
-> draggingConnection(sourcePort)
-> hoverCandidate(targetPort)
-> commitConnection / cancelConnection
```

Delete state:

```text
idle
-> edgeSelected(edgeId)
-> deletePressed
-> disconnect

idle
-> nodeSelected(nodeId)
-> deletePressed
-> delete_node
```

Reconnect state:

```text
connectedInput
-> detachExistingEdge
-> draggingConnection(existingSource)
-> commitReconnect / cancelReconnect
```

Cancellation rule:

- If a new connection drag is cancelled, no graph mutation happens.
- If reconnect detaches an existing edge visually, cancellation restores the previous edge.
- If implementation chooses to execute detach immediately, it must wrap detach+reconnect in a macro command with correct undo semantics.

Validation depth:

- L1 required: both ports must exist in NodeSpec / graph contract.
- L2 required: port hit-test and edge hit-test.
- L3 required: connection drag, cancel, delete, reconnect state machines.
- L4 required: `connect`, `disconnect`, `reconnect`, undo/redo.
- L5 required after every connection command.
- L6 required for edges.
- L7 required with Tooll3-style traces.

First tests:

```text
drag Shader.output to Output.input emits connect
connect adds one editorGraph edge and one runtimeGraph edge
delete selected edge emits disconnect
delete selected node emits delete_node and removes incident edges
undo disconnect restores edge
undo delete_node restores node and incident edges
type mismatch rejects command and leaves graph unchanged
cancelled connection drag leaves graph unchanged
save/load preserves connected graph
```

## Graph Invariants

Run after each mutation command in T0-T2:

```text
node ids are unique
edge ids are unique
edge source node exists
edge target node exists
edge source port exists
edge target port exists
source is output, target is input
port data types are compatible
input cardinality is respected
runtimeGraph can be regenerated from editorGraph
```

View-only commands such as `set_view` may skip runtimeGraph regeneration, but they must still reject invalid view state.

## Behavior Traces

The first trace file describes Tooll3-style behavior in our terms. V1 fixture path:

```text
fixtures/interaction/tooll3-t0-t7.behavior.json
```

Minimum trace:

```json
{
  "version": 1,
  "source": "Tooll3 interaction contract, not copied code",
  "initialGraph": "fixtures/storage/minimal-work/patches/main.patch.json",
  "traces": [
    {
      "name": "move shader node",
      "events": [
        "mouseDown node:shader1.body",
        "mouseMove canvasDelta:40,20",
        "mouseUp"
      ],
      "expectedCommands": ["select", "move_node"],
      "expectedPersistentState": ["editorGraph.nodes.shader1.position"]
    },
    {
      "name": "connect shader to output",
      "events": [
        "mouseDown port:shader1.output",
        "mouseMove port:out1.input",
        "mouseUp port:out1.input"
      ],
      "expectedCommands": ["connect"],
      "expectedEditorGraph": ["edge shader1.output -> out1.input"],
      "expectedRuntimeGraph": ["edge shader1.output -> out1.input"],
      "expectedUndo": ["edge removed"]
    }
  ]
}
```

This v1 trace schema is scoped to the T0-T7 command proof. If a later interaction needs fields that cannot be represented here, add a schema version and a migration test instead of silently changing existing traces.

## Test Plan

Implementation started with failing tests and now passes.

Proposed new tests:

```text
tests/CanvasViewStateTests.cpp
tests/NodeHitTestTests.cpp
tests/GraphCommandTests.cpp
tests/GraphInvariantTests.cpp
tests/InteractionTraceTests.cpp
tests/InteractionStorageRoundtripTests.cpp
```

Test order:

1. L2 transform and hit-test tests.
2. L4 command Do/Undo/Redo tests.
3. L5 graph invariant tests.
4. L7 behavior trace parser / runner tests.
5. L6 storage roundtrip tests.

UI drawing can come after command tests pass. This prevents visual code from becoming the source of truth.

## Implementation Evidence

Implemented core proof files:

```text
source/core/InteractionContract.h
source/core/InteractionContract.cpp
source/render/OpenGLShaderPreview.h
source/render/OpenGLShaderPreview.cpp
source/ui/ImGuiSmokeOverlay.h
source/ui/ImGuiSmokeOverlay.cpp
fixtures/interaction/tooll3-t0-t7.behavior.json
tests/CanvasViewStateTests.cpp
tests/GraphCommandTests.cpp
tests/GraphInvariantTests.cpp
tests/NodeHitTestTests.cpp
tests/T3T5CommandTests.cpp
tests/InteractionStorageRoundtripTests.cpp
tests/InteractionTraceTests.cpp
```

Final proof commands:

```bash
cmake --build build
./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit
ctest --test-dir build --output-on-failure
```

Final result:

```text
17/17 tests passed
```

Visible gesture status:

```text
empty-canvas drag -> pan view state
mouse wheel over canvas -> zoom view state
node body drag -> move_node command
output port drag -> temporary connection state
release on input port -> connect command
release on empty canvas -> compatible node popup
select popup candidate -> create_node+connect command
edge click -> selected edge state
Disconnect with selected edge -> disconnect command
Delete with selected node -> delete_node command and incident-edge cleanup
Add Loudness -> create compound.loudness command
selected compound Enter/Exit -> patch path command
selected compound Collapse/Expand -> collapsed editor state command
selected node Inspector -> set_param and set_port_binding commands
Save State -> mark clean and serialize interaction state
Reload State -> deserialize saved interaction state
Run Trace -> replay Tooll3 T0-T7 behavior fixture in-app
```

## Implementation Boundaries

Allowed first implementation objects:

```text
CanvasViewState
CanvasTransform
NodeBounds
PortBounds
HitTestResult
InteractionState
GraphCommand
CommandStack
GraphInvariantReport
```

Not allowed in first implementation:

```text
custom node styling system
full ImGui node editor library commitment
compound editor UI
parameter inspector
AI worker UI
Metal backend coupling
direct JSON mutation from UI
```

## Required Contract Changes Before Code

Before implementing T0-T2, the implementation plan must add or update contracts for:

```text
move_node command type
disconnect command type
NodeInstance editor position
edge id
port metadata for hit-test and validation
CanvasViewState if view persistence is included
behavior trace fixture format
```

`reconnect` may remain a gesture-level trace that lowers to `disconnect + connect`, but only if undo/redo treats the whole reconnect as one user action.

## Acceptance

The slice is accepted only when all are true:

- Default Shader and Output nodes can be represented as node views derived from graph data.
- Pan and zoom keep hit-tests stable.
- Shader node can be selected and moved through commands.
- Shader output can connect to Output input through commands.
- Connection can be deleted through commands.
- Undo/redo works for move/connect/disconnect.
- Graph invariants pass after each command.
- Save/load preserves node positions and edges.
- Tooll3 behavior trace exists and is run by a test.

## Self-Review

Placeholder scan:

- No placeholder markers remain.
- Parked scope is explicit.

Internal consistency:

- Tooll3 is a behavior witness, not a codebase to fork.
- T0 is view state; T1 and T2 are graph/editor state.
- UI drawing is downstream from commands and graph data.

Scope check:

- The detailed written contract remains deepest for T0-T2.
- T3-T7 are implemented as core command/storage/trace proof, and the current ImGui workspace proves the first visible operation surface. Production-level polish, AI worker edits, timeline editing, and Metal rendering are still parked.

Ambiguity check:

- Reconnect cancellation has an explicit rule.
- View-only commands are separated from graph mutation commands.
- Selection is editor UI state unless later promoted to persistent restore state.
- Missing command vocabulary is called out as required contract work before code.

Resolved facts and remaining risk:

- `GraphContract` now stores node positions, editor edges, edge ids, and port metadata needed by the interaction proof.
- Interaction-state roundtrip now proves positions, edges, collapsed state, current patch path, dirty status, params, and port bindings.
- The exact production ImGui node rendering layer is still not selected; this remains deferred because command and hit-test tests now carry the first weight.

Verdict:

```text
Spec and T0-T7 core proof are coherent.
The next production canvas pass can build drawing and gesture state machines on top of command/invariant tests, not inside UI-only state.
```
