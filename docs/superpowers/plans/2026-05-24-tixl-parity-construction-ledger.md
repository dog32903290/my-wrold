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
- Future P-SEARCH1 artifacts:
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
| TAX-004 | `top.*`, `sop.*`, `mat.*` are aliases, not roots | non-negotiable rules | TiXL taxonomy plus local NodeSpec aliases | partial | L3 visible | alias search returns native saved type | P-SEARCH1 |
| TAX-005 | 923 operator catalog remains witness, not backlog | node function spec catalog rule | `operator-catalog.json` | proven | L0 law | parked catalog entries do not create runtime rows | none |

### Browser, Search, And Compatible Create

| ID | Feature | Spec row | Witness | Status | Phase | Acceptance trace | Blocker / next proof |
| --- | --- | --- | --- | --- | --- | --- | --- |
| BROW-001 | Empty browser curated root page | Browser And Search Parity | `SymbolBrowsing.UpdateLibPage` | planned | L3 visible | `category_browser_root_exact_tixl_paths` | P-TAX1 fixture |
| BROW-002 | Category drilldown by namespace path | Browser And Search Parity | `SymbolBrowsing` path mutation | planned | L3 visible | `category_browser_drilldown_exact_namespace` | P-TAX1 fixture |
| BROW-003 | Essential overview rows | Browser And Search Parity | `SymbolTags.Essential` | parked | L3 visible | `category_overview_essential_only` | NodeSpec metadata flag |
| BROW-004 | Full symbol library namespace tree | Browser And Search Parity | `SymbolLibrary`, `NamespaceTreeNode` | parked | L3 visible | `symbol_library_namespace_tree_root_order` | secondary library UI |
| SEARCH-001 | Fuzzy search across name, namespace, description | Browser And Search Parity | `SymbolFilter` | planned | L3 visible | `search_fuzzy_name_namespace_description` | NodeSpec browser query helper |
| SEARCH-002 | Search ranking exact, starts-with, contains, PascalCase | Browser And Search Parity | `SymbolFilter` ranking | planned | L3 visible | `search_ranking_exact_starts_contains_pascal` | fixture order |
| COMPAT-001 | Drag output to empty canvas suggests compatible nodes | Browser And Search Parity | `PlaceHolderUi`, `SymbolFilter` | partial | L2 command | `compatible_create_from_output_drag` | P-SEARCH1 |
| COMPAT-002 | Drag input to empty canvas suggests compatible nodes | Browser And Search Parity | `PlaceHolderUi`, `SymbolFilter` | planned | L2 command | `compatible_create_from_input_drag` | P-SEARCH1 |
| COMPAT-003 | Split existing connection through browser | Browser And Search Parity | legacy `ConnectionMaker.SplitConnectionWithSymbolBrowser` | partial | L2 command | `compatible_split_connection_insert_node` | P-OPS1 macro command trace |
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
| GEST-007 | Reconnect existing input end | Graph Interaction Parity | `HoldingConnectionEnd`, `InputSnapper` | partial | L2 command | `reconnect_input_end_one_undo_step` | reconnect macro trace |
| GEST-008 | Reconnect existing output beginning | Graph Interaction Parity | `HoldingConnectionBeginning`, `OutputSnapper` | planned | L2 command | `reconnect_output_beginning_one_undo_step` | P-OPS1 |
| GEST-009 | Drop connection/node onto operator body and choose hidden input | Graph Interaction Parity | `InputPicking` | planned | L2 command | `drop_connection_onto_operator_hidden_input` | hidden input metadata |
| GEST-010 | Multi-input insert before, after, replace | Graph Interaction Parity | `InputSnapper.InputSnapTypes` | planned | L2 command | `multi_input_insert_before_after_replace` | ordered multi-input edges |
| GEST-011 | Split edge by creating operator | Graph Interaction Parity | `ConnectionMaker.SplitConnectionWithSymbolBrowser` | partial | L2 command | `split_edge_create_operator_undo_macro` | P-OPS1 |
| GEST-012 | Drag existing node onto edge to insert | Graph Interaction Parity | `MagItemMovement.TrySplitInsert` | planned | L2 command | `drag_node_onto_edge_split_insert` | edge hit-test plus macro mutation |
| GEST-013 | Snap move creates connection and unsnap removes connection | Graph Interaction Parity | `MagItemMovement` snap/unsnap | planned | L2 command | `snap_move_connect_unsnap_disconnect_undo` | preview vs committed graph split |
| GEST-014 | Shake dragged node to disconnect | Graph Interaction Parity | `ShakeDetector` | planned | L2 command | `shake_disconnect_dragged_node_edges` | gesture trace input model |
| GEST-015 | Enter, exit, collapse, expand compound | Graph Interaction Parity | graph navigation state | proven for compounds | L2 command | existing compound traces | public port persistence line continues |
| GEST-016 | Inspector param and port binding | Graph Interaction Parity | graph input commands | partial | L2 command | `reset_default_manual_binding_fallback` | parameter parity rows |
| GEST-017 | Annotation add, drag, resize, rename, delete, collapse | Graph Interaction Parity | annotation interaction files | parked | L3 visible | `annotation_frame_move_resize_rename_collapse_undo` | annotation scope decision |
| GEST-018 | Symbol, file, or asset drop creates graph item | Graph Interaction Parity | `DropHandling` | parked | L3 visible | `drop_symbol_creates_node_undo` | asset browser boundary |

### Parameters, Presets, Snapshots, Variations

| ID | Feature | Spec row | Witness | Status | Phase | Acceptance trace | Blocker / next proof |
| --- | --- | --- | --- | --- | --- | --- | --- |
| PARAM-001 | Parameter inspector shell | Parameter, Preset, And Snapshot Parity | `ParameterWindow`, `IInputUi` | partial | L3 visible | `select_node_inspector_rows_set_param` | richer NodeSpec params |
| PARAM-002 | Row states: normal, connected, animated, default/reset | Parameter parity | input UI state | partial | L3 visible | `default_manual_connected_animated_undo` | animated state model |
| PARAM-003 | Type color families | Parameter parity | `TypeUiProperties` | partial | L3 visible | `port_type_family_visual_mapping` | TypeSpec color map |
| PARAM-004 | Scalar/vector controls | Parameter parity | typed input UIs | planned | L3 visible | `typed_scalar_vector_param_roundtrip` | param storage schema |
| PARAM-005 | Enum, string, path, multiline controls | Parameter parity | typed input UIs | planned | L3 visible | `typed_text_enum_path_param_roundtrip` | param storage schema |
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
| OUT-001 | Output follows selection unless pinned | Output parity | `ViewSelectionPinning` | planned | L3 visible | `select_changes_output_pin_freezes_output_reload` | P-OUT1 |
| OUT-002 | Output slot and final eval start pin | Output parity | output id/eval-start pin | parked | L4 live | `multi_output_view_pin_final_eval_pin` | multi-output runtimeGraph |
| OUT-003 | Image canvas Fit, 1:1, Custom pan/zoom, overlay | Output parity | `ImageOutputCanvas` | partial | L3 visible | `output_fit_1to1_custom_view_state` | RenderBackend extraction |
| OUT-004 | Output toolbar screenshot and render settings vocabulary | Output parity | output window toolbar | parked mixed | L3 visible | `screenshot_or_frame_dump_active_output` | active output view state |
| OUT-005 | Resolution presets | Output parity | output settings | parked | L3 visible | `requested_resolution_preset_roundtrip` | output state model |
| OUT-006 | Render/export window | Output parity | render export files | parked | L4 live | `render_settings_roundtrip_frame_count` | RenderBackend plus timeline |
| OUT-007 | Render process states | Output parity | render state model | parked | L4 live | `invalid_output_blocks_render_state` | active output type system |
| TIME-001 | Bars are canonical timeline truth; seconds/frames are views | Timeline parity | timeline files | parked | L2 command | `bars_seconds_frames_conversion_bpm_fps` | timeline model |
| TIME-002 | Playback controls and IO indicator | Timeline parity | transport controls | partial/parked | L4 live | `transport_play_loop_io_indicator` | timeline plus IO bus |
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

- [ ] Define `BrowserNodeEntry` from `NodeSpec`, visible taxonomy path, aliases, description, ports, runtime readiness, and hidden/default state.
- [ ] Write search tests for exact, starts-with, contains, PascalCase, namespace, description, `top.*` alias, and native saved type preservation.
- [ ] Write compatible-create tests for output context and input context using existing seed specs plus loaded module specs.
- [ ] Implement the smallest deterministic query helper without coupling it to ImGui.
- [ ] Add behavior fixture rows for `compatible_create_from_output_drag`, `compatible_create_from_input_drag`, and cancel-without-mutation.
- [ ] Run `cmake --build build`.
- [ ] Run `./build/my_world_node_spec_browser_tests` and `./build/my_world_interaction_trace_tests`.

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

- [ ] Add trace cases for reconnect input end, reconnect output beginning, split edge, hidden input picker, multi-input ordering, drag-node-onto-edge, snap/unsnap, and shake disconnect.
- [ ] Add command result expectations for one user gesture equals one undo unit.
- [ ] Implement the smallest commandGraph path for one operation at a time.
- [ ] Run `cmake --build build`.
- [ ] Run `./build/my_world_interaction_trace_tests`.
- [ ] Run `ctest --test-dir build --output-on-failure`.

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

- [ ] Write tests for selection-following output, pin output, selection changes while pinned, unpin returns to follow mode, and save/load roundtrip.
- [ ] Implement output view state as data, not ImGui-only state.
- [ ] Wire the visible workspace to read the output view state.
- [ ] Run `cmake --build build`.
- [ ] Run `./build/my_world_output_view_state_tests`.
- [ ] Run `ctest --test-dir build --output-on-failure`.

## Downstream Plan Order

The next plans should be created only when the previous queue item has proof evidence:

| Order | Plan | Starts after | Why |
| --- | --- | --- | --- |
| 1 | P-TAX1 taxonomy fixture | this ledger | Browser needs canonical visible paths before UI polish |
| 2 | P-SEARCH1 browser/search/compatible create | P-TAX1 | Search must read taxonomy and NodeSpec metadata |
| 3 | P-OPS1 richer graph gestures | P-SEARCH1 can run in parallel after query helper exists | Split/reconnect needs compatible-create semantics |
| 4 | P-OUT1 output pinning | current live compound runtime surface is stable | Output behavior affects workspace composition |
| 5 | P-PARAM1 parameter row states | P-SEARCH1 and P-OPS1 | Inspector actions need commandGraph verbs and NodeSpec metadata |
| 6 | P-VAR1 presets/snapshots foundation | P-PARAM1 | Variation capture depends on parameter state semantics |
| 7 | P-TIME1 bars-native timeline model | P-OUT1 | Timeline affects render/export and transport |
| 8 | P-LIVE1 MIDI/OSC/live IO bus | A1/C1 live runtime remains stable | Live IO should drive proof-backed graph values |

## Self-Review

- The ledger covers taxonomy, browser/search, compatible creation, graph gestures, parameters, presets/snapshots, output, timeline, render/export, audio/MIDI/OSC, and native carrying lines.
- Rows point back to the taxonomy parity spec or existing skeleton/interaction/skin specs instead of redefining their law.
- The immediate queue is limited to P-TAX1, P-SEARCH1, P-OPS1, and P-OUT1 so current work remains controllable.
- Parked rows name a blocker or next proof condition instead of becoming vague future scope.
- No implementation is considered complete from UI visuals alone; each queue item names a test, fixture, trace, or proof dump.
