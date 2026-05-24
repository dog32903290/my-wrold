#pragma once

#include "AnalyzerAggregatePressure.h"
#include "RuntimeRegistry.h"

#include <string>
#include <vector>

namespace myworld
{
struct AggregatePressureFixtureFrame
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

struct AggregatePressureExpectation
{
    bool aggregateOk = true;
    bool hasPressureValue = false;
    double pressureValue = 0.0;
    bool hasEnergyComponent = false;
    double energyComponent = 0.0;
    bool hasAttackComponent = false;
    double attackComponent = 0.0;
    bool hasDensityComponent = false;
    double densityComponent = 0.0;
    bool hasSustainComponent = false;
    double sustainComponent = 0.0;
    bool hasResidueComponent = false;
    double residueComponent = 0.0;
    std::string diagnostic;
};

struct AggregatePressureFixtureCase
{
    std::string id;
    AggregatePressureFixtureFrame frame;
    AggregatePressureExpectation expect;
};

struct AggregatePressureFixture
{
    std::string id;
    AggregatePressureParameters parameters;
    std::vector<AggregatePressureFixtureCase> cases;
};

struct AggregatePressureFixtureLoadResult
{
    bool ok = false;
    AggregatePressureFixture fixture;
    std::string error;
};

struct AggregatePressureCaseResult
{
    std::string id;
    bool passed = false;
    bool aggregateOk = true;
    double pressureValue = 0.0;
    double energyComponent = 0.0;
    double attackComponent = 0.0;
    double densityComponent = 0.0;
    double sustainComponent = 0.0;
    double residueComponent = 0.0;
    double confidence = 0.0;
    std::string diagnostic;
    std::string message;
};

struct AggregatePressureProofResult
{
    bool ok = false;
    std::string operation = "pv_aggregate_pressure";
    std::string selectedAggregate = "aggregate_pressure";
    std::string fixturePath;
    size_t caseCount = 0;
    size_t passedCaseCount = 0;
    std::vector<AggregatePressureCaseResult> cases;
    bool usesRawRms = true;
    bool usesDetectorStates = true;
    bool usesOutputSmoothingForAggregate = false;
    std::string error;
};

AggregatePressureFixtureLoadResult loadAggregatePressureFixture (const std::string& fixturePath);
AggregatePressureProofResult runAggregatePressureProof (const RuntimeRegistry& registry, const std::string& fixturePath);
std::string makeAggregatePressureProofReportJson (const AggregatePressureProofResult& result);
std::string makeAggregatePressureCookOrderJson();
std::string makeAggregatePressureNodeStatsJson (const AggregatePressureProofResult& result);
std::string makeAggregatePressureErrorsJson (const AggregatePressureProofResult& result);
}
