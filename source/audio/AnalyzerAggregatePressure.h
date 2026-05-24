#pragma once

#include <string>

namespace myworld
{
struct AggregatePressureParameters
{
    double energyScale = 4.0;
    double energyWeight = 0.25;
    double attackWeight = 0.25;
    double densityWeight = 0.20;
    double sustainWeight = 0.20;
    double residueWeight = 0.10;
    bool silenceClears = true;
    double outputSmoothMs = 0.0;
};

struct AggregatePressureFrameInput
{
    bool hasRms = false;
    double rms = 0.0;
    bool hasAttackValue = false;
    double attackValue = 0.0;
    bool hasDensityValue = false;
    double densityValue = 0.0;
    bool hasSustainEnvelope = false;
    double sustainEnvelope = 0.0;
    bool hasResidueEnvelope = false;
    double residueEnvelope = 0.0;
    bool hasSilenceState = false;
    bool silenceState = false;
};

struct AggregatePressureFrameOutput
{
    bool aggregateOk = true;
    double pressureValue = 0.0;
    double energyComponent = 0.0;
    double attackComponent = 0.0;
    double densityComponent = 0.0;
    double sustainComponent = 0.0;
    double residueComponent = 0.0;
    double confidence = 0.0;
    std::string diagnostic;
};

class AggregatePressure
{
public:
    explicit AggregatePressure (AggregatePressureParameters parameters);

    AggregatePressureFrameOutput processFrame (const AggregatePressureFrameInput& input) const;

private:
    AggregatePressureParameters parameters;
};
}
