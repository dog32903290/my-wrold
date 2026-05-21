# Tooll3 / TiXL Borrowing Notes

Date: 2026-05-22
Source: `https://github.com/tixl3d/tixl`
Inspected commit: `26dc80c`
License observed: MIT

## Purpose

Tooll3 is a reference system, not a base to fork.

Use it to pressure-test our node, JSON, ImGui, and parameter-binding choices. Do not inherit its C# / DirectX / HLSL / Windows-oriented runtime body.

## Source Anchors

- `Core/Operator/Symbol.cs`
  - `Symbol` is the operator definition. It owns children, connections, input definitions, and output definitions.
  - A symbol can have multiple instances.

- `Core/Operator/Symbol.Child.cs`
  - `Child` is a placed reference to a `Symbol` inside another symbol.
  - This is close to our future `NodeInstance` / child patch instance concept.

- `Core/Operator/Instance.cs`
  - `Instance` is the runtime object with slots, children, parent path, and resource ownership.
  - This maps to our `RuntimeOp` / runtime instance layer, not to our saved graph law.

- `Core/Model/SymbolJson.cs`
  - Serialization separates format/id, inputs, children, connections, settings, and animation.
  - Child input values are written only when they differ from defaults.

- `Core/Operator/Slots/InputSlot.cs`
  - A slot can expose manual/default typed input values, but `GetCurrentValue()` returns connected or animated values when they exist.
  - This is the strongest borrowing point for our `PortBinding` / `ParamBinding` design.

- `Editor/UiModel/Commands/Graph/*`
  - Graph edits are wrapped as undoable commands such as adding a child or adding/deleting a connection.
  - This supports our rule that UI and AI must share command semantics.

## Borrow

### Definition / Placement / Runtime Split

```text
Tooll3 Symbol         -> our NodeSpec / ModuleSpec
Tooll3 Child          -> our NodeInstance / child reference
Tooll3 Instance       -> our RuntimeOp / runtime instance
Tooll3 Connection     -> our typed Edge
```

This confirms the current split is worth keeping:

```text
NodeSpec      definition
NodeInstance  saved placement and editor state
RuntimeOp     executable runtime body
Command       mutation path
```

### JSON Shape, Not JSON Law

Borrow the idea that the graph is readable text with separate sections for:

```text
definitions
children / instances
connections
input values
ui state
animation / collaboration state
```

Do not copy Tooll3's schema. Our schema must carry `Region`, `StreamKind`, `commandGraph`, `collaborationLog`, `saveLog`, and future `graphIR`.

### Parameter Binding

Borrow the live editing pattern:

```text
default value
manual value
connected value
animated value
```

A slider should not disappear when a cable is connected. The manual value remains stored, while the live value can be overridden by a connection or animation.

First native vocabulary:

```text
PortBinding
  bindingMode: default | manual | connected | animated
  dataType
  storedValue
  overrideSource
```

### Command And Undo Shape

Borrow the command discipline, but not the class implementation.

Our version:

```text
UI gesture / AI request
-> commandGraph command
-> validation
-> editorGraph mutation
-> graphIR / runtimeGraph rebuild
-> collaborationLog / undo entry
```

### ImGui Canvas Lessons

Borrow the rendering strategy:

```text
ImGui draw lists
custom connection drawing
zoom / pan canvas transform
hit testing in canvas coordinates
minimal node surface
```

Do not let ImGui ids become saved graph ids. UI ids are disposable; graph ids are law.

## Do Not Borrow

- Do not fork Tooll3 into this repo.
- Do not import its C# runtime model.
- Do not import DirectX / HLSL as the first shader backend.
- Do not import `SymbolPackage` / C# compilation as our module system.
- Do not let UI state own storage truth.
- Do not let JSON diffs bypass command validation.

## Plan Impact

- G0 must include `PortBinding` / parameter binding modes before production parameter UI.
- S0 patch documents must save parameter binding state, not just slider values.
- A0 ImGui work should prove canvas adapter mechanics while graph truth stays serialized and command-driven.
- Future C# tools may inspect or generate `commandGraph` / `graphIR`, but they must stay outside realtime audio/render and native app lifecycle.

