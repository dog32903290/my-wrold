#pragma once

#include "AnalyzerResidueDetector.h"
#include "RuntimeRegistry.h"

#include <string>
#include <vector>

namespace myworld
{
struct ResidueDetectorFixtureFrame
{
    bool hasRms = false;
    double rms = 0.0;
    bool hasSustainEnvelope = false;
    double sustainEnvelope = 0.0;
    bool hasSilenceState = false;
    bool silenceState = false;
};

struct ResidueDetectorFixtureExpectation
{
    bool detectorOk = true;
    bool hasResidueStateLast = false;
    bool residueStateLast = false;
    bool hasResidueEnvelopeLast = false;
    double residueEnvelopeLast = 0.0;
    bool hasResidueTimerMsLast = false;
    double residueTimerMsLast = 0.0;
    std::string diagnostic;
};

struct ResidueDetectorFixtureCase
{
    std::string id;
    std::vector<ResidueDetectorFixtureFrame> frames;
    ResidueDetectorFixtureExpectation expect;
};

struct ResidueDetectorFixture
{
    std::string id;
    double frameIntervalMs = 100.0;
    ResidueDetectorParameters parameters;
    std::vector<ResidueDetectorFixtureCase> cases;
};

struct ResidueDetectorFixtureLoadResult
{
    bool ok = false;
    ResidueDetectorFixture fixture;
    std::string error;
};

struct ResidueDetectorCaseResult
{
    std::string id;
    bool passed = false;
    bool detectorOk = true;
    bool residueStateLast = false;
    double residueEnvelopeLast = 0.0;
    double residueTimerMsLast = 0.0;
    double confidenceLast = 0.0;
    std::string diagnostic;
    std::string message;
};

struct ResidueDetectorProofResult
{
    bool ok = false;
    std::string operation = "pv_residue_detector";
    std::string selectedDetector = "residue";
    std::string fixturePath;
    size_t caseCount = 0;
    size_t passedCaseCount = 0;
    std::vector<ResidueDetectorCaseResult> cases;
    bool usesRawRms = true;
    bool usesSustainEnvelopeForDetector = true;
    bool usesSilenceStateForClear = true;
    bool usesOutputSmoothingForDetector = false;
    std::string error;
};

ResidueDetectorFixtureLoadResult loadResidueDetectorFixture (const std::string& fixturePath);
ResidueDetectorProofResult runResidueDetectorProof (const RuntimeRegistry& registry, const std::string& fixturePath);
std::string makeResidueDetectorProofReportJson (const ResidueDetectorProofResult& result);
std::string makeResidueDetectorCookOrderJson();
std::string makeResidueDetectorNodeStatsJson (const ResidueDetectorProofResult& result);
std::string makeResidueDetectorErrorsJson (const ResidueDetectorProofResult& result);
}
