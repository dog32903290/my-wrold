#include "AnalyzerDensityDetector.h"

#include <algorithm>

namespace myworld
{
namespace
{
double clamp01 (const double value)
{
    return std::clamp (value, 0.0, 1.0);
}
}

DensityDetector::DensityDetector (DensityDetectorParameters newParameters)
    : parameters (newParameters)
{
}

DensityDetectorFrameOutput DensityDetector::processFrame (const DensityDetectorFrameInput& input)
{
    DensityDetectorFrameOutput output;

    if (! input.hasOnsetEvent)
    {
        output.detectorOk = false;
        output.diagnostic = "missing_input:onset_event";
        output.densityEnvelope = densityEnvelope;
        return output;
    }

    const auto elapsedMs = hasLastFrameTime ? std::max (0.0, input.timeMs - lastFrameTimeMs) : 0.0;
    if (parameters.releaseMs > 0.0 && elapsedMs > 0.0)
        densityEnvelope *= std::max (0.0, 1.0 - (elapsedMs / parameters.releaseMs));

    eventTimesMs.erase (std::remove_if (eventTimesMs.begin(),
                                        eventTimesMs.end(),
                                        [&input, this] (const auto eventTime) {
                                            return input.timeMs - eventTime > parameters.windowMs;
                                        }),
                        eventTimesMs.end());

    if (input.onsetEvent)
        eventTimesMs.push_back (input.timeMs);

    output.eventCount = static_cast<int> (eventTimesMs.size());
    output.densityValue = parameters.maxEvents <= 0.0 ? 0.0
                                                      : clamp01 (static_cast<double> (output.eventCount)
                                                                 / parameters.maxEvents);
    densityEnvelope = std::max (densityEnvelope, output.densityValue);
    output.densityEnvelope = densityEnvelope;
    output.confidence = 1.0;

    lastFrameTimeMs = input.timeMs;
    hasLastFrameTime = true;
    return output;
}
}
