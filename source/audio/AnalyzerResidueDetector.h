#pragma once

#include <string>

namespace myworld
{
struct ResidueDetectorParameters
{
    double floor = 0.02;
    double armThreshold = 0.6;
    double residueThreshold = 0.1;
    double decayMs = 200.0;
    bool silenceClears = true;
    double outputSmoothMs = 0.0;
};

struct ResidueDetectorFrameInput
{
    bool hasRms = false;
    double rms = 0.0;
    bool hasSustainEnvelope = false;
    double sustainEnvelope = 0.0;
    bool hasSilenceState = false;
    bool silenceState = false;
    double timeMs = 0.0;
};

struct ResidueDetectorFrameOutput
{
    bool detectorOk = true;
    bool residueState = false;
    double residueEnvelope = 0.0;
    double residueTimerMs = 0.0;
    double confidence = 0.0;
    std::string diagnostic;
};

class ResidueDetector
{
public:
    explicit ResidueDetector (ResidueDetectorParameters parameters);

    ResidueDetectorFrameOutput processFrame (const ResidueDetectorFrameInput& input);

private:
    ResidueDetectorParameters parameters;
    double armedEnvelope = 0.0;
    double residueEnvelope = 0.0;
    double residueTimerMs = 0.0;
    double lastFrameTimeMs = 0.0;
    bool hasLastFrameTime = false;
};
}
