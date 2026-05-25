# AV4 App Dump Visual Reaction Readback

## Status

Closed on 2026-05-25.

## Acceptance

- The built app can run `--dump-proof-and-exit` and quit cleanly.
- The app proof dump writes `debug/v1-shader-proof/visual_reaction.json`.
- The JSON reports `kind: v1VisualReactionProof`, `ok: true`, `status: changed`, non-zero `changedPixels`, and non-zero `meanAbsDelta`.
- The proof remains outside git history; the committed evidence is the readback spec and master plan state.
- No graph node mapping, mapping editor, offscreen GL harness, realtime callback send, or Metal backend work is added.

## Target Line

```text
my-world --dump-proof-and-exit
-> debug/v1-shader-proof/visual_reaction.json
-> ok true / changed / non-zero pixel delta
```

## Evidence

- App command:
  `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit`
- Readback file:
  `debug/v1-shader-proof/visual_reaction.json`

## Readback Snapshot

```json
{
  "kind": "v1VisualReactionProof",
  "ok": true,
  "status": "changed",
  "quietLoudness": 0.000000,
  "loudLoudness": 1.000000,
  "loudnessDelta": 1.000000,
  "width": 2832,
  "height": 1532,
  "pixelCount": 4338624,
  "changedPixels": 4147429,
  "meanAbsDelta": 0.071055
}
```

## Verification

- App dump: `MY_WORLD_PROJECT_DIR=/Users/chenbaiwei/Projects/my-world perl -e 'alarm shift; exec @ARGV' 20 ./build/my-world_artefacts/我的世界.app/Contents/MacOS/我的世界 --dump-proof-and-exit`
- Artifact list contains `visual_reaction.json`.
- Readback confirms `ok: true`, `status: changed`, `changedPixels: 4147429`, and `meanAbsDelta: 0.071055`.
- `git status --short` stayed clean after the ignored proof dump.

## Parked

- Headless/offscreen OpenGL capture harness.
- Graph IO node mapping.
- Full mapping editor.
- Direct realtime callback send.
- Metal backend.
