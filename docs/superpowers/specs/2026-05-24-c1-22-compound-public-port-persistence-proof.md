# C1.22 Compound Public-Port Persistence Proof

Date: 2026-05-24 02:18 Asia/Taipei

## Behavior

```text
root compound public-port edges
-> serialize/deserialize interaction state
-> editorGraph/runtimeGraph preserve endpoints
-> disconnect undo/redo keeps command history honest
```

## Acceptance Trace

Behavior fixture:

```text
fixtures/interaction/tooll3-t0-t7.behavior.json
-> compound public ports persist undo
-> create audio.input, compound.loudness, io.midi.cc_out
-> connect live_audio.channels -> loud1.audio.in
-> connect loud1.out -> midi1.value
-> save_work:saved-and-committed
-> deserializeInteractionState()
-> editorGraph and runtimeGraph both keep public input/output edges
-> disconnect public output edge
-> undo restores it
-> redo removes it again
```

## Proven

```text
fixtures/interaction/tooll3-t0-t7.behavior.json
source/core/InteractionContract.cpp
tests/InteractionTraceTests.cpp
```

## Limits

```text
This proves root public-port edge persistence and undo/redo, not child layout persistence.
It uses the current interaction state serializer, not the future project patch document format.
Visual port grouping and mapping-editor affordances remain parked.
```

## Follow-On

```text
C1.23 expanded child layout persistence proof is now implemented:
user-moved expanded child node
-> per compound instance layout snapshot
-> exit/re-enter expanded patch keeps child position
-> root public-port edges remain intact
```
