# Loudness

Use `analyzer.loudness` when you need a stable 0..1-ish signal representing trusted sound energy.

## Inputs

- `input` (`audio.mono`): mono audio measurement stream.

## Outputs

- `out` (`signal.float`): shaped loudness value.
- `rms` (`signal.float`): raw RMS-style energy when exposed by a compound.
- `peak` (`signal.float`): raw peak when exposed by a compound.
- `confidence` (`signal.float`): trust signal when exposed by a compound.

## Use

- Drive shader uniforms such as `u_loudness`.
- Drive MIDI CC values.
- Gate visuals only when the room or instrument is active.

## Failure Modes

- Threshold too high erases soft breath or quiet performance.
- Smooth too high makes the visual response late.
- Treating loudness as a semantic emotion signal will overclaim what the analyzer knows.
