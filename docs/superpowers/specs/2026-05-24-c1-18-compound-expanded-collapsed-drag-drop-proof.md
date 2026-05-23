# C1.18 Compound Expanded/Collapsed Drag-Drop Proof

Date: 2026-05-24 01:45 Asia/Taipei

## Behavior

```text
loaded compound node
-> collapsed root canvas drag/move/select
-> enter expanded child patcher view
-> child patcher graph uses parent-qualified node ids
-> child drag and state roundtrip preserve graph evidence
```

## Acceptance Trace

Collapsed root canvas:

```text
load default module library
-> create compound.loudness as library_loud1 through visible registry
-> collapse library_loud1
-> CanvasHands drag library_loud1
-> commandLog contains move_node
-> library_loud1 remains selected and collapsed
-> serialize/deserialize preserves position, collapsed state, and currentPatchPath after enter_patch
```

Expanded child patcher canvas:

```text
fixtures/compounds/loudness.compound.json
-> makeCompoundPatchInteractionGraph(spec, "library_loud1")
-> child node ids become library_loud1/<child>
-> child-to-child internal edges become parent-qualified endpoints
-> validateGraphInvariants passes against seed NodeSpecs
-> CanvasHands drag library_loud1/mono_mix
-> serialize/deserialize preserves child position and internal edges
```

Behavior trace:

```text
fixtures/interaction/tooll3-t0-t7.behavior.json
-> compound collapsed drag expanded roundtrip
-> create_node, collapse_compound, move_node, enter_patch, save_work:saved-and-committed
```

## Proven

```text
tests/CompoundInteractionTests.cpp
source/core/CompoundPatch.cpp
source/core/InteractionContract.cpp
source/ui/ImGuiSmokeOverlay.cpp
fixtures/interaction/tooll3-t0-t7.behavior.json
```

## Limits

```text
Expanded child patcher layout is deterministic proof layout, not saved user layout per compound instance yet.
Expanded patcher create-node search is parked; C1.18 only proves child graph visibility, drag, internal edges, and roundtrip.
The live A1 analyzer still runs direct analyzer code while exporting matching C1 runtime evidence.
```

## Next Line

```text
C1.19 loaded loudness runtime bridge:
loaded compound runtime publicOutputs
-> live analyzer/debug surface reads the same loudness output vocabulary
-> direct A1 analyzer path remains a fallback, not the only truth
```
