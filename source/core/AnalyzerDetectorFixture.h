#pragma once

#include "AnalyzerAttackDetector.h"
#include "RuntimeRegistry.h"

#include <string>
#include <vector>

namespace myworld
{
struct AttackDetectorFixtureFrame
{
    bool hasRms = false;
    double rms = 0.0;
    bool hasPeak = false;
    double peak = 0.0;
    bool hasSampleCount = false;
    double sampleCount = 0.0;
};

struct AttackDetectorFixtureExpectation
{
    bool detectorOk = true;
    bool hasOnsetEventCount = false;
    int onsetEventCount = 0;
    bool hasFirstOnsetFrame = false;
    int firstOnsetFrame = -1;
    bool hasConfidenceLast = false;
    double confidenceLast = 0.0;
    bool hasAttackValueMin = false;
    double attackValueMin = 0.0;
    std::string diagnostic;
    std::string reason;
};

struct AttackDetectorFixtureCase
{
    std::string id;
    std::vector<AttackDetectorFixtureFrame> frames;
    AttackDetectorFixtureExpectation expect;
};

struct AttackDetectorFixture
{
    std::string id;
    double frameIntervalMs = 40.0;
    AttackDetectorParameters parameters;
    std::vector<AttackDetectorFixtureCase> cases;
};

struct AttackDetectorFixtureLoadResult
{
    bool ok = false;
    AttackDetectorFixture fixture;
    std::string error;
};

struct AttackDetectorCaseResult
{
    std::string id;
    bool passed = false;
    bool detectorOk = true;
    int onsetEventCount = 0;
    int firstOnsetFrame = -1;
    double attackValueMax = 0.0;
    double confidenceLast = 0.0;
    std::string diagnostic;
    std::string message;
    std::vector<std::string> warnings;
};

struct AttackDetectorProofResult
{
    bool ok = false;
    std::string operation = "pv_attack_detector";
    std::string selectedDetector = "attack";
    std::string fixturePath;
    size_t caseCount = 0;
    size_t passedCaseCount = 0;
    std::vector<AttackDetectorCaseResult> cases;
    bool usesRawEnergyFacts = true;
    bool usesOutputSmoothingForDetector = false;
    bool densityImplemented = false;
    bool silenceImplemented = false;
    std::string error;
};

AttackDetectorFixtureLoadResult loadAttackDetectorFixture (const std::string& fixturePath);
AttackDetectorProofResult runAttackDetectorProof (const RuntimeRegistry& registry, const std::string& fixturePath);
std::string makeAttackDetectorProofReportJson (const AttackDetectorProofResult& result);
std::string makeAttackDetectorCookOrderJson();
std::string makeAttackDetectorNodeStatsJson (const AttackDetectorProofResult& result);
std::string makeAttackDetectorErrorsJson (const AttackDetectorProofResult& result);
}
