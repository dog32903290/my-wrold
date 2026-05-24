# Silence

Use `compound.silence` when a patch needs to know whether raw energy has stayed quiet long enough to trust as silence.

## Inputs

- `audio.in` (`audio.channels`): audio stream to fold into a mono analysis lane.

## Outputs

- `silence_state` (`signal.float`): 1 when quiet has held past the detector hold time.
- `silence_timer_ms` (`signal.float`): how long raw RMS has stayed below the silence floor.
- `confidence` (`signal.float`): trust signal for the silence decision.

## Use

- Gate later detectors that need a stable no-input state.
- Explain when visuals or event detectors should stop believing small room noise.

## Failure Modes

- Silence is not just "low loudness"; it is sustained raw RMS below a floor.
- A short gap is not silence until `holdMs` is reached.
- Missing RMS reports `missing_input:rms`.
