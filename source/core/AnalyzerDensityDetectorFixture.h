#pragma once

#include "AnalyzerDensityDetector.h"
#include "RuntimeRegistry.h"

#include <string>
#include <vector>

namespace myworld
{
struct DensityDetectorFixtureFrame
{
    bool hasOnsetEvent = false;
    bool onsetEvent = false;
    bool hasAttackValue = false;
    double attackValue = 0.0;
    bool hasAttackEnvelope = false;
    double attackEnvelope = 0.0;
};

struct DensityDetectorFixtureExpectation
{
    bool detectorOk = true;
    bool hasEventCountLast = false;
    int eventCountLast = 0;
    bool hasDensityValueLast = false;
    double densityValueLast = 0.0;
    std::string diagnostic;
};

struct DensityDetectorFixtureCase
{
    std::string id;
    std::vector<DensityDetectorFixtureFrame> frames;
    DensityDetectorFixtureExpectation expect;
};

struct DensityDetectorFixture
{
    std::string id;
    double frameIntervalMs = 100.0;
    DensityDetectorParameters parameters;
    std::vector<DensityDetectorFixtureCase> cases;
};

struct DensityDetectorFixtureLoadResult
{
    bool ok = false;
    DensityDetectorFixture fixture;
    std::string error;
};

struct DensityDetectorCaseResult
{
    std::string id;
    bool passed = false;
    bool detectorOk = true;
    int eventCountLast = 0;
    double densityValueLast = 0.0;
    double densityEnvelopeLast = 0.0;
    double confidenceLast = 0.0;
    std::string diagnostic;
    std::string message;
};

struct DensityDetectorProofResult
{
    bool ok = false;
    std::string operation = "pv_density_detector";
    std::string selectedDetector = "density";
    std::string fixturePath;
    size_t caseCount = 0;
    size_t passedCaseCount = 0;
    std::vector<DensityDetectorCaseResult> cases;
    bool usesOnsetEvents = true;
    bool usesAttackEnvelopeForDetector = false;
    bool silenceImplemented = false;
    std::string error;
};

DensityDetectorFixtureLoadResult loadDensityDetectorFixture (const std::string& fixturePath);
DensityDetectorProofResult runDensityDetectorProof (const RuntimeRegistry& registry, const std::string& fixturePath);
std::string makeDensityDetectorProofReportJson (const DensityDetectorProofResult& result);
std::string makeDensityDetectorCookOrderJson();
std::string makeDensityDetectorNodeStatsJson (const DensityDetectorProofResult& result);
std::string makeDensityDetectorErrorsJson (const DensityDetectorProofResult& result);
}
