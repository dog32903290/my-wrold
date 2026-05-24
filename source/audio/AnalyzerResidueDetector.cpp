#include "AnalyzerResidueDetector.h"

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
}

ResidueDetector::ResidueDetector (ResidueDetectorParameters newParameters)
    : parameters (newParameters)
{
}

ResidueDetectorFrameOutput ResidueDetector::processFrame (const ResidueDetectorFrameInput& input)
{
    ResidueDetectorFrameOutput output;

    if (! input.hasRms || ! std::isfinite (input.rms))
    {
        output.detectorOk = false;
        output.diagnostic = "missing_input:rms";
        output.residueEnvelope = residueEnvelope;
        output.residueTimerMs = residueTimerMs;
        return output;
    }

    if (! input.hasSustainEnvelope || ! std::isfinite (input.sustainEnvelope))
    {
        output.detectorOk = false;
        output.diagnostic = "missing_input:sustain_envelope";
        output.residueEnvelope = residueEnvelope;
        output.residueTimerMs = residueTimerMs;
        return output;
    }

    const auto elapsedMs = hasLastFrameTime ? std::max (0.0, input.timeMs - lastFrameTimeMs) : 0.0;
    const auto active = std::max (0.0, input.rms) > parameters.floor;
    const auto silenceClear = parameters.silenceClears && input.hasSilenceState && input.silenceState;
    const auto sustainEnvelope = clamp01 (input.sustainEnvelope);

    if (silenceClear)
    {
        armedEnvelope = 0.0;
        residueEnvelope = 0.0;
        residueTimerMs = 0.0;
    }
    else if (active)
    {
        armedEnvelope = std::max (armedEnvelope, sustainEnvelope);
        residueEnvelope = 0.0;
        residueTimerMs = 0.0;
    }
    else if (armedEnvelope >= parameters.armThreshold)
    {
        if (residueEnvelope <= 0.0)
            residueEnvelope = armedEnvelope;

        residueTimerMs += elapsedMs;

        if (parameters.decayMs > 0.0 && elapsedMs > 0.0)
            residueEnvelope *= std::max (0.0, 1.0 - (elapsedMs / parameters.decayMs));
        else if (parameters.decayMs <= 0.0)
            residueEnvelope = 0.0;
    }
    else
    {
        residueEnvelope = 0.0;
        residueTimerMs = 0.0;
    }

    output.residueEnvelope = clamp01 (residueEnvelope);
    output.residueTimerMs = residueTimerMs;
    output.residueState = ! active && ! silenceClear && output.residueEnvelope >= parameters.residueThreshold;
    output.confidence = 1.0;

    lastFrameTimeMs = input.timeMs;
    hasLastFrameTime = true;
    return output;
}
}
