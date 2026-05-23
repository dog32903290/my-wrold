# C1.21 Compound Public-Port Surface Proof

Date: 2026-05-24 02:13 Asia/Taipei

## Behavior

```text
collapsed loaded compound node
-> public input/output ports from loaded NodeSpec registry
-> command-backed connect/disconnect at root canvas
-> expanded child graph remains parent-qualified
```

## Acceptance Trace

Root canvas public ports:

```text
load default module library
-> merge loaded compound NodeSpec into visible registry
-> create/collapse library_loud1 compound.loudness
-> portCenter(library_loud1.audio.in) succeeds
-> portCenter(library_loud1.out) succeeds
-> connect live_audio.channels -> library_loud1.audio.in through loaded registry
-> connect library_loud1.out -> midi_loudness.value through loaded registry
-> edge dataTypes remain audio.channels and signal.float
```

Interaction hands/UI:

```text
CanvasHands drag connection
-> connectPorts(session, visibleRegistry, from, to)
ImGui port release gesture
-> connectPorts(canvasSession, nodeSpecs, from, to)
```

## Proven

```text
source/core/InteractionContract.h
source/core/InteractionContract.cpp
source/core/CanvasHands.cpp
source/ui/ImGuiSmokeOverlay.cpp
tests/CompoundInteractionTests.cpp
```

## Limits

```text
Collapsed compound ports are command/hit-test backed; deeper visual polish for labels and port grouping is still a skin line.
Public port connect does not yet open an inline mapping editor; it trusts loaded NodeSpec public ports.
Expanded child patcher create-node search remains parked.
```

## Next Line

```text
C1.22 compound public-port persistence proof:
root compound public-port edges
-> serialize/deserialize interaction state
-> runtimeGraph preserves the same public endpoints
-> undo/redo disconnect keeps expanded child graph intact
```
