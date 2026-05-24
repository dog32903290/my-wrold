#pragma once

#include "AnalyzerSilenceDetector.h"
#include "RuntimeRegistry.h"

#include <string>
#include <vector>

namespace myworld
{
struct SilenceDetectorFixtureFrame
{
    bool hasRms = false;
    double rms = 0.0;
    bool hasPeak = false;
    double peak = 0.0;
};

struct SilenceDetectorFixtureExpectation
{
    bool detectorOk = true;
    bool hasSilenceStateLast = false;
    bool silenceStateLast = false;
    bool hasSilenceTimerMsLast = false;
    double silenceTimerMsLast = 0.0;
    std::string diagnostic;
};

struct SilenceDetectorFixtureCase
{
    std::string id;
    std::vector<SilenceDetectorFixtureFrame> frames;
    SilenceDetectorFixtureExpectation expect;
};

struct SilenceDetectorFixture
{
    std::string id;
    double frameIntervalMs = 100.0;
    SilenceDetectorParameters parameters;
    std::vector<SilenceDetectorFixtureCase> cases;
};

struct SilenceDetectorFixtureLoadResult
{
    bool ok = false;
    SilenceDetectorFixture fixture;
    std::string error;
};

struct SilenceDetectorCaseResult
{
    std::string id;
    bool passed = false;
    bool detectorOk = true;
    bool silenceStateLast = false;
    double silenceTimerMsLast = 0.0;
    double confidenceLast = 0.0;
    std::string diagnostic;
    std::string message;
};

struct SilenceDetectorProofResult
{
    bool ok = false;
    std::string operation = "pv_silence_detector";
    std::string selectedDetector = "silence";
    std::string fixturePath;
    size_t caseCount = 0;
    size_t passedCaseCount = 0;
    std::vector<SilenceDetectorCaseResult> cases;
    bool usesRawRms = true;
    bool attackImplemented = false;
    bool densityImplemented = false;
    std::string error;
};

SilenceDetectorFixtureLoadResult loadSilenceDetectorFixture (const std::string& fixturePath);
SilenceDetectorProofResult runSilenceDetectorProof (const RuntimeRegistry& registry, const std::string& fixturePath);
std::string makeSilenceDetectorProofReportJson (const SilenceDetectorProofResult& result);
std::string makeSilenceDetectorCookOrderJson();
std::string makeSilenceDetectorNodeStatsJson (const SilenceDetectorProofResult& result);
std::string makeSilenceDetectorErrorsJson (const SilenceDetectorProofResult& result);
}
