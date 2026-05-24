#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct DensityDetectorParameters
{
    double windowMs = 400.0;
    double maxEvents = 4.0;
    double releaseMs = 200.0;
    double outputSmoothMs = 0.0;
};

struct DensityDetectorFrameInput
{
    bool hasOnsetEvent = false;
    bool onsetEvent = false;
    bool hasAttackValue = false;
    double attackValue = 0.0;
    bool hasAttackEnvelope = false;
    double attackEnvelope = 0.0;
    double timeMs = 0.0;
};

struct DensityDetectorFrameOutput
{
    bool detectorOk = true;
    int eventCount = 0;
    double densityValue = 0.0;
    double densityEnvelope = 0.0;
    double confidence = 0.0;
    std::string diagnostic;
};

class DensityDetector
{
public:
    explicit DensityDetector (DensityDetectorParameters parameters);

    DensityDetectorFrameOutput processFrame (const DensityDetectorFrameInput& input);

private:
    DensityDetectorParameters parameters;
    std::vector<double> eventTimesMs;
    double densityEnvelope = 0.0;
    double lastFrameTimeMs = 0.0;
    bool hasLastFrameTime = false;
};
}
