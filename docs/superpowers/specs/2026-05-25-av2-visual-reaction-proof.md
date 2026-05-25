# AV2 Visual Reaction Proof

## Status

Closed on 2026-05-25.

## Acceptance

- V1 proof artifacts can receive a quiet frame and a loud frame.
- When the frames share dimensions, the proof writes `visual_reaction.json`.
- The artifact records quiet/loud loudness, loudness delta, dimensions, changed pixel count, and mean absolute pixel delta.
- Existing V1 proof artifacts remain compatible when no reaction frame pair is supplied.
- No real OpenGL offscreen frame pair, shader preview live binding, graph node mapping, full mapping editor, or Metal work is added.

## Target Line

```text
quiet/loud proof frames
-> V1ShaderProofArtifacts
-> visual_reaction.json
-> changedPixels + meanAbsDelta
```

## Evidence

- `source/render/V1ShaderProofArtifacts.h`
- `source/render/V1ShaderProofArtifacts.cpp`
- `tests/V1ShaderProofArtifactsTests.cpp`

## Contract Notes

AV2 proves the artifact layer can detect visual reaction from a loudness pair. It does not claim the app has rendered a true OpenGL before/after pair yet. That is the next AV lane.

The proof compares RGB channels per pixel and reports:

- `changedPixels`
- `meanAbsDelta`
- `quietLoudness`
- `loudLoudness`
- `loudnessDelta`

## Verification

- RED: `cmake --build build --target my_world_v1_shader_proof_artifacts_tests && ./build/my_world_v1_shader_proof_artifacts_tests` failed first because `V1ShaderProofArtifactRequest` had no quiet/loud frame fields.
- GREEN: `cmake --build build --target my_world_v1_shader_proof_artifacts_tests && ./build/my_world_v1_shader_proof_artifacts_tests`
- App build: `cmake --build build --target my-world`
- Full suite: `ctest --test-dir build --output-on-failure` passed 77/77.
- Diff check: `git diff --check` passed.

## Parked

- AV3 real OpenGL capture pair.
- Shader preview live binding proof.
- Graph node mapping.
- Full mapping editor.
- Metal backend.
