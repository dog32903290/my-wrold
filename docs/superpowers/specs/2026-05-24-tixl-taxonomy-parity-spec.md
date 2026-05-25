# TiXL Taxonomy Parity Spec

Date: 2026-05-24
Status: initial parity gate. Source audit completed with five read-only subagents over TiXL taxonomy, browser/search, graph interaction, parameter/variation, and output/timeline/live-performance surfaces. No implementation is implied by this file.

## Purpose

`我的世界` can keep its own native graph, runtime, storage, renderer, and command system.

The user-facing operator classification entry points must mirror TiXL:

```text
TiXL visible operator path
-> My World browser category path
-> native NodeSpec / module / command implementation
```

If a node, module, search result, compatible suggestion, preset, timeline action, output view, or live IO entry point is user-visible, it needs a row in this spec or in a downstream spec linked from this spec.

## Source Witness

```text
repo: https://github.com/tixl3d/tixl.git
commit: 26dc80c7e3a85389f6619ffcd76b1e75e03547aa
local audit clone: /tmp/tixl-upstream
primary taxonomy root: Operators/Lib
existing witness catalog: fixtures/tixl-witness/operator-catalog.json
catalog entries: 923
complete .cs + .t3 + .t3ui triples: 922
```

Relevant TiXL source surfaces:

```text
Operators/Lib/**
Editor/Gui/MagGraph/Interaction/SymbolBrowsing.cs
Editor/Gui/MagGraph/Interaction/PlaceHolderUi.cs
Editor/Gui/Windows/SymbolLib/SymbolLibrary.cs
Editor/UiModel/Helpers/SymbolFilter.cs
Editor/Gui/MagGraph/Interaction/PlaceholderCreation.cs
Editor/Gui/MagGraph/Interaction/InputSnapper.cs
Editor/Gui/MagGraph/Interaction/OutputSnapper.cs
Editor/Gui/MagGraph/Interaction/InputPicking.cs
Editor/UiModel/Commands/Graph/*.cs
Editor/UiModel/Commands/UndoRedoStack.cs
Editor/Gui/Windows/ParameterWindow.cs
Editor/UiModel/InputsAndTypes/*.cs
Editor/Gui/InputUi/**/*.cs
Editor/Gui/Windows/Variations/*.cs
Editor/Gui/Windows/ImageOutputCanvas.cs
Editor/Gui/Windows/ViewSelectionPinning.cs
Editor/Gui/Interaction/WithCurves/AnimationCanvas.cs
Editor/Gui/Windows/TimeLine/**/*.cs
Editor/UiModel/Commands/Animation/*.cs
```

## Non-Negotiable Rules

- Browser category labels and paths mirror TiXL visible `Operators/Lib` taxonomy.
- Saved graph `type` remains My World native law. TiXL paths are browser taxonomy and search aliases, not saved identity.
- `top.*`, `sop.*`, `mat.*`, and similar names are aliases or filters. They must not replace TiXL roots such as `image`, `mesh`, `point`, and `render`.
- Category/subcategory never decides runtime execution. Runtime dispatch still uses `runtimeDomain`, ports, `TypeSpec`, and `RuntimeOp` coverage.
- Hidden, internal, obsolete, experimental, and resource folders are not exposed in the default operator browser.
- Every user gesture that mutates graph state lowers to commandGraph and has undo/redo evidence.
- Every future implementation pass that borrows TiXL behavior must name TiXL witness files, mirror/adapt/park/reject status, and an acceptance trace before code.

## Status Vocabulary

```text
mirror   user-facing label/path/operation should match TiXL
adapt    user-facing intent matches, native mechanics differ; reason required
proven   covered by current tests, fixtures, or proof dump
partial  shell or core exists, but TiXL parity trace is incomplete
parked   explicit requirement, blocked by a named later proof line
reject   explicitly not part of My World; reason required
```

## Default Operator Browser Taxonomy

The default operator browser mirrors visible TiXL operator paths under `Operators/Lib`, excluding hidden/import-only paths listed later.

| TiXL root | Visible children | Mirror policy | Local internal mapping | Status |
| --- | --- | --- | --- | --- |
| `data` | `object` | mirror | `data.*` | planned |
| `field` | `adjust`, `analyze`, `combine`, `generate`, `render`, `space`, `use` | mirror | `field.*`, future `sdf.*` | planned |
| `flow` | root operators, `context` | mirror partial | `command.flow.*` | planned |
| `image` | `analyze`, `color`, `fx`, `generate`, `transform`, `use` | mirror | `image.*`; aliases may include `top.*` | partial |
| `io` | `audio`, `dmx`, `file`, `freed`, `http`, `input`, `json`, `midi`, `osc`, `posistage`, `ptz`, `serial`, `tcp`, `udp`, `video`, `websocket` | mirror with platform flags | `io.*` | partial |
| `mesh` | `draw`, `generate`, `modify` | mirror | `mesh.*`; aliases may include `sop.*` | partial |
| `numbers` | `anim`, `bool`, `color`, `curve`, `data`, `float`, `floats`, `int`, `int2`, `ints`, `vec2`, `vec3`, `vec4` | mirror | `signal.*`, `vector.*`, `color.*` | planned |
| `particle` | `force` | mirror, parked runtime | `particle.*` | parked |
| `point` | `combine`, `draw`, `generate`, `helper`, `io`, `modify`, `sim`, `transform`, `usse` | mirror | `point.*` | partial |
| `render` | `analyze`, `basic`, `camera`, `gizmo`, `postfx`, `scene`, `shading`, `sprite`, `transform`, `utils` | mirror, runtime parked by RenderBackend | `render.*`, `command.graph`, `material.shader` | partial |
| `string` | `buffers`, `combine`, `convert`, `datetime`, `list`, `logic`, `random`, `search`, `transform` | mirror | `text.*`, future `string.*` alias | planned |

Important third-level paths to preserve when those branches are exposed:

```text
image/fx/blur
image/fx/distort
image/fx/feedback
image/fx/glitch
image/fx/stylize
image/generate/basic
image/generate/fractal
image/generate/load
image/generate/misc
image/generate/noise
image/generate/pattern
field/generate/sdf
field/generate/texture
field/generate/vec3
numbers/anim/animators
numbers/anim/time
numbers/anim/vj
numbers/bool/combine
numbers/bool/convert
numbers/bool/logic
numbers/bool/process
numbers/float/adjust
numbers/float/basic
numbers/float/logic
numbers/float/process
numbers/float/random
numbers/float/trigonometry
render/camera/analyze
string/buffers/convert
string/buffers/transform
```

## Hidden Or Non-Default Paths

These must not appear in the default operator browser even if present in the witness catalog.

| TiXL path pattern | Default policy | Reason | Future entry point |
| --- | --- | --- | --- |
| `Operators/Lib/.meta` | hidden | thumbnails/cache | none |
| `Operators/Lib/Assets` | not an operator category | resources, shaders, meshes, audio, templates | resource/asset browser |
| `Operators/Lib/Utils` | hidden | source helpers, no operator triples | none |
| `*/_` | hidden/import-only | internal implementation helpers | debug/import mode |
| `*/_obsolete` | hidden/import-only | obsolete compatibility | migration mode |
| `*/_internal` | hidden/import-only | internal graph helpers | debug/import mode |
| `*/_experimental` | hidden/import-only | unstable/lab | optional lab mode |
| `*/obsolete` | hidden/import-only | old compatibility | migration mode |
| `*/legacy` | hidden/import-only | legacy compatibility | migration mode |
| `*/experimental` | hidden/import-only | unstable/lab | optional lab mode |
| operator files beginning with `_` | hidden by default | helper/internal naming | debug/import mode |
| `flow/skillQuest` | hidden by default | skill/quiz-like material, not default creative operator taxonomy | learning mode |

## P-TAX1 Fixture Evidence

Closed as of 2026-05-25 02:16 Asia/Taipei.

`TAX-001` through `TAX-003` are proven at L1 witness level by:

```text
fixtures/tixl-witness/operator-browser-taxonomy.json
tests/TiXLTaxonomyFixtureTests.cpp
```

Verified acceptance traces:

```text
category_browser_root_exact_tixl_paths
category_browser_drilldown_exact_namespace
hidden_paths_excluded_from_default_browser
```

This proves the deterministic taxonomy fixture and hidden/default rules. It does not implement the visible browser UI, search ranking, compatible create, or saved graph type mapping.

## P-SEARCH1 Fixture Evidence

Closed as of 2026-05-25 02:27 Asia/Taipei.

Search, alias, and compatible-create candidate filtering are proven at core helper / fixture level by:

```text
source/core/NodeSpecBrowser.h
source/core/NodeSpecBrowser.cpp
tests/NodeSpecBrowserTests.cpp
fixtures/interaction/tixl-search-compatible-create.behavior.json
```

Verified acceptance traces:

```text
search_fuzzy_name_namespace_description
search_ranking_exact_starts_contains_pascal
compatible_create_from_output_drag
compatible_create_from_input_drag
browser_keyboard_return_escape_no_mutation_on_cancel
```

This proves deterministic query behavior and native saved type preservation for TiXL-style aliases such as `top.texture -> image.texture`. It does not implement the visible ImGui browser, keyboard focus handling, or split-connection macro command.

## Browser And Search Parity

| Behavior | TiXL witness | Policy | Required My World trace | Blocker |
| --- | --- | --- | --- | --- |
| Empty browser curated page | `SymbolBrowsing.UpdateLibPage`: `Lib` category page | mirror visible labels/path | `category_browser_root_exact_tixl_paths` | canonical visible path fixture |
| Category drilldown | `SymbolBrowsing` path mutation and active namespace results | mirror | `category_browser_drilldown_exact_namespace` | none |
| Essential overview rows | `SymbolTags.Essential` for level-one overview | adapt until `NodeSpec` has essential flag | `category_overview_essential_only` | NodeSpec metadata |
| Full symbol library tree | `SymbolLibrary`, `NamespaceTreeNode` | mirror later as secondary library surface | `symbol_library_namespace_tree_root_order` | full library UI |
| Fuzzy text search | `SymbolFilter`: subsequence regex, namespace/description match | mirror user-visible behavior, not regex bugs | `search_fuzzy_name_namespace_description` | P-SEARCH1 core helper proven; UI wiring later |
| Ranking | exact, starts-with, contains, PascalCase, package boosts | mirror visible fixture order; adapt scoring internals | `search_ranking_exact_starts_contains_pascal` | P-SEARCH1 deterministic helper proven; usage boosts optional later |
| Output drag to empty canvas | source output type filters candidate inputs | mirror | `compatible_create_from_output_drag` | P-SEARCH1 candidate filter proven; visible browser trigger later |
| Input drag to empty canvas | target input type filters candidate outputs | mirror | `compatible_create_from_input_drag` | P-SEARCH1 candidate filter proven; visible browser trigger later |
| Split existing connection through browser | input/output filters plus old-edge removal and two-edge insert | mirror as one macro command | `compatible_split_connection_insert_node` | reconnect/split trace |
| Keyboard create/cancel | arrows, Return, Escape/click outside | mirror | `browser_keyboard_return_escape_no_mutation_on_cancel` | P-SEARCH1 fixture row recorded; UI event trace schema still needed |

## P-OPS1A Fixture Evidence

Closed as of 2026-05-25 08:36 Asia/Taipei.

Reconnect and split-edge macro operations are proven at commandGraph / behavior trace level by:

```text
source/core/InteractionContract.h
source/core/InteractionContract.cpp
tests/T3T5CommandTests.cpp
tests/InteractionTraceTests.cpp
fixtures/interaction/tooll3-t0-t7.behavior.json
```

Verified acceptance traces:

```text
reconnect_input_end_one_undo_step
reconnect_output_beginning_one_undo_step
split_edge_create_operator_undo_macro
```

This proves one undoable command unit for reconnecting either edge end and for splitting a compatible edge by creating an inserted node. It does not implement hidden input picking, ordered multi-input insertion, drag-existing-node insertion, snap/unsnap preview state, shake disconnect, or visible ImGui gesture handling.

## P-OPS1B Fixture Evidence

Closed as of 2026-05-25 08:47 Asia/Taipei.

Remaining graph-operation command traces are proven at commandGraph / behavior trace level by:

```text
source/core/InteractionContract.h
source/core/InteractionContract.cpp
source/core/GraphLanguage.cpp
tests/T3T5CommandTests.cpp
tests/GraphCommandTests.cpp
tests/InteractionTraceTests.cpp
fixtures/interaction/tooll3-t0-t7.behavior.json
```

Verified acceptance traces:

```text
drop_connection_onto_operator_hidden_input
multi_input_insert_before_after_replace
drag_node_onto_edge_split_insert
snap_move_connect_unsnap_disconnect_undo
shake_disconnect_dragged_node_edges
```

This proves explicit hidden-input connection, ordered fixed-slot multi-input insertion, inserting an existing node into an edge, committed snap/unsnap connection changes, and shake-disconnect of a dragged node's incident edges. It does not implement visible ImGui gesture detection, visual preview state, unbounded variadic input lists, or picker menus.

## Graph Interaction Parity

Current My World claim remains `T0-T7 core parity`, not full TiXL operation parity.

| Gesture | TiXL witness | Current status | Required trace |
| --- | --- | --- | --- |
| Canvas pan / wheel zoom / focus-preserving zoom | `ScalableCanvas` | proven | existing `canvas pan zoom stable hit-test` |
| Single node select and move | `GraphStates`, `MagItemMovement` | proven | existing `move_node` trace |
| Multi-select / fence / group move | MagGraph fence and selected item movement | partial | `multi_select_group_move_undo_save` |
| Port-to-port connect | `InputSnapper`, `OutputSnapper` | proven | existing `connect` trace |
| Selected edge delete | `Modifications.DeleteSelection` | proven | existing `disconnect` trace |
| Selected node delete plus incident edges | `DeleteSymbolChildrenCommand` | proven | existing `delete_node` trace |
| Reconnect existing input end | `HoldingConnectionEnd`, `InputSnapper` | proven at commandGraph macro level | `reconnect_input_end_one_undo_step` |
| Reconnect existing output beginning | `HoldingConnectionBeginning`, `OutputSnapper` | proven at commandGraph macro level | `reconnect_output_beginning_one_undo_step` |
| Drop connection/node onto operator body and choose hidden input | `InputPicking` | proven at commandGraph macro level; picker UI later | `drop_connection_onto_operator_hidden_input` |
| Multi-input insert before/after/replace | `InputSnapper.InputSnapTypes` | proven for fixed ordered input slots; variadic lists parked | `multi_input_insert_before_after_replace` |
| Split edge by creating operator | legacy `ConnectionMaker.SplitConnectionWithSymbolBrowser` | proven at commandGraph macro level; browser trigger later | `split_edge_create_operator_undo_macro` |
| Drag existing node onto edge to insert | `MagItemMovement.TrySplitInsert` | proven at commandGraph macro level; edge hit-test UI later | `drag_node_onto_edge_split_insert` |
| Snap move creates connection / unsnap removes connection | `MagItemMovement` snap/unsnap paths | proven for committed graph mutation; preview UI later | `snap_move_connect_unsnap_disconnect_undo` |
| Shake dragged node to disconnect | `ShakeDetector`, `NodeActions.DisconnectDraggedNodes` | proven at commandGraph macro level; gesture threshold UI later | `shake_disconnect_dragged_node_edges` |
| Enter / exit symbol or compound | graph navigation state | proven for compounds | existing compound traces |
| Inspector param and port binding | graph input commands | partial vs TiXL defaults | `reset_default_manual_binding_fallback` |
| Annotation add/drag/resize/rename/delete/collapse | annotation interaction files | parked | `annotation_frame_move_resize_rename_collapse_undo` if in scope |
| Symbol/file/asset drop to create | `DropHandling` | parked | `drop_symbol_creates_node_undo` |

One user action must become one undo unit even when the native implementation lowers it to several commands.

## Parameter, Preset, And Snapshot Parity

| Feature | TiXL witness | Policy | Current status | Required trace |
| --- | --- | --- | --- | --- |
| Parameter inspector shell | `ParameterWindow`, `IInputUi` | mirror | partial; row state and typed control core proven | `select_node_inspector_rows_set_param` |
| Row state model | normal, connected, animated, default/reset | mirror | proven at core/storage/visible-state level | `default_manual_connected_animated_undo` |
| Type color families | `TypeUiProperties` | mirror user families, adapt TypeSpec | partial | `port_type_family_visual_mapping` |
| Scalar/vector controls | float, int, bool, double, vector, quaternion | mirror core widgets | proven at core/storage/visible-state level | `typed_scalar_vector_param_roundtrip` |
| Enum/string/path/multiline | enum combo/flags, string usage modes | mirror | proven for enum menu, string, path, multiline; enum flags parked | `typed_text_enum_path_param_roundtrip` |
| Lists, curves, gradients, ADSR | specialized input UIs | parked | parked | `list_curve_gradient_adsr_roundtrip` |
| Parameter metadata | group title, padding, relevancy, description, exclude-from-presets | mirror | gap | `nodespec_parameter_metadata_visibility` |
| Input operations menu | set default, reset default, extract value node, connect/search, rename, settings | mirror core ops | partial | `reset_param_set_default_extract_value_node` |
| Presets vs snapshots | `VariationsWindow` mode split | mirror as separate concepts | shell only | `presets_vs_snapshots_context_switch` |
| Preset capture | selected instance non-default blendable inputs | mirror semantics | gap | `create_apply_preset_capture_report` |
| Snapshot capture | snapshot-enabled children only | mirror | gap | `create_apply_snapshot_enabled_children` |
| Variation canvas ops | create, rename, move, delete, apply, hover preview, Alt blend | mirror first create/apply/rename/delete; park preview/blend | gap | `variation_thumbnail_crud_undo` |
| Blending | `ValueUtils` type-specific blend rules | parked | parked | `deterministic_variation_blend_preview_commit_cancel` |
| Presets in symbol browser | create node with preset | parked | parked | `drag_pin_choose_node_preset_create_apply_connect` |

Variation operations need explicit commandGraph verbs before implementation:

```text
create_preset
create_snapshot
apply_preset
apply_snapshot
rename_variation
delete_variation
move_variation
set_snapshot_enabled
begin_variation_preview
commit_variation_preview
cancel_variation_preview
```

Preset/snapshot capture reports must include skip reasons:

```text
default
excludedFromPresets
unsupportedType
missingInput
```

## P-PARAM1A Fixture Evidence

Closed as of 2026-05-25 09:15 Asia/Taipei.

Parameter row state is proven at core, commandGraph, storage, and visible inspector state level by:

```text
source/core/ParameterRowState.h
source/core/ParameterRowState.cpp
tests/ParameterRowStateTests.cpp
source/core/InteractionContract.h
source/core/InteractionContract.cpp
source/ui/ImGuiSmokeOverlayInspector.cpp
```

Verified acceptance trace:

```text
default_manual_connected_animated_undo
```

This proves default/manual parameter rows, edge-connected input rows, animated/manual port binding rows, `reset_param`, `reset_port_binding`, undo/redo for reset, PatchDocument roundtrip, and visible inspector state consumption. It does not implement typed scalar/vector widgets, enum/path/multiline controls, lists/curves/gradients/ADSR, parameter metadata grouping/relevancy, extract-value-node commands, preset/snapshot capture, or variation blending.

## P-PARAM1B Fixture Evidence

Closed as of 2026-05-25 09:24 Asia/Taipei.

Typed parameter controls are proven at core, commandGraph, storage, and visible inspector consumption level by:

```text
source/core/ParameterControl.h
source/core/ParameterControl.cpp
tests/ParameterControlTests.cpp
source/ui/ImGuiSmokeOverlayInspector.cpp
```

Verified acceptance traces:

```text
typed_scalar_vector_param_roundtrip
typed_text_enum_path_param_roundtrip
```

This proves deterministic control-kind selection and edit normalization for float, double, int, bool, vec2/vec3/vec4/quaternion, enum options encoded as `a|b|c`, string, resource/path, and multiline `text.*` params. It also proves range clamping with `min..max`, invalid edit rejection without graph mutation, `setTypedParam -> set_param`, PatchDocument roundtrip, and visible inspector typed-control consumption. It does not implement enum flag sets, native file chooser dialogs, specialized list/curve/gradient/ADSR editors, parameter grouping/relevancy metadata, extract-value-node commands, preset/snapshot capture, or variation blending.

## P-OUT1 Fixture Evidence

Closed as of 2026-05-25 08:58 Asia/Taipei.

Output follow/pin state is proven at core, storage, save-work, and visible workspace state level by:

```text
source/core/OutputViewState.h
source/core/OutputViewState.cpp
tests/OutputViewStateTests.cpp
source/storage/StorageContractPatchDocument.cpp
source/storage/StorageCommand.cpp
source/ui/ImGuiSmokeOverlay.cpp
```

Verified acceptance trace:

```text
select_changes_output_pin_freezes_output_reload
```

This proves selection-following output state, pin-to-selected-node, pinned output staying fixed across later selection changes, unpin returning to the current selected node, PatchDocument roundtrip, saveWork roundtrip, and visible workspace state display/control. It does not implement multiple output slots, final eval start pinning, image canvas view modes, screenshot/render toolbar commands, resolution presets, or render/export process states.

## P-TIME1 Fixture Evidence

Closed as of 2026-05-25 09:35 Asia/Taipei.

Bars-native timeline state is proven at core, commandGraph, storage, and saveWork level by:

```text
source/core/TimelineState.h
source/core/TimelineState.cpp
source/core/InteractionContract.h
source/core/InteractionContract.cpp
source/storage/StorageContract.h
source/storage/StorageContractPatchDocument.cpp
source/storage/StorageCommand.cpp
tests/TimelineStateTests.cpp
```

Verified acceptance trace:

```text
bars_seconds_frames_conversion_bpm_fps
```

This proves bars as canonical timeline storage, seconds/frames as deterministic views, bpm/fps/position/loop edits through commandGraph, invalid timeline edit rejection without command logging, undo/redo for timeline edits, PatchDocument roundtrip, and saveWork roundtrip. It does not implement transport playback, IO indicators, playhead UI, keyframes, curves, clips, time warp, render/export settings, audio soundtrack sync, BPM detection/tapping, or live IO bus behavior.

## P-TIME2 Fixture Evidence

Closed as of 2026-05-25 09:47 Asia/Taipei.

Transport playback controls are proven at core, commandGraph, storage, saveWork, and visible bottom-strip consumption level by:

```text
source/core/TimelineState.h
source/core/TimelineState.cpp
source/core/InteractionContract.h
source/core/InteractionContract.cpp
source/storage/StorageContractPatchDocument.cpp
source/storage/StorageCommand.cpp
source/ui/ImGuiSmokeOverlay.cpp
tests/TransportControlTests.cpp
```

Verified acceptance trace:

```text
transport_play_loop_io_indicator
```

This proves default stopped transport state, play/pause/stop, reverse direction, playback rate validation, frame stepping from bars-native time, looped playback tick wrapping, invalid transport edit rejection without command logging, undo/redo for transport commands, PatchDocument roundtrip, saveWork roundtrip, and visible bottom transport controls. It does not implement audio IO indicators, live IO bus routing, keyframes, curves, time clips, time warp, render/export, soundtrack sync, BPM detection/tapping, or realtime render scheduling.

## Output, Timeline, And Live Performance Parity

| Feature | TiXL witness | Policy | Current status | Required trace | Blocker |
| --- | --- | --- | --- | --- | --- |
| Output follows selection unless pinned | `ViewSelectionPinning`, output window state | mirror as UX contract | proven at core/storage/visible-state level | `select_changes_output_pin_freezes_output_reload` | multi-output slot and render toolbar parked |
| Output slot / final eval start pin | output id and eval-start pin | parked | parked | `multi_output_view_pin_final_eval_pin` | multi-output runtimeGraph |
| Image canvas modes | Fit, 1:1, Custom pan/zoom, size/format overlay | mirror soon | partial | `output_fit_1to1_custom_view_state` | RenderBackend extraction |
| Output toolbar | gizmo/grid/camera/background/resolution/screenshot/render settings | mirror screenshot/resolution vocabulary; park camera/gizmo | parked mixed | `screenshot_or_frame_dump_active_output` | 3D/camera phase |
| Resolution presets | Fill, 1:1, aspect ratios, 480p/720p/1080p/4k/8k, custom | mirror later | parked | `requested_resolution_preset_roundtrip` | RenderBackend/output state |
| Render/export window | video/image sequence, Bars/Secs/Frames, FPS, resolution %, motion blur, audio | mirror taxonomy, park implementation | parked | `render_settings_roundtrip_frame_count` | RenderBackend + timeline |
| Render process states | NoOutputWindow, NoValidOutputType, ReadyForExport, Exporting | mirror later | parked | `invalid_output_blocks_render_state` | active output type system |
| Time model/display | bars canonical; seconds/frames are views | mirror when timeline begins | proven at core/storage/command level | `bars_seconds_frames_conversion_bpm_fps` | transport/render/export parked |
| Playback controls | play, reverse, speed, stop/reset, stepping, loop, idle motion, IO indicator | mirror gradually | proven for command-backed transport controls; IO bus parked | `transport_play_loop_io_indicator` | live IO bus |
| Keyframes / curves | add/change/delete/move, interpolation, tangents, copy/paste | parked | parked | `keyframe_curve_undo_redo_exact` | animation data model |
| Time clips / time warp | trim/stretch/remap/split/delete/warp handles | parked | parked | `time_clip_retime_no_overlap` | timeline phase |
| Composition audio source | soundtrack vs external device, sync mode, BPM, tapping, gain/decay | mirror A1 input/gain now, park advanced | partial | `audio_input_to_meter_to_uniform` | live sample-window runner |
| Global audio mixers | app/operator/soundtrack volume/mute/meters | parked | parked | `audio_mixer_mute_route` | audio graph/mix bus |
| MIDI input taxonomy | device, channel, control, event type, output range, damping, teach, wasHit | mirror taxonomy soon | partial | `teach_midi_cc_binding_flash` | MIDI input manager |
| MIDI output taxonomy | CC, note, pitchbend, sysex, trigger | park broad nodes; keep loudness CC proof | partial | `loudness_cc_output_stream_enabled` | live performance graph |
| OSC input | port/address/scanned addresses/filters/value grouping | parked | parked | `osc_address_to_signal_value` | IO event bus |
| Audio analyzer/operator family | `io/audio`, `AudioReaction`, `DetectBpm` | mirror analyzer decomposition, not black box | partial/proven loudness | `loaded_loudness_outputs_drive_live_surface` | C1.19 |
| Exported executable/live show controls | project executable settings, playback controls | parked | parked | `exported_show_keyboard_playback` | packaging/runtime mode |

TiXL's animation truth is bars. My World must not store timeline animation as frame-native data if it wants parity; frames and seconds are views/export references.

## Current Local Spec Mapping

| Existing local spec | Covered | Missing from that spec |
| --- | --- | --- |
| `2026-05-22-native-canvas-skeleton-design.md` | native graph/runtime/storage boundaries, first TiXL-seeded category list, V1/A1/C1, RenderBackend parking | full TiXL visible taxonomy tree and hidden/default rules |
| `2026-05-22-tooll3-interaction-borrowing-design.md` | T0-T7 core command-backed interaction | reconnect, split/insert, hidden input picker, multi-input order, snap/unsnap, shake disconnect |
| `2026-05-22-tooll3-skin-parity-spec.md` | output-as-background, left rail, bottom strip, node/port skin, browser shell | taxonomy mirror gate, search ranking, output pinning, timeline commandGraph |
| `2026-05-23-node-function-spec-from-tixl.md` | witness catalog and selected seed node functions | full browser taxonomy as a user-facing contract |

## Spec Gate For Future Work

Progress is tracked in:

```text
docs/superpowers/plans/2026-05-24-tixl-parity-construction-ledger.md
```

Before adding or changing any visible node/module/browser/search/parameter/output/timeline/live-IO feature:

1. Add or update a row in this spec or a downstream parity spec.
2. Name TiXL witness file(s) and source commit.
3. Mark `mirror`, `adapt`, `parked`, or `reject`.
4. Define the acceptance trace before implementation.
5. Keep saved graph type and user-facing taxonomy separate.
6. Route persistent mutation through commandGraph.
7. Add proof evidence before changing the row to `proven`.

## Next Lines

The practical parity proof order is:

```text
P-TAX1 category browser fixture: closed
Operators/Lib visible taxonomy
-> generated My World browser taxonomy fixture
-> hidden/default rules tested

P-SEARCH1 search and compatible-create fixture: closed at core helper / fixture level
TiXL-style fuzzy search + namespace match + port-compatible candidate filter
-> NodeSpec registry query
-> compatible-create behavior fixture

P-OPS1A richer graph-operation trace: closed for reconnect/split macro commands
reconnect input end, reconnect output beginning, split edge create operator
-> one user gesture = one undo unit

P-OPS1B richer graph-operation continuation: closed at commandGraph trace level
hidden input picker, multi-input ordering, drag-existing-node insert, snap/unsnap, shake disconnect
-> one user gesture = one undo unit

P-OUT1 output pinning: closed at core/storage/visible-state level
selection-following output
-> pin output
-> selection changes do not change output
-> save/load preserves pin

P-PARAM1A parameter row states: closed at core/storage/visible-state level
default/manual parameter rows
-> connected/animated input rows
-> reset commands with undo/redo
-> save/load preserves row state

P-PARAM1B typed parameter controls: closed at core/storage/visible-state level
ParamSpec type/range
-> typed control kind
-> normalized edit
-> set_param command
-> save/load preserves typed value

P-TIME1 bars-native timeline model: closed at core/storage/command level
positionBars/loopBars
-> seconds/frames derived from bpm/fps
-> timeline commandGraph edits with undo/redo
-> save/load preserves timeline state

P-TIME2 transport playback controls: closed for command-backed transport controls
play/pause/stop/step/reverse/loop controls
-> transport commandGraph edits with undo/redo
-> deterministic playhead tick from bars-native timeline state
-> save/load preserves transport state

Next selectable parity line:
No active parity line until selected. Closest ordered lane is P-VAR1 presets/snapshots foundation.
```

These should stay after C1.19 unless they are needed to unblock the live compound runtime surface.
