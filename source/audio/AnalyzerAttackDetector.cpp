#include "AnalyzerAttackDetector.h"

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

AttackDetector::AttackDetector (AttackDetectorParameters newParameters)
    : parameters (newParameters)
{
}

AttackDetectorFrameOutput AttackDetector::processFrame (const AttackDetectorFrameInput& input)
{
    AttackDetectorFrameOutput output;

    if (! input.hasRms || ! std::isfinite (input.rms))
    {
        output.detectorOk = false;
        output.diagnostic = "missing_input:rms";
        output.attackEnvelope = attackEnvelope;
        return output;
    }

    if (input.hasSampleCount && (! std::isfinite (input.sampleCount) || input.sampleCount <= 0.0))
    {
        output.detectorOk = false;
        output.diagnostic = "invalid_sample_count";
        output.currentRms = input.rms;
        output.previousRms = previousRms;
        output.attackEnvelope = attackEnvelope;
        return output;
    }

    const auto elapsedMs = hasPreviousRms ? std::max (0.0, input.timeMs - lastFrameTimeMs) : 0.0;
    if (parameters.releaseMs > 0.0 && elapsedMs > 0.0)
        attackEnvelope *= std::max (0.0, 1.0 - (elapsedMs / parameters.releaseMs));

    const auto currentRms = std::max (0.0, input.rms);
    const auto baselineRms = hasPreviousRms ? previousRms : currentRms;
    const auto delta = currentRms - baselineRms;
    const auto trusted = currentRms >= parameters.floor;
    const auto gatedDelta = trusted ? delta : 0.0;
    const auto debounceActive = lastOnsetTimeMs >= 0.0
                                && (input.timeMs - lastOnsetTimeMs) < parameters.debounceMs;
    const auto onset = trusted && gatedDelta >= parameters.riseThreshold && ! debounceActive;
    const auto clipped = input.hasPeak && input.peak > 1.0;

    output.currentRms = currentRms;
    output.previousRms = baselineRms;
    output.delta = delta;
    output.gatedDelta = gatedDelta;
    output.confidence = trusted ? (clipped ? 0.5 : 1.0) : 0.0;

    if (clipped)
        output.warnings.push_back ("clipped_peak");

    if (onset)
    {
        output.onsetEvent = true;
        output.attackValue = clamp01 (gatedDelta);
        attackEnvelope = output.attackValue;
        lastOnsetTimeMs = input.timeMs;
    }

    output.attackEnvelope = attackEnvelope;
    previousRms = currentRms;
    lastFrameTimeMs = input.timeMs;
    hasPreviousRms = true;
    return output;
}
}
