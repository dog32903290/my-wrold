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

## Browser And Search Parity

| Behavior | TiXL witness | Policy | Required My World trace | Blocker |
| --- | --- | --- | --- | --- |
| Empty browser curated page | `SymbolBrowsing.UpdateLibPage`: `Lib` category page | mirror visible labels/path | `category_browser_root_exact_tixl_paths` | canonical visible path fixture |
| Category drilldown | `SymbolBrowsing` path mutation and active namespace results | mirror | `category_browser_drilldown_exact_namespace` | none |
| Essential overview rows | `SymbolTags.Essential` for level-one overview | adapt until `NodeSpec` has essential flag | `category_overview_essential_only` | NodeSpec metadata |
| Full symbol library tree | `SymbolLibrary`, `NamespaceTreeNode` | mirror later as secondary library surface | `symbol_library_namespace_tree_root_order` | full library UI |
| Fuzzy text search | `SymbolFilter`: subsequence regex, namespace/description match | mirror user-visible behavior, not regex bugs | `search_fuzzy_name_namespace_description` | search fixture |
| Ranking | exact, starts-with, contains, PascalCase, package boosts | mirror visible fixture order; adapt scoring internals | `search_ranking_exact_starts_contains_pascal` | usage data optional |
| Output drag to empty canvas | source output type filters candidate inputs | mirror | `compatible_create_from_output_drag` | none |
| Input drag to empty canvas | target input type filters candidate outputs | mirror | `compatible_create_from_input_drag` | none |
| Split existing connection through browser | input/output filters plus old-edge removal and two-edge insert | mirror as one macro command | `compatible_split_connection_insert_node` | reconnect/split trace |
| Keyboard create/cancel | arrows, Return, Escape/click outside | mirror | `browser_keyboard_return_escape_no_mutation_on_cancel` | UI event trace schema |

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
| Reconnect existing input end | `HoldingConnectionEnd`, `InputSnapper` | partial | `reconnect_input_end_one_undo_step` |
| Reconnect existing output beginning | `HoldingConnectionBeginning`, `OutputSnapper` | missing | `reconnect_output_beginning_one_undo_step` |
| Drop connection/node onto operator body and choose hidden input | `InputPicking` | missing | `drop_connection_onto_operator_hidden_input` |
| Multi-input insert before/after/replace | `InputSnapper.InputSnapTypes` | missing | `multi_input_insert_before_after_replace` |
| Split edge by creating operator | legacy `ConnectionMaker.SplitConnectionWithSymbolBrowser` | partial | `split_edge_create_operator_undo_macro` |
| Drag existing node onto edge to insert | `MagItemMovement.TrySplitInsert` | missing | `drag_node_onto_edge_split_insert` |
| Snap move creates connection / unsnap removes connection | `MagItemMovement` snap/unsnap paths | missing | `snap_move_connect_unsnap_disconnect_undo` |
| Shake dragged node to disconnect | `ShakeDetector`, `NodeActions.DisconnectDraggedNodes` | missing | `shake_disconnect_dragged_node_edges` |
| Enter / exit symbol or compound | graph navigation state | proven for compounds | existing compound traces |
| Inspector param and port binding | graph input commands | partial vs TiXL defaults | `reset_default_manual_binding_fallback` |
| Annotation add/drag/resize/rename/delete/collapse | annotation interaction files | parked | `annotation_frame_move_resize_rename_collapse_undo` if in scope |
| Symbol/file/asset drop to create | `DropHandling` | parked | `drop_symbol_creates_node_undo` |

One user action must become one undo unit even when the native implementation lowers it to several commands.

## Parameter, Preset, And Snapshot Parity

| Feature | TiXL witness | Policy | Current status | Required trace |
| --- | --- | --- | --- | --- |
| Parameter inspector shell | `ParameterWindow`, `IInputUi` | mirror | partial | `select_node_inspector_rows_set_param` |
| Row state model | normal, connected, animated, default/reset | mirror | partial | `default_manual_connected_animated_undo` |
| Type color families | `TypeUiProperties` | mirror user families, adapt TypeSpec | partial | `port_type_family_visual_mapping` |
| Scalar/vector controls | float, int, bool, double, vector, quaternion | mirror core widgets | gap | `typed_scalar_vector_param_roundtrip` |
| Enum/string/path/multiline | enum combo/flags, string usage modes | mirror | gap | `typed_text_enum_path_param_roundtrip` |
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

## Output, Timeline, And Live Performance Parity

| Feature | TiXL witness | Policy | Current status | Required trace | Blocker |
| --- | --- | --- | --- | --- | --- |
| Output follows selection unless pinned | `ViewSelectionPinning`, output window state | mirror as UX contract | gap | `select_changes_output_pin_freezes_output_reload` | stable output state |
| Output slot / final eval start pin | output id and eval-start pin | parked | parked | `multi_output_view_pin_final_eval_pin` | multi-output runtimeGraph |
| Image canvas modes | Fit, 1:1, Custom pan/zoom, size/format overlay | mirror soon | partial | `output_fit_1to1_custom_view_state` | RenderBackend extraction |
| Output toolbar | gizmo/grid/camera/background/resolution/screenshot/render settings | mirror screenshot/resolution vocabulary; park camera/gizmo | parked mixed | `screenshot_or_frame_dump_active_output` | 3D/camera phase |
| Resolution presets | Fill, 1:1, aspect ratios, 480p/720p/1080p/4k/8k, custom | mirror later | parked | `requested_resolution_preset_roundtrip` | RenderBackend/output state |
| Render/export window | video/image sequence, Bars/Secs/Frames, FPS, resolution %, motion blur, audio | mirror taxonomy, park implementation | parked | `render_settings_roundtrip_frame_count` | RenderBackend + timeline |
| Render process states | NoOutputWindow, NoValidOutputType, ReadyForExport, Exporting | mirror later | parked | `invalid_output_blocks_render_state` | active output type system |
| Time model/display | bars canonical; seconds/frames are views | mirror when timeline begins | parked | `bars_seconds_frames_conversion_bpm_fps` | timeline model |
| Playback controls | play, reverse, speed, stop/reset, stepping, loop, idle motion, IO indicator | mirror gradually | partial/parked | `transport_play_loop_io_indicator` | timeline + IO bus |
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

The next practical parity proofs are:

```text
P-TAX1 category browser fixture:
Operators/Lib visible taxonomy
-> generated My World browser taxonomy fixture
-> hidden/default rules tested

P-SEARCH1 search and compatible-create fixture:
TiXL-style fuzzy search + namespace match + port-compatible candidate filter
-> NodeSpec registry query
-> create_node/connect command trace

P-OPS1 richer graph-operation trace:
reconnect, split edge, hidden input picker, multi-input ordering, shake disconnect
-> one user gesture = one undo unit

P-OUT1 output pinning:
selection-following output
-> pin output
-> selection changes do not change output
-> save/load preserves pin
```

These should stay after C1.19 unless they are needed to unblock the live compound runtime surface.
