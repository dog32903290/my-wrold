# Gemini Risk Pass

Date: 2026-05-22

Gemini's feedback is mostly valid, but it does not overturn the current skeleton. It turns several parked risks into gates.

## Decisions

### OpenGL

Initial OpenGL/GLSL stays because it already proves the native shader loop and keeps GLSL study fast.

It is now proof-only. Before high-resolution previews, compute-heavy graphs, or performance claims, `OpenGLShaderPreview` must be extracted behind `RenderBackend`, then the production backend choice must be pressed between Metal, WGPU, and bgfx.

### C# Worker IPC

File exchange is acceptable for batch fixtures, CI, proof dumps, and offline code generation.

It is not acceptable for live drag-time feedback. A future interactive C# compiler worker needs named pipes, gRPC, ZeroMQ, or shared memory. It still cannot enter realtime audio callbacks, render hot paths, or app lifecycle ownership.

### Audio Graph Mutation

A1 only measures audio and publishes bounded analyzer snapshots. It does not prove dynamic audio graph mutation.

Live connect/disconnect of audio nodes is parked until runtime graph rebuilds happen off the audio thread and swap through a realtime-safe prepared snapshot or queue. The callback must not allocate, lock, log, parse, or rebuild graph state.

### Command+S Git Commit

Atomic file save can happen on the save path. Git add/commit must be background work.

The first UI status after a dirty save can be `save-ok commit-pending`; the background worker later records `saved-and-committed` or `save-ok commit-failed`.

### Tooll3 Style And Node Previews

Tooll3 becomes the primary visual style reference: compact ImGui nodes, dense technical panels, previews/parameters/timeline/node graph rhythm.

Node previews are required for visual debugging, but they must display cached runtime/debug outputs. ImGui drawing a node must not trigger expensive cooks.

### Parameter Override UX

`PortBinding` modes are not enough by themselves. The UI must visibly distinguish `default`, `manual`, `connected`, and `animated`.

Connected or animated values must show their source and must not pretend a manual slider drag changed the live overridden value.

### macOS Microphone Permission

A1 cannot request live input until the app bundle has `NSMicrophoneUsageDescription`.

The CMake target now sets JUCE `MICROPHONE_PERMISSION_ENABLED` and `MICROPHONE_PERMISSION_TEXT`; verification should inspect the built app's `Info.plist` before claiming live audio proof.

