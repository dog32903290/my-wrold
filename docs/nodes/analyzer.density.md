# Density

Use `compound.density` when a patch needs a recent count of trusted entry events.

## Inputs

- `onset_event` (`event.trigger`): event stream from `attack`.
- `attack_value` (`signal.float`): optional strength attached to the event.

## Outputs

- `density_value` (`signal.float`): bounded event density for the current analysis window.
- `event_count` (`signal.float`): raw count of onset events still inside the window.
- `density_envelope` (`signal.float`): short state value that can decay after events leave the window.
- `confidence` (`signal.float`): trust signal for the density decision.

## Use

- Count trusted `attack.onset_event` entries over a short window.
- Feed later aggregate pressure or visual response with event density.

## Failure Modes

- Do not feed `attack_envelope` into density as detector truth.
- Window length is analysis behavior, not UI smoothing.
- Missing onset input reports `missing_input:onset_event`.
