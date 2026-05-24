#pragma once

#include <string>

namespace myworld
{
struct SilenceDetectorParameters
{
    double floor = 0.02;
    double holdMs = 300.0;
    double releaseMs = 100.0;
    double outputSmoothMs = 0.0;
};

struct SilenceDetectorFrameInput
{
    bool hasRms = false;
    double rms = 0.0;
    bool hasPeak = false;
    double peak = 0.0;
    double timeMs = 0.0;
};

struct SilenceDetectorFrameOutput
{
    bool detectorOk = true;
    bool silenceState = false;
    double silenceTimerMs = 0.0;
    double confidence = 0.0;
    std::string diagnostic;
};

class SilenceDetector
{
public:
    explicit SilenceDetector (SilenceDetectorParameters parameters);

    SilenceDetectorFrameOutput processFrame (const SilenceDetectorFrameInput& input);

private:
    SilenceDetectorParameters parameters;
    double silenceTimerMs = 0.0;
    double lastFrameTimeMs = 0.0;
    bool hasLastFrameTime = false;
};
}
