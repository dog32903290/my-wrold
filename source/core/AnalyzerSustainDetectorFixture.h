#pragma once

#include "AnalyzerSustainDetector.h"
#include "RuntimeRegistry.h"

#include <string>
#include <vector>

namespace myworld
{
struct SustainDetectorFixtureFrame
{
    bool hasRms = false;
    double rms = 0.0;
    bool hasPeak = false;
    double peak = 0.0;
};

struct SustainDetectorFixtureExpectation
{
    bool detectorOk = true;
    bool hasSustainStateLast = false;
    bool sustainStateLast = false;
    bool hasSustainTimerMsLast = false;
    double sustainTimerMsLast = 0.0;
    bool hasSustainEnvelopeLast = false;
    double sustainEnvelopeLast = 0.0;
    std::string diagnostic;
};

struct SustainDetectorFixtureCase
{
    std::string id;
    std::vector<SustainDetectorFixtureFrame> frames;
    SustainDetectorFixtureExpectation expect;
};

struct SustainDetectorFixture
{
    std::string id;
    double frameIntervalMs = 100.0;
    SustainDetectorParameters parameters;
    std::vector<SustainDetectorFixtureCase> cases;
};

struct SustainDetectorFixtureLoadResult
{
    bool ok = false;
    SustainDetectorFixture fixture;
    std::string error;
};

struct SustainDetectorCaseResult
{
    std::string id;
    bool passed = false;
    bool detectorOk = true;
    bool sustainStateLast = false;
    double sustainTimerMsLast = 0.0;
    double sustainEnvelopeLast = 0.0;
    double confidenceLast = 0.0;
    std::string diagnostic;
    std::string message;
};

struct SustainDetectorProofResult
{
    bool ok = false;
    std::string operation = "pv_sustain_detector";
    std::string selectedDetector = "sustain";
    std::string fixturePath;
    size_t caseCount = 0;
    size_t passedCaseCount = 0;
    std::vector<SustainDetectorCaseResult> cases;
    bool usesRawRms = true;
    bool usesAttackOnsetForDetector = false;
    bool usesSilenceStateForDetector = false;
    std::string error;
};

SustainDetectorFixtureLoadResult loadSustainDetectorFixture (const std::string& fixturePath);
SustainDetectorProofResult runSustainDetectorProof (const RuntimeRegistry& registry, const std::string& fixturePath);
std::string makeSustainDetectorProofReportJson (const SustainDetectorProofResult& result);
std::string makeSustainDetectorCookOrderJson();
std::string makeSustainDetectorNodeStatsJson (const SustainDetectorProofResult& result);
std::string makeSustainDetectorErrorsJson (const SustainDetectorProofResult& result);
}
