#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct AttackDetectorParameters
{
    double floor = 0.02;
    double riseThreshold = 0.08;
    double debounceMs = 80.0;
    double releaseMs = 120.0;
    double outputSmoothMs = 0.0;
};

struct AttackDetectorFrameInput
{
    bool hasRms = false;
    double rms = 0.0;
    bool hasPeak = false;
    double peak = 0.0;
    bool hasSampleCount = false;
    double sampleCount = 0.0;
    double timeMs = 0.0;
};

struct AttackDetectorFrameOutput
{
    bool detectorOk = true;
    bool onsetEvent = false;
    double attackValue = 0.0;
    double attackEnvelope = 0.0;
    double confidence = 0.0;
    double currentRms = 0.0;
    double previousRms = 0.0;
    double delta = 0.0;
    double gatedDelta = 0.0;
    std::string diagnostic;
    std::vector<std::string> warnings;
};

class AttackDetector
{
public:
    explicit AttackDetector (AttackDetectorParameters parameters);

    AttackDetectorFrameOutput processFrame (const AttackDetectorFrameInput& input);

private:
    AttackDetectorParameters parameters;
    bool hasPreviousRms = false;
    double previousRms = 0.0;
    double lastFrameTimeMs = 0.0;
    double lastOnsetTimeMs = -1.0;
    double attackEnvelope = 0.0;
};
}
