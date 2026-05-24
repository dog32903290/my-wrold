#include "AnalyzerAggregatePressure.h"

#include <algorithm>
#include <cmath>

namespace myworld
{
namespace
{
double clamp01 (const double value)
{
    return std::clamp (value, 0.0, 1.0);
}

double positiveWeight (const double value)
{
    return std::max (0.0, value);
}

bool missingRequired (const bool hasValue, const double value)
{
    return ! hasValue || ! std::isfinite (value);
}

AggregatePressureFrameOutput makeMissingInputOutput (const std::string& port)
{
    AggregatePressureFrameOutput output;
    output.aggregateOk = false;
    output.diagnostic = "missing_input:" + port;
    return output;
}
}

AggregatePressure::AggregatePressure (AggregatePressureParameters newParameters)
    : parameters (newParameters)
{
}

AggregatePressureFrameOutput AggregatePressure::processFrame (const AggregatePressureFrameInput& input) const
{
    if (missingRequired (input.hasRms, input.rms))
        return makeMissingInputOutput ("rms");

    if (missingRequired (input.hasAttackValue, input.attackValue))
        return makeMissingInputOutput ("attack_value");

    if (missingRequired (input.hasDensityValue, input.densityValue))
        return makeMissingInputOutput ("density_value");

    if (missingRequired (input.hasSustainEnvelope, input.sustainEnvelope))
        return makeMissingInputOutput ("sustain_envelope");

    if (missingRequired (input.hasResidueEnvelope, input.residueEnvelope))
        return makeMissingInputOutput ("residue_envelope");

    if (! input.hasSilenceState)
    {
        AggregatePressureFrameOutput output;
        output.aggregateOk = false;
        output.diagnostic = "missing_input:silence_state";
        return output;
    }

    AggregatePressureFrameOutput output;
    output.confidence = 1.0;

    if (parameters.silenceClears && input.silenceState)
        return output;

    const auto energySource = clamp01 (std::max (0.0, input.rms) * std::max (0.0, parameters.energyScale));
    output.energyComponent = energySource * positiveWeight (parameters.energyWeight);
    output.attackComponent = clamp01 (input.attackValue) * positiveWeight (parameters.attackWeight);
    output.densityComponent = clamp01 (input.densityValue) * positiveWeight (parameters.densityWeight);
    output.sustainComponent = clamp01 (input.sustainEnvelope) * positiveWeight (parameters.sustainWeight);
    output.residueComponent = clamp01 (input.residueEnvelope) * positiveWeight (parameters.residueWeight);
    output.pressureValue = clamp01 (output.energyComponent
                                    + output.attackComponent
                                    + output.densityComponent
                                    + output.sustainComponent
                                    + output.residueComponent);
    return output;
}
}
