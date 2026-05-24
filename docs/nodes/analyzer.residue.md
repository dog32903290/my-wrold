# Residue

Use `compound.residue` when a patch needs to know whether a held sound has left a tail after raw RMS falls below the active floor.

## Inputs

- `audio.in` (`audio.channels`): audio stream to fold into a mono analysis lane.

## Outputs

- `residue_state` (`signal.float`): 1 while a previously armed tail remains.
- `residue_envelope` (`signal.float`): bounded 0..1 tail amount for later mapping.
- `residue_timer_ms` (`signal.float`): time since the armed sound fell below the active floor.
- `confidence` (`signal.float`): trust signal for the residue decision.

## Use

- Keep tail/memory separate from attack, density, sustain, silence, and aggregate pressure.
- Give future aggregate pressure a named input instead of inventing arbitrary weights.

## Failure Modes

- Residue is not aggregate pressure; it only reports tail state.
- Short unsustained energy should not create residue.
- Missing RMS reports `missing_input:rms`.
- Missing sustain envelope reports `missing_input:sustain_envelope`.
