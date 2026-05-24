# Aggregate Pressure

Use `compound.aggregate-pressure` when a patch needs a bounded pressure value made from trusted raw and detector-state lanes.

## Inputs

- `audio.in` (`audio.channels`): audio stream to fold into a mono analysis lane.

## Outputs

- `pressure_value` (`signal.float`): bounded 0..1 aggregate pressure.
- `energy_component` (`signal.float`): weighted raw RMS contribution.
- `attack_component` (`signal.float`): weighted attack contribution.
- `density_component` (`signal.float`): weighted density contribution.
- `sustain_component` (`signal.float`): weighted sustain contribution.
- `residue_component` (`signal.float`): weighted residue contribution.
- `confidence` (`signal.float`): trust signal for the aggregate value.

## Use

- Feed later MIDI/shader/UI mapping from a single pressure lane.
- Debug the pressure value by reading each weighted component.

## Failure Modes

- Aggregate pressure is not output shaping; MIDI and shader ranges live later.
- Hidden weights make pressure arbitrary, so each component is public/debuggable.
- Missing required source signals report `missing_input:<port>`.
