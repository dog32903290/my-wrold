# C1.23 Expanded Child Layout Persistence Proof

Date: 2026-05-24 02:25 Asia/Taipei

## Behavior

```text
user-moved expanded child node
-> store layout on the root compound instance
-> serialize/deserialize root graph
-> re-enter expanded patch
-> child position is restored
```

## Acceptance Trace

Core interaction:

```text
makeCompoundPatchInteractionGraph(spec, "library_loud1")
-> drag library_loud1/mono_mix in expanded graph
-> storeExpandedPatchLayout(rootSession, "library_loud1", expandedGraph)
-> commandLog contains store_expanded_patch_layout
-> serialize/deserialize rootSession
-> makeCompoundPatchInteractionGraph(spec, "library_loud1", restoredRoot.graph)
-> library_loud1/mono_mix position matches the user-moved expanded graph
-> root public-port edges remain intact
```

Visible app path:

```text
inside expanded patch canvas
-> Exit
-> storeExpandedPatchLayout(interactionSession, parentNodeId, expandedPatchSession.graph)
-> exitPatch(interactionSession)
-> next enter uses makeCompoundPatchInteractionGraph(spec, parentNodeId, interactionSession.graph)
```

## Proven

```text
source/core/CompoundPatch.h
source/core/CompoundPatch.cpp
source/core/InteractionContract.h
source/core/InteractionContract.cpp
source/ui/ImGuiSmokeOverlay.cpp
tests/CompoundInteractionTests.cpp
```

## Limits

```text
Layout is stored as parent node params in the interaction graph, not yet as a polished patch-document schema.
Only child node positions are persisted; expanded view pan/zoom is still session-local.
No visual mapping editor exists yet for public port mapping.
```

## Next Line

```text
C1.24 compound proof closure:
C1.1-C1.23 evidence
-> close first loudness compound proof status
-> identify remaining C2 work outside the first C1 spine
-> keep raw callback-buffer runtime and visual polish parked
```
