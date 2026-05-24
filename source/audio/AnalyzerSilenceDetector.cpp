#include "AnalyzerSilenceDetector.h"

#include <algorithm>
#include <cmath>

namespace myworld
{
SilenceDetector::SilenceDetector (SilenceDetectorParameters newParameters)
    : parameters (newParameters)
{
}

SilenceDetectorFrameOutput SilenceDetector::processFrame (const SilenceDetectorFrameInput& input)
{
    SilenceDetectorFrameOutput output;

    if (! input.hasRms || ! std::isfinite (input.rms))
    {
        output.detectorOk = false;
        output.diagnostic = "missing_input:rms";
        output.silenceTimerMs = silenceTimerMs;
        return output;
    }

    const auto elapsedMs = hasLastFrameTime ? std::max (0.0, input.timeMs - lastFrameTimeMs) : 0.0;
    const auto quiet = std::max (0.0, input.rms) <= parameters.floor;

    if (quiet)
        silenceTimerMs += elapsedMs;
    else
        silenceTimerMs = 0.0;

    output.silenceTimerMs = silenceTimerMs;
    output.silenceState = silenceTimerMs >= parameters.holdMs;
    output.confidence = 1.0;

    lastFrameTimeMs = input.timeMs;
    hasLastFrameTime = true;
    return output;
}
}
