# Sustain

Use `compound.sustain` when a patch needs to know whether raw energy has stayed active long enough to trust as a sustained sound.

## Inputs

- `audio.in` (`audio.channels`): audio stream to fold into a mono analysis lane.

## Outputs

- `sustain_state` (`signal.float`): 1 when raw RMS has stayed above the sustain floor for the hold time.
- `sustain_timer_ms` (`signal.float`): how long raw RMS has stayed above the sustain floor.
- `sustain_envelope` (`signal.float`): held-energy state envelope for later mapping.
- `confidence` (`signal.float`): trust signal for the sustain decision.

## Use

- Separate held energy from short attacks.
- Feed later visual/MIDI mapping without making UI smoothing part of detector truth.

## Failure Modes

- Sustain is not loudness; it is raw RMS held above a floor for `holdMs`.
- A short note is not sustain until `holdMs` is reached.
- Missing RMS reports `missing_input:rms`.
