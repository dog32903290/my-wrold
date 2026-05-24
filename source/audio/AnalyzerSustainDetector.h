#pragma once

#include <string>

namespace myworld
{
struct SustainDetectorParameters
{
    double floor = 0.02;
    double holdMs = 300.0;
    double releaseMs = 200.0;
    double outputSmoothMs = 0.0;
};

struct SustainDetectorFrameInput
{
    bool hasRms = false;
    double rms = 0.0;
    bool hasPeak = false;
    double peak = 0.0;
    double timeMs = 0.0;
};

struct SustainDetectorFrameOutput
{
    bool detectorOk = true;
    bool sustainState = false;
    double sustainTimerMs = 0.0;
    double sustainEnvelope = 0.0;
    double confidence = 0.0;
    std::string diagnostic;
};

class SustainDetector
{
public:
    explicit SustainDetector (SustainDetectorParameters parameters);

    SustainDetectorFrameOutput processFrame (const SustainDetectorFrameInput& input);

private:
    SustainDetectorParameters parameters;
    double sustainTimerMs = 0.0;
    double sustainEnvelope = 0.0;
    double lastFrameTimeMs = 0.0;
    bool hasLastFrameTime = false;
};
}
