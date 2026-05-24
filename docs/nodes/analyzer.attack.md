# Attack

Use `compound.attack` when a patch needs a trusted entry event from raw energy.

## Inputs

- `audio.in` (`audio.channels`): audio stream to fold into a mono analysis lane.

## Outputs

- `onset_event` (`event.trigger`): one-frame event when a trusted rising edge is detected.
- `attack_value` (`signal.float`): bounded strength of the detected entry.
- `attack_envelope` (`signal.float`): detector state that decays after an onset.
- `confidence` (`signal.float`): trust signal for the detector decision.

## Use

- Feed future `density` with `onset_event`.
- Drive later visual or MIDI mapping with `attack_value`, not detector truth.

## Failure Modes

- `attack_envelope` is not the event source for density.
- UI smoothing must not feed back into the detector.
- Missing RMS produces no onset and reports `missing_input:rms`.
