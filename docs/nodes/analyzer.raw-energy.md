# Raw Energy

Use `compound.raw-energy` when a patch needs raw analyzer facts before loudness, gate, smoothing, or detector decisions.

## Inputs

- `audio.in` (`audio.channels`): audio stream to fold into a mono analysis lane.

## Outputs

- `rms` (`signal.float`): measured mono RMS.
- `peak` (`signal.float`): measured mono peak.
- `sampleCount` (`signal.float`): analyzed window length in the current proof runtime.

## Use

- Feed future analyzer detectors with raw facts.
- Inspect whether a later detector is lying because the raw measurement already drifted.

## Failure Modes

- Raw RMS is not semantic loudness.
- `sampleCount` is diagnostic evidence in C6.1, not a musical window contract yet.
