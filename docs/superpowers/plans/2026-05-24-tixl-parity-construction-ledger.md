# TiXL Parity Construction Ledger Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Maintain one progress-control ledger for TiXL-style user-facing parity so every visible feature has a spec row, status, phase, witness, and next proof trace.

**Architecture:** Specs define correctness; this ledger defines sequencing and progress. Each row points back to a spec, a TiXL witness surface, and an acceptance trace before implementation. Native graph identity, commandGraph, RuntimeOp coverage, and proof dumps remain My World law.

**Tech Stack:** Markdown plan ledger, existing TiXL witness catalog JSON, C++20/CMake proof binaries, behavior trace fixtures, `git diff --check`.

---

## Operating Rules

- This ledger is not a second spec. If a row changes the required behavior, update the linked spec first.
- Every user-visible TiXL parity feature must have one row here or in a downstream plan linked from this ledger.
- A parked row is still captured scope. It is not a promise to implement it in the current phase.
- A row can move to `proven` only after a test, fixture, proof dump, or behavior trace names the evidence.
- A user gesture that mutates graph state must route through commandGraph and undo/redo evidence.
- TiXL `Operators/Lib` taxonomy is user-facing browser law. Saved graph `type` remains native My World law.

## File Structure

- Create: `docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md`
  - Owns progress rows, phases, immediate queue, and downstream plan order.
- Modify: `docs/superpowers/specs/2026-05-24-tixl-taxonomy-parity-spec.md`
  - Link this ledger as the progress-control surface.
- Future P-TAX1 artifacts:
  - Create: `fixtures/tixl-witness/operator-browser-taxonomy.json`
  - Create: `tests/TiXLTaxonomyFixtureTests.cpp`
  - Modify: `CMakeLists.txt`
- P-SEARCH1 artifacts:
  - Modify or create: `source/core/NodeSpecBrowser.h/.cpp`
  - Create: `tests/NodeSpecBrowserTests.cpp`
  - Create: `fixtures/interaction/tixl-search-compatible-create.behavior.json`
- Future P-OPS1 artifacts:
  - Modify: `source/core/InteractionContract.h/.cpp`
  - Modify: `fixtures/interaction/tooll3-t0-t7.behavior.json`
  - Create or extend: `tests/InteractionTraceTests.cpp`
- Future P-OUT1 artifacts:
  - Modify or create: `source/core/OutputViewState.h/.cpp`
  - Create: `tests/OutputViewStateTests.cpp`
  - Modify: `source/ui/ImGuiSmokeOverlay.cpp`

## Status And Phase Vocabulary

Status:

```text
mirror   user-facing label/path/operation should match TiXL
adapt    user-facing intent matches, native mechanics differ; reason required
proven   covered by current tests, fixtures, or proof dump
partial  shell or core exists, but TiXL parity trace is incomplete
planned  accepted near-term construction row
parked   explicit requirement, blocked by a named proof line
reject   explicitly not part of My World; reason required
```

Phase:

```text
L0 law       spec rule only
L1 witness   catalog, fixture, source audit, or trace file
L2 command   commandGraph, graph invariant, undo/redo, save/load proof
L3 visible   UI/browser/inspector/output surface reads proof-backed state
L4 live      runtime, audio, timeline, render/export, or performance proof
```

## Master Ledger

### Source And Taxonomy

| ID | Feature | Spec row | Witness | Status | Phase | Acceptance trace | Blocker / next proof |
| --- | --- | --- | --- | --- | --- | --- | --- |
| WIT-001 | TiXL `Operators/Lib` witness catalog | node function spec catalog rule | `fixtures/tixl-witness/operator-catalog.json` | proven | L1 witness | catalog counts and source commit recorded | none |
| TAX-001 | Default browser root mirrors TiXL visible roots | taxonomy default table | `fixtures/tixl-witness/operator-browser-taxonomy.json` | proven | L1 witness | `category_browser_root_exact_tixl_paths` | none |
| TAX-002 | TiXL child paths mirror visible taxonomy | taxonomy default table and third-level paths | `fixtures/tixl-witness/operator-browser-taxonomy.json` | proven | L1 witness | `category_browser_drilldown_exact_namespace` | none |
| TAX-003 | Hidden/internal/resource paths excluded from default browser | hidden paths table | `fixtures/tixl-witness/operator-browser-taxonomy.json` | proven | L1 witness | hidden/default fixture assertions | none |
| TAX-004 | `top.*`, `sop.*`, `mat.*` are aliases, not roots | non-negotiable rules | TiXL taxonomy plus local NodeSpec aliases | proven | L1 witness | alias search returns native saved type | `node_spec_browser` proves alias search preserves native saved type; UI browser polish remains separate |
| TAX-005 | 923 operator catalog remains witness, not backlog | node function spec catalog rule | `operator-catalog.json` | proven | L0 law | parked catalog entries do not create runtime rows | none |

### Browser, Search, And Compatible Create

| ID | Feature | Spec row | Witness | Status | Phase | Acceptance trace | Blocker / next proof |
| --- | --- | --- | --- | --- | --- | --- | --- |
| BROW-001 | Empty browser curated root page | Browser And Search Parity | `SymbolBrowsing.UpdateLibPage` | planned | L3 visible | `category_browser_root_exact_tixl_paths` | P-TAX1 fixture |
| BROW-002 | Category drilldown by namespace path | Browser And Search Parity | `SymbolBrowsing` path mutation | planned | L3 visible | `category_browser_drilldown_exact_namespace` | P-TAX1 fixture |
| BROW-003 | Essential overview rows | Browser And Search Parity | `SymbolTags.Essential` | parked | L3 visible | `category_overview_essential_only` | NodeSpec metadata flag |
| BROW-004 | Full symbol library namespace tree | Browser And Search Parity | `SymbolLibrary`, `NamespaceTreeNode` | parked | L3 visible | `symbol_library_namespace_tree_root_order` | secondary library UI |
| SEARCH-001 | Fuzzy search across name, namespace, description | Browser And Search Parity | `SymbolFilter` | proven | L1 witness | `search_fuzzy_name_namespace_description` | `node_spec_browser` proves query helper behavior; UI event wiring remains separate |
| SEARCH-002 | Search ranking exact, starts-with, contains, PascalCase | Browser And Search Parity | `SymbolFilter` ranking | proven | L1 witness | `search_ranking_exact_starts_contains_pascal` | `node_spec_browser` proves deterministic fixture order |
| COMPAT-001 | Drag output to empty canvas suggests compatible nodes | Browser And Search Parity | `PlaceHolderUi`, `SymbolFilter` | proven | L1 witness | `compatible_create_from_output_drag` | `node_spec_browser` proves candidate filtering; mutation still uses existing `create_node+connect` command path |
| COMPAT-002 | Drag input to empty canvas suggests compatible nodes | Browser And Search Parity | `PlaceHolderUi`, `SymbolFilter` | proven | L1 witness | `compatible_create_from_input_drag` | `node_spec_browser` proves candidate filtering; UI gesture trace remains separate |
| COMPAT-003 | Split existing connection through browser | Browser And Search Parity | legacy `ConnectionMaker.SplitConnectionWithSymbolBrowser` | partial | L2 command | `compatible_split_connection_insert_node` | P-OPS1A proves split macro command; visible browser trigger remains |
| COMPAT-004 | Keyboard create/cancel in browser | Browser And Search Parity | browser keyboard handling | planned | L3 visible | `browser_keyboard_return_escape_no_mutation_on_cancel` | UI event trace schema |

### Graph Interaction

| ID | Feature | Spec row | Witness | Status | Phase | Acceptance trace | Blocker / next proof |
| --- | --- | --- | --- | --- | --- | --- | --- |
| GEST-001 | Canvas pan, wheel zoom, focus-preserving zoom | Graph Interaction Parity | `ScalableCanvas` | proven | L2 command | existing canvas transform tests | none |
| GEST-002 | Single node select and move | Graph Interaction Parity | `GraphStates`, `MagItemMovement` | proven | L2 command | existing `move_node` trace | none |
| GEST-003 | Multi-select, fence, group move | Graph Interaction Parity | selected item movement | partial | L2 command | `multi_select_group_move_undo_save` | selection set command model |
| GEST-004 | Port-to-port connect | Graph Interaction Parity | `InputSnapper`, `OutputSnapper` | proven | L2 command | existing `connect` trace | none |
| GEST-005 | Selected edge delete | Graph Interaction Parity | `Modifications.DeleteSelection` | proven | L2 command | existing `disconnect` trace | none |
| GEST-006 | Selected node delete plus incident edges | Graph Interaction Parity | `DeleteSymbolChildrenCommand` | proven | L2 command | existing `delete_node` trace | none |
| GEST-007 | Reconnect existing input end | Graph Interaction Parity | `HoldingConnectionEnd`, `InputSnapper` | proven | L2 command | `reconnect_input_end_one_undo_step` | `reconnect` macro command trace proven; visible drag state remains separate |
| GEST-008 | Reconnect existing output beginning | Graph Interaction Parity | `HoldingConnectionBeginning`, `OutputSnapper` | proven | L2 command | `reconnect_output_beginning_one_undo_step` | `reconnect` macro command trace proven; visible drag state remains separate |
| GEST-009 | Drop connection/node onto operator body and choose hidden input | Graph Interaction Parity | `InputPicking` | proven | L2 command | `drop_connection_onto_operator_hidden_input` | `connect_hidden_input` trace proves explicit input command; visible picker UI remains separate |
| GEST-010 | Multi-input insert before, after, replace | Graph Interaction Parity | `InputSnapper.InputSnapTypes` | proven | L2 command | `multi_input_insert_before_after_replace` | `multi_input_insert` proves fixed ordered slots; variadic/unbounded lists remain parked |
| GEST-011 | Split edge by creating operator | Graph Interaction Parity | `ConnectionMaker.SplitConnectionWithSymbolBrowser` | proven | L2 command | `split_edge_create_operator_undo_macro` | `split_edge_create_node` macro trace proven; browser UI trigger remains separate |
| GEST-012 | Drag existing node onto edge to insert | Graph Interaction Parity | `MagItemMovement.TrySplitInsert` | proven | L2 command | `drag_node_onto_edge_split_insert` | `insert_node_on_edge` trace proves macro mutation; visible edge hit-test remains separate |
| GEST-013 | Snap move creates connection and unsnap removes connection | Graph Interaction Parity | `MagItemMovement` snap/unsnap | proven | L2 command | `snap_move_connect_unsnap_disconnect_undo` | `snap_connect` / `unsnap_disconnect` traces prove committed graph mutation; preview state remains UI layer |
| GEST-014 | Shake dragged node to disconnect | Graph Interaction Parity | `ShakeDetector` | proven | L2 command | `shake_disconnect_dragged_node_edges` | `shake_disconnect` trace proves command mutation; gesture detection threshold remains UI layer |
| GEST-015 | Enter, exit, collapse, expand compound | Graph Interaction Parity | graph navigation state | proven for compounds | L2 command | existing compound traces | public port persistence line continues |
| GEST-016 | Inspector param and port binding | Graph Interaction Parity | graph input commands | proven | L2 command | `reset_default_manual_binding_fallback` | `set_param`, `reset_param`, `set_port_binding`, and `reset_port_binding` command path proven; extract-value node remains PARAM-008 |
| GEST-017 | Annotation add, drag, resize, rename, delete, collapse | Graph Interaction Parity | annotation interaction files | parked | L3 visible | `annotation_frame_move_resize_rename_collapse_undo` | annotation scope decision |
| GEST-018 | Symbol, file, or asset drop creates graph item | Graph Interaction Parity | `DropHandling` | parked | L3 visible | `drop_symbol_creates_node_undo` | asset browser boundary |

### Parameters, Presets, Snapshots, Variations

| ID | Feature | Spec row | Witness | Status | Phase | Acceptance trace | Blocker / next proof |
| --- | --- | --- | --- | --- | --- | --- | --- |
| PARAM-001 | Parameter inspector shell | Parameter, Preset, And Snapshot Parity | `ParameterWindow`, `IInputUi` | partial | L3 visible | `select_node_inspector_rows_set_param` | row state and typed core controls proven; richer layout/metadata remains PARAM-007 |
| PARAM-002 | Row states: normal, connected, animated, default/reset | Parameter parity | input UI state | proven | L3 visible | `default_manual_connected_animated_undo` | none for current row-state layer |
| PARAM-003 | Type color families | Parameter parity | `TypeUiProperties` | partial | L3 visible | `port_type_family_visual_mapping` | TypeSpec color map |
| PARAM-004 | Scalar/vector controls | Parameter parity | typed input UIs | proven | L3 visible | `typed_scalar_vector_param_roundtrip` | dedicated vector UI polish remains later |
| PARAM-005 | Enum, string, path, multiline controls | Parameter parity | typed input UIs | proven | L3 visible | `typed_text_enum_path_param_roundtrip` | enum flags and native file picker remain parked |
| PARAM-006 | Lists, curves, gradients, ADSR | Parameter parity | specialized input UIs | parked | L3 visible | `list_curve_gradient_adsr_roundtrip` | curve/gradient value types |
| PARAM-007 | Parameter metadata: groups, relevancy, descriptions, exclude from presets | Parameter parity | input metadata | planned | L3 visible | `nodespec_parameter_metadata_visibility` | NodeSpec metadata extension |
| PARAM-008 | Input operations menu | Parameter parity | input context menu | partial | L2 command | `reset_param_set_default_extract_value_node` | command verbs for extract/reset |
| VAR-001 | Presets and snapshots are separate concepts | Variation parity | `VariationsWindow` | shell only | L3 visible | `presets_vs_snapshots_context_switch` | variation state model |
| VAR-002 | Preset capture and apply | Variation parity | `SymbolVariationPool` | planned | L2 command | `create_apply_preset_capture_report` | capture skip reasons |
| VAR-003 | Snapshot capture and apply | Variation parity | snapshot-enabled children | planned | L2 command | `create_apply_snapshot_enabled_children` | child enable flags |
| VAR-004 | Variation canvas CRUD | Variation parity | `VariationBaseCanvas` | planned | L2 command | `variation_thumbnail_crud_undo` | commandGraph verbs |
| VAR-005 | Hover preview and Alt blend | Variation parity | `ValueUtils` blend rules | parked | L3 visible | `deterministic_variation_blend_preview_commit_cancel` | deterministic blend rules |
| VAR-006 | Presets in symbol browser | Variation parity | browser preset creation | parked | L3 visible | `drag_pin_choose_node_preset_create_apply_connect` | browser plus variation bridge |

### Output, Timeline, Render, Live IO

| ID | Feature | Spec row | Witness | Status | Phase | Acceptance trace | Blocker / next proof |
| --- | --- | --- | --- | --- | --- | --- | --- |
| OUT-001 | Output follows selection unless pinned | Output parity | `ViewSelectionPinning` | proven | L3 visible | `select_changes_output_pin_freezes_output_reload` | P-OUT1 closed; output slot/final-eval pin remains OUT-002 |
| OUT-002 | Output slot and final eval start pin | Output parity | output id/eval-start pin | parked | L4 live | `multi_output_view_pin_final_eval_pin` | multi-output runtimeGraph |
| OUT-003 | Image canvas Fit, 1:1, Custom pan/zoom, overlay | Output parity | `ImageOutputCanvas` | partial | L3 visible | `output_fit_1to1_custom_view_state` | RenderBackend extraction |
| OUT-004 | Output toolbar screenshot and render settings vocabulary | Output parity | output window toolbar | parked mixed | L3 visible | `screenshot_or_frame_dump_active_output` | active output view state |
| OUT-005 | Resolution presets | Output parity | output settings | parked | L3 visible | `requested_resolution_preset_roundtrip` | output state model |
| OUT-006 | Render/export window | Output parity | render export files | parked | L4 live | `render_settings_roundtrip_frame_count` | RenderBackend plus timeline |
| OUT-007 | Render process states | Output parity | render state model | parked | L4 live | `invalid_output_blocks_render_state` | active output type system |
| TIME-001 | Bars are canonical timeline truth; seconds/frames are views | Timeline parity | timeline files | proven | L2 command | `bars_seconds_frames_conversion_bpm_fps` | P-TIME1 closed; transport/render/export remain separate |
| TIME-002 | Playback controls and IO indicator | Timeline parity | transport controls | proven for transport controls | L4 live | `transport_play_loop_io_indicator` | P-TIME2 closed for play/pause/stop/step/loop; IO bus indicator remains parked |
| TIME-003 | Keyframes and curves | Timeline parity | animation commands | parked | L2 command | `keyframe_curve_undo_redo_exact` | animation data model |
| TIME-004 | Time clips and time warp | Timeline parity | time clip files | parked | L2 command | `time_clip_retime_no_overlap` | timeline phase |
| LIVE-001 | Composition audio source | Live parity | audio settings, playback source | partial | L4 live | `audio_input_to_meter_to_uniform` | live sample-window runner |
| LIVE-002 | Global audio mixers | Live parity | mixer settings | parked | L4 live | `audio_mixer_mute_route` | audio graph/mix bus |
| LIVE-003 | MIDI input taxonomy and teach | Live parity | MIDI input UI | partial | L4 live | `teach_midi_cc_binding_flash` | MIDI input manager |
| LIVE-004 | MIDI output taxonomy | Live parity | MIDI output operators | partial | L4 live | `loudness_cc_output_stream_enabled` | live performance graph |
| LIVE-005 | OSC input | Live parity | OSC files | parked | L4 live | `osc_address_to_signal_value` | IO event bus |
| LIVE-006 | Audio analyzer/operator family | Live parity | `io/audio`, `AudioReaction`, `DetectBpm` | partial/proven loudness | L4 live | `loaded_loudness_outputs_drive_live_surface` | C1.19/C1.20 line |
| LIVE-007 | Exported executable and live show controls | Live parity | executable settings | parked | L4 live | `exported_show_keyboard_playback` | packaging/runtime mode |

### Native Carrying Lines

| ID | Feature | Spec row | Witness | Status | Phase | Acceptance trace | Blocker / next proof |
| --- | --- | --- | --- | --- | --- | --- | --- |
| NATIVE-001 | V1 shader preview proof | skeleton design V1 | local OpenGL shader preview | proven | L4 live | `frame.png`, `cook_order.json`, `node_stats.json` | RenderBackend extraction parked |
| NATIVE-002 | A1 audio proof | skeleton design A1 | local audio analyzer | proven/partial | L4 live | audio proof dumps and `u_loudness` uniform | live sample-window runner |
| NATIVE-003 | C1 loaded loudness compound runtime | skeleton design C1 | module fixtures and RuntimeRegistry | partial/proven through C1.20 | L4 live | loudness runtime execution and bridge dumps | current public-port persistence/runtime bridge lines |
| NATIVE-004 | AI worker commandGraph loop | skeleton design AI worker | local commandGraph contract | parked | L2 command | `ai_worker_create_repair_proof_loop` | command vocabulary and proof runner |

## Immediate Work Queue

### P-TAX1 Category Browser Fixture

Claim:

```text
TiXL visible Operators/Lib taxonomy can be converted into a deterministic My World browser taxonomy fixture without exposing hidden/resource paths.
```

Evidence target:

```text
fixtures/tixl-witness/operator-browser-taxonomy.json
tests/TiXLTaxonomyFixtureTests.cpp
```

- [x] Read `fixtures/tixl-witness/operator-catalog.json` and the taxonomy parity spec default/hidden path tables.
- [x] Write `tests/TiXLTaxonomyFixtureTests.cpp` with assertions for root paths, selected child paths, hidden paths, source commit, and deterministic order.
- [x] Create `fixtures/tixl-witness/operator-browser-taxonomy.json` with root paths, visible children, hidden path patterns, and source metadata.
- [x] Add the test target to `CMakeLists.txt`.
- [x] Run `cmake --build build --target my_world_tixl_taxonomy_fixture_tests`.
- [x] Run `./build/my_world_tixl_taxonomy_fixture_tests`.
- [x] Update taxonomy parity spec rows `TAX-001` through `TAX-003` from `planned` to `proven` after the test passes.

Closed as of 2026-05-25 02:16 Asia/Taipei.

Verification:

```text
cmake -S . -B build
cmake --build build --target my_world_tixl_taxonomy_fixture_tests
ctest --test-dir build --output-on-failure -R tixl_taxonomy_fixture
./build/my_world_tixl_taxonomy_fixture_tests
```

Accepted result:

```text
tixl_taxonomy_fixture passed.
```

### P-SEARCH1 Search And Compatible Create Fixture

Claim:

```text
The native NodeSpec browser can search TiXL-style labels/aliases and filter compatible create candidates from either dragged output or dragged input context.
```

Evidence target:

```text
source/core/NodeSpecBrowser.h/.cpp
tests/NodeSpecBrowserTests.cpp
fixtures/interaction/tixl-search-compatible-create.behavior.json
```

- [x] Define `BrowserNodeEntry` from `NodeSpec`, visible taxonomy path, aliases, description, ports, runtime readiness, and hidden/default state.
- [x] Write search tests for exact, starts-with, contains, PascalCase, namespace, description, `top.*` alias, and native saved type preservation.
- [x] Write compatible-create tests for output context and input context using existing seed specs plus loaded module specs.
- [x] Implement the smallest deterministic query helper without coupling it to ImGui.
- [x] Add behavior fixture rows for `compatible_create_from_output_drag`, `compatible_create_from_input_drag`, and cancel-without-mutation.
- [x] Run `cmake --build build`.
- [x] Run `./build/my_world_node_spec_browser_tests` and `./build/my_world_interaction_trace_tests`.

Closed as of 2026-05-25 02:27 Asia/Taipei.

Verification:

```text
cmake -S . -B build && cmake --build build --target my_world_node_spec_browser_tests
# RED first failed on missing source/core/NodeSpecBrowser.h.
cmake --build build --target my_world_node_spec_browser_tests
./build/my_world_node_spec_browser_tests
./build/my_world_interaction_trace_tests
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted result:

```text
node spec browser tests ok
interaction traces ok
55/55 tests passed.
```

Scope boundary:

```text
P-SEARCH1 proves deterministic core browser/search/compatible-create candidate logic.
It does not implement the visible ImGui browser, keyboard navigation, or split-connection macro command.
```

### P-OPS1 Richer Graph Operation Trace

Claim:

```text
The missing TiXL-style gesture operations can be lowered to one undoable command unit each before UI polish.
```

Evidence target:

```text
source/core/InteractionContract.h/.cpp
fixtures/interaction/tooll3-t0-t7.behavior.json
tests/InteractionTraceTests.cpp
```

- [x] Add trace cases for reconnect input end, reconnect output beginning, and split edge create operator.
- [x] Add command result expectations for one user gesture equals one undo unit for reconnect/split macros.
- [x] Implement the smallest commandGraph path for reconnect input end, reconnect output beginning, and split edge create operator.
- [x] Add trace cases for hidden input picker, multi-input ordering, drag-node-onto-edge, snap/unsnap, and shake disconnect.
- [x] Run `cmake --build build --target my_world_t3_t5_command_tests my_world_graph_command_tests my_world_interaction_trace_tests`.
- [x] Run `./build/my_world_interaction_trace_tests`.
- [x] Run `ctest --test-dir build --output-on-failure -R "t3_t5_commands|interaction_traces|graph_commands"`.

P-OPS1A closed as of 2026-05-25 08:36 Asia/Taipei.

Verification:

```text
cmake --build build --target my_world_t3_t5_command_tests my_world_interaction_trace_tests my_world_graph_command_tests
# RED first failed on missing reconnectInputEnd / reconnectOutputBeginning / splitEdgeWithNode helpers.
./build/my_world_t3_t5_command_tests
./build/my_world_graph_command_tests
./build/my_world_interaction_trace_tests
ctest --test-dir build --output-on-failure -R "t3_t5_commands|interaction_traces|graph_commands"
```

Accepted result:

```text
t3 t5 commands ok
graph commands ok
interaction traces ok
3/3 focused tests passed.
```

Scope boundary:

```text
P-OPS1A proves commandGraph macro semantics for reconnecting either edge end and splitting a compatible edge by creating an inserted node.
It does not implement hidden input picking, multi-input ordering, drag-existing-node insert, snap/unsnap preview state, shake disconnect, or visible ImGui gesture handling.
```

P-OPS1B closed as of 2026-05-25 08:47 Asia/Taipei.

Verification:

```text
cmake --build build --target my_world_t3_t5_command_tests my_world_graph_command_tests my_world_interaction_trace_tests
# RED first failed on missing connectHiddenInput / insertInputEdge / insertExistingNodeOnEdge / snapConnect / unsnapDisconnect / shakeDisconnectNode helpers.
./build/my_world_t3_t5_command_tests
./build/my_world_graph_command_tests
./build/my_world_interaction_trace_tests
ctest --test-dir build --output-on-failure -R "t3_t5_commands|interaction_traces|graph_commands"
```

Accepted result:

```text
t3 t5 commands ok
graph commands ok
interaction traces ok
3/3 focused tests passed.
```

Scope boundary:

```text
P-OPS1B proves commandGraph trace semantics for explicit hidden-input connection, ordered fixed-slot multi-input insert, existing-node edge insertion, snap/unsnap committed connection changes, and shake-disconnect of incident edges.
It does not implement ImGui gesture detection, visual preview state, unbounded variadic input lists, or visible picker menus.
```

### P-OUT1 Output Pinning

Claim:

```text
Output view selection follows the selected node until the user pins an output, and save/load preserves that pin.
```

Evidence target:

```text
source/core/OutputViewState.h/.cpp
tests/OutputViewStateTests.cpp
source/ui/ImGuiSmokeOverlay.cpp
```

- [x] Write tests for selection-following output, pin output, selection changes while pinned, unpin returns to follow mode, and save/load roundtrip.
- [x] Implement output view state as data, not ImGui-only state.
- [x] Wire the visible workspace to read the output view state.
- [x] Run `cmake --build build`.
- [x] Run `./build/my_world_output_view_state_tests`.
- [x] Run `ctest --test-dir build --output-on-failure`.

P-OUT1 closed as of 2026-05-25 08:58 Asia/Taipei.

Verification:

```text
cmake -S . -B build && cmake --build build --target my_world_output_view_state_tests
# RED first failed on missing source/core/OutputViewState.h.
cmake --build build --target my_world_output_view_state_tests
./build/my_world_output_view_state_tests
ctest --test-dir build --output-on-failure -R "output_view_state|patch_document|save_work_command|interaction_traces"
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted result:

```text
output view state ok
4/4 focused tests passed.
56/56 full tests passed.
```

Scope boundary:

```text
P-OUT1 proves core output-view follow/pin/unpin state, PatchDocument save/load persistence, saveWork persistence, and visible workspace reading that state.
It does not implement multiple output slots, final-eval-start pins, image canvas fit/1:1/custom modes, screenshot/render toolbar actions, resolution presets, or render/export process states.
```

### P-PARAM1A Parameter Row States

Claim:

```text
Inspector parameter/input rows can derive default/manual/connected/animated state from graph data, reset state through commandGraph, and survive save/load roundtrip.
```

Evidence target:

```text
source/core/ParameterRowState.h/.cpp
tests/ParameterRowStateTests.cpp
source/core/InteractionContract.h/.cpp
source/ui/ImGuiSmokeOverlayInspector.cpp
```

- [x] Write tests for default param rows, manual param rows, reset param undo/redo, edge-connected input rows, animated/manual port binding rows, reset binding, and PatchDocument roundtrip.
- [x] Implement parameter row state as data, not ImGui-only state.
- [x] Add `reset_param` and `reset_port_binding` command paths.
- [x] Wire the visible inspector to read the row state helper and call reset commands.
- [x] Run `cmake --build build --target my_world_parameter_row_state_tests`.
- [x] Run focused command/storage tests.
- [x] Run `cmake --build build`.
- [x] Run `ctest --test-dir build --output-on-failure`.

P-PARAM1A closed as of 2026-05-25 09:15 Asia/Taipei.

Verification:

```text
cmake -S . -B build && cmake --build build --target my_world_parameter_row_state_tests
# RED first failed on missing source/core/ParameterRowState.h.
cmake --build build --target my_world_parameter_row_state_tests
./build/my_world_parameter_row_state_tests
ctest --test-dir build --output-on-failure -R "parameter_row_state|graph_commands|interaction_storage_roundtrip|patch_document|save_work_command"
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted result:

```text
parameter row state ok
5/5 focused tests passed.
57/57 full tests passed.
```

Scope boundary:

```text
P-PARAM1A proves row state derivation and reset command mechanics for existing ParamSpec/Input PortSpec rows.
It does not implement typed scalar/vector widgets, enum/path/multiline controls, lists/curves/gradients/ADSR, parameter grouping/relevancy metadata, context menus, extract-value-node commands, preset capture, snapshot capture, or variation blending.
```

### P-PARAM1B Typed Parameter Controls

Claim:

```text
ParamSpec data types map to deterministic control kinds, typed edits normalize before set_param, invalid edits leave the graph unchanged, and stored typed values roundtrip through PatchDocument.
```

Evidence target:

```text
source/core/ParameterControl.h/.cpp
tests/ParameterControlTests.cpp
source/ui/ImGuiSmokeOverlayInspector.cpp
```

- [x] Write tests for scalar, vector, enum, string, path, multiline, invalid edits, commandGraph logging, and PatchDocument roundtrip.
- [x] Implement deterministic `ParamSpec` type/range to control-kind selection.
- [x] Implement typed edit normalization before `set_param`.
- [x] Reject invalid typed edits without graph mutation.
- [x] Wire the visible inspector to consume typed control state and call the typed command path.
- [x] Run `cmake --build build --target my_world_parameter_control_tests`.
- [x] Run focused parameter/command tests.
- [x] Run `cmake --build build`.
- [x] Run `ctest --test-dir build --output-on-failure`.

P-PARAM1B closed as of 2026-05-25 09:24 Asia/Taipei.

Verification:

```text
cmake -S . -B build && cmake --build build --target my_world_parameter_control_tests
# RED first failed on missing source/core/ParameterControl.h.
cmake --build build --target my_world_parameter_control_tests
./build/my_world_parameter_control_tests
ctest --test-dir build --output-on-failure -R "parameter_controls|parameter_row_state|graph_commands"
cmake --build build --target my_world_imgui
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted result:

```text
parameter controls ok
3/3 focused tests passed.
58/58 full tests passed.
```

Scope boundary:

```text
P-PARAM1B proves deterministic typed control mapping and value normalization for float, double, int, bool, vec2/vec3/vec4/quaternion, enum, string, resource/path, and text.* params.
It does not implement enum flag sets, native file chooser dialogs, specialized list/curve/gradient/ADSR editors, parameter grouping/relevancy metadata, extract-value-node commands, preset/snapshot capture, or variation blending.
```

### P-TIME1 Bars-Native Timeline Model

Claim:

```text
Bars are the canonical timeline truth; seconds and frames are deterministic views derived from bpm, beats-per-bar, and fps, and timeline edits survive command undo/redo plus PatchDocument/saveWork roundtrip.
```

Evidence target:

```text
source/core/TimelineState.h/.cpp
source/core/InteractionContract.h/.cpp
source/storage/StorageContract.h
source/storage/StorageContractPatchDocument.cpp
source/storage/StorageCommand.cpp
tests/TimelineStateTests.cpp
```

- [x] Write RED tests for bars/seconds/frames conversion, bpm/fps command edits, invalid edit rejection, undo/redo, PatchDocument roundtrip, and saveWork roundtrip.
- [x] Implement `TimelineState` as core data with bars-native fields.
- [x] Add commandGraph verbs for tempo, fps, positionBars, and loopBars edits.
- [x] Store timeline state in `PatchDocument`.
- [x] Preserve timeline state through `save_work`.
- [x] Run `cmake --build build --target my_world_timeline_state_tests`.
- [x] Run `./build/my_world_timeline_state_tests`.
- [x] Run focused storage/command tests.
- [x] Run `cmake --build build`.
- [x] Run `ctest --test-dir build --output-on-failure`.

P-TIME1 closed as of 2026-05-25 09:35 Asia/Taipei.

Verification:

```text
cmake -S . -B build && cmake --build build --target my_world_timeline_state_tests
# RED first failed on missing source/core/TimelineState.h.
cmake --build build --target my_world_timeline_state_tests
./build/my_world_timeline_state_tests
ctest --test-dir build --output-on-failure -R "timeline_state|output_view_state|save_work_command|graph_commands"
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted result:

```text
timeline state ok
4/4 focused tests passed.
59/59 full tests passed.
```

Scope boundary:

```text
P-TIME1 proves the bars-native timeline data model, deterministic bars/seconds/frames conversion, command-backed bpm/fps/position/loop edits, undo/redo, PatchDocument roundtrip, and saveWork roundtrip.
It does not implement transport playback, IO indicators, playhead UI, keyframes, curves, clips, time warp, render/export settings, audio soundtrack sync, BPM detection/tapping, or live IO bus behavior.
```

### P-TIME2 Transport Playback Controls

Claim:

```text
Timeline transport controls can play, pause, stop/reset, step by frames, reverse, and toggle loop through commandGraph, while playback tick advances the playhead deterministically from bars-native timeline state.
```

Evidence target:

```text
source/core/TimelineState.h/.cpp
source/core/InteractionContract.h/.cpp
source/storage/StorageContractPatchDocument.cpp
source/storage/StorageCommand.cpp
source/ui/ImGuiSmokeOverlay.cpp
tests/TransportControlTests.cpp
```

- [x] Write RED tests for default transport state, play/pause/stop, invalid speed rejection, frame stepping, looped playback tick, PatchDocument roundtrip, and saveWork roundtrip.
- [x] Add transport state, playback rate, and playback direction to `TimelineState`.
- [x] Add commandGraph verbs for play, pause, stop, and frame step.
- [x] Preserve transport state through `PatchDocument` and `save_work`.
- [x] Wire bottom transport controls to the command-backed transport state.
- [x] Run `cmake --build build --target my_world_transport_control_tests`.
- [x] Run `./build/my_world_transport_control_tests`.
- [x] Run focused timeline/storage tests.
- [x] Run `cmake --build build --target my_world_imgui`.
- [x] Run `cmake --build build`.
- [x] Run `ctest --test-dir build --output-on-failure`.

P-TIME2 closed as of 2026-05-25 09:47 Asia/Taipei.

Verification:

```text
cmake -S . -B build && cmake --build build --target my_world_transport_control_tests
# RED first failed on missing transportState / TimelineTransportState / playTimeline APIs.
cmake --build build --target my_world_transport_control_tests
./build/my_world_transport_control_tests
cmake --build build --target my_world_transport_control_tests my_world_timeline_state_tests my_world_output_view_state_tests my_world_save_work_command_tests my_world_imgui
ctest --test-dir build --output-on-failure -R "transport_controls|timeline_state|output_view_state|save_work_command"
cmake --build build
ctest --test-dir build --output-on-failure
```

Accepted result:

```text
transport controls ok
4/4 focused tests passed.
60/60 full tests passed.
```

Scope boundary:

```text
P-TIME2 proves command-backed transport controls, deterministic playhead tick math, loop wrapping, visible bottom transport consumption, PatchDocument roundtrip, and saveWork roundtrip.
It does not implement audio IO indicators, live IO bus routing, keyframes, curves, time clips, time warp, render/export, soundtrack sync, BPM detection/tapping, or realtime render scheduling.
```

## Downstream Plan Order

The next plans should be created only when the previous queue item has proof evidence:

| Order | Plan | Starts after | Why |
| --- | --- | --- | --- |
| 1 | P-TAX1 taxonomy fixture | this ledger | Browser needs canonical visible paths before UI polish |
| 2 | P-SEARCH1 browser/search/compatible create | P-TAX1 | Search must read taxonomy and NodeSpec metadata |
| 3 | P-OPS1 richer graph gestures | P-SEARCH1 can run in parallel after query helper exists | Split/reconnect needs compatible-create semantics |
| 4 | P-OUT1 output pinning | current live compound runtime surface is stable | Output behavior affects workspace composition |
| 5 | P-PARAM1A parameter row states | P-SEARCH1 and P-OPS1 | Inspector actions need commandGraph verbs and NodeSpec metadata |
| 6 | P-PARAM1B typed parameter controls | P-PARAM1A | Typed edit normalization should precede presets/snapshots |
| 7 | P-TIME1 bars-native timeline model | P-OUT1 | Timeline affects render/export and transport |
| 8 | P-VAR1 presets/snapshots foundation | P-PARAM1B | Variation capture depends on parameter state semantics |
| 9 | P-LIVE1 MIDI/OSC/live IO bus | A1/C1 live runtime remains stable | Live IO should drive proof-backed graph values |

## Self-Review

- The ledger covers taxonomy, browser/search, compatible creation, graph gestures, parameters, presets/snapshots, output, timeline, render/export, audio/MIDI/OSC, and native carrying lines.
- Rows point back to the taxonomy parity spec or existing skeleton/interaction/skin specs instead of redefining their law.
- The immediate queue is limited to the selected parity lane so current work remains controllable.
- Parked rows name a blocker or next proof condition instead of becoming vague future scope.
- No implementation is considered complete from UI visuals alone; each queue item names a test, fixture, trace, or proof dump.
