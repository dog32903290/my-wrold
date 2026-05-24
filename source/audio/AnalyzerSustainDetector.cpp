#include "AnalyzerSustainDetector.h"

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

SustainDetector::SustainDetector (SustainDetectorParameters newParameters)
    : parameters (newParameters)
{
}

SustainDetectorFrameOutput SustainDetector::processFrame (const SustainDetectorFrameInput& input)
{
    SustainDetectorFrameOutput output;

    if (! input.hasRms || ! std::isfinite (input.rms))
    {
        output.detectorOk = false;
        output.diagnostic = "missing_input:rms";
        output.sustainTimerMs = sustainTimerMs;
        output.sustainEnvelope = sustainEnvelope;
        return output;
    }

    const auto elapsedMs = hasLastFrameTime ? std::max (0.0, input.timeMs - lastFrameTimeMs) : 0.0;
    const auto active = std::max (0.0, input.rms) > parameters.floor;

    if (active)
    {
        sustainTimerMs += elapsedMs;
        sustainEnvelope = parameters.holdMs <= 0.0 ? 1.0 : clamp01 (sustainTimerMs / parameters.holdMs);
    }
    else
    {
        sustainTimerMs = 0.0;
        if (parameters.releaseMs > 0.0 && elapsedMs > 0.0)
            sustainEnvelope *= std::max (0.0, 1.0 - (elapsedMs / parameters.releaseMs));
        else if (parameters.releaseMs <= 0.0)
            sustainEnvelope = 0.0;
    }

    output.sustainTimerMs = sustainTimerMs;
    output.sustainState = active && sustainTimerMs >= parameters.holdMs;
    output.sustainEnvelope = output.sustainState ? 1.0 : sustainEnvelope;
    output.confidence = 1.0;

    lastFrameTimeMs = input.timeMs;
    hasLastFrameTime = true;
    return output;
}
}
