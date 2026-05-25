# Gemini Risk Triage Hardening

## Status

Closed on 2026-05-25.

## Trigger

External review raised concerns about OSC packet over-read, packet truncation, OpenGL render-thread blocking, hardcoded local paths, path-resolution search depth, and Live IO blocking sends.

## Closed Fixes

- OSC datagram string reads now use the bounded decoder in `LiveIOOscReceiver`.
- `LiveIOProofRunner` no longer constructs OSC address/type strings from null-terminated `reinterpret_cast` pointers.
- `LiveIOSendAdapterTests` also verifies loopback datagrams through the bounded decoder instead of null-terminated pointer reads.
- Malformed OSC decoder tests cover missing null termination, truncated type tag, and truncated float payload.
- OSC receive/proof loopback buffers use a 65536-byte datagram buffer instead of 1024/256-byte buffers.
- The app OpenGL proof dump captures GL frames on the render thread but writes V1 proof artifacts on a background thread.
- Source and CMake no longer contain user-specific `/Users/chenbaiwei/...` hardcoded paths.

## Target Line

```text
malformed OSC packet
-> bounded decode
-> failed packet status
-> no null-terminated over-read

OpenGL proof dump
-> capture frames on GL thread
-> write artifacts on background thread
-> status callback on message thread
```

## Evidence

- `source/core/LiveIOOscReceiver.cpp`
- `source/app/LiveIOProofRunner.cpp`
- `source/render/OpenGLShaderPreview.cpp`
- `source/ui/ImGuiSmokeOverlayHelpers.cpp`
- `tests/LiveIOOscReceiverTests.cpp`
- `tests/LiveIOSendAdapterTests.cpp`
- `CMakeLists.txt`

## Risk Decisions

- `LiveIOOscReceiver` was already bounded for string reads; the risky copy was in proof-runner readback code.
- Full OSC bundle support remains parked. Unsupported or malformed packets fail closed instead of being partially trusted.
- Blocking OSC/MIDI sends remain outside the audio callback. `AudioInputAnalyzer` still only analyzes, publishes realtime snapshots, and clears output buffers.
- `resolvePathNearWithReport` remains unchanged because it has a bounded default parent depth and does not recursively scan directory trees.
- Replacing all hand-written JSON with a library is parked; this lane only fixed the concrete packet-safety and portability risks.

## Verification

- Focused build/tests:
  `cmake --build build --target my-world my_world_live_io_send_adapter_tests my_world_live_io_osc_receiver_tests my_world_live_io_proof_runner_tests`
- Focused test run:
  `./build/my_world_live_io_send_adapter_tests && ./build/my_world_live_io_osc_receiver_tests && ./build/my_world_live_io_proof_runner_tests`
- App proof smoke:
  `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world perl -e 'alarm shift; exec @ARGV' 20 ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit`
- Full suite:
  `ctest --test-dir build --output-on-failure`
- Diff check:
  `git diff --check`

## Parked

- Full OSC bundle parser.
- Replacing all manual JSON report writers.
- Non-loopback external OSC send UI/lifecycle.
- Dedicated proof-writer job system.
- CI-provided JUCE dependency bootstrap.
