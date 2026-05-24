#include "AnalyzerAggregatePressureFixture.h"

#include "JsonWriter.h"
#include "StorageContractJson.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

namespace myworld
{
namespace
{
using storage_contract_internal::JsonParser;
using storage_contract_internal::JsonValue;
using storage_contract_internal::boolMember;
using storage_contract_internal::member;
using storage_contract_internal::numberMember;
using storage_contract_internal::stringMember;

constexpr const char* aggregateNodeType = "compound.aggregate-pressure";

std::string readTextFile (const std::string& path)
{
    std::ifstream input (path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

bool hasNumberMember (const JsonValue& value, const std::string& name)
{
    const auto* found = member (value, name);
    return found != nullptr && found->kind == JsonValue::Kind::number;
}

bool hasBoolMember (const JsonValue& value, const std::string& name)
{
    const auto* found = member (value, name);
    return found != nullptr && found->kind == JsonValue::Kind::boolean;
}

AggregatePressureFixtureFrame parseFrame (const JsonValue& value)
{
    AggregatePressureFixtureFrame frame;
    frame.hasRms = hasNumberMember (value, "rms");
    frame.rms = numberMember (value, "rms", 0.0);
    frame.hasAttackValue = hasNumberMember (value, "attack_value");
    frame.attackValue = numberMember (value, "attack_value", 0.0);
    frame.hasDensityValue = hasNumberMember (value, "density_value");
    frame.densityValue = numberMember (value, "density_value", 0.0);
    frame.hasSustainEnvelope = hasNumberMember (value, "sustain_envelope");
    frame.sustainEnvelope = numberMember (value, "sustain_envelope", 0.0);
    frame.hasResidueEnvelope = hasNumberMember (value, "residue_envelope");
    frame.residueEnvelope = numberMember (value, "residue_envelope", 0.0);
    frame.hasSilenceState = hasBoolMember (value, "silence_state");
    frame.silenceState = boolMember (value, "silence_state", false);
    return frame;
}

AggregatePressureExpectation parseExpectation (const JsonValue& value)
{
    AggregatePressureExpectation expectation;
    expectation.aggregateOk = boolMember (value, "aggregateOk", true);
    expectation.hasPressureValue = hasNumberMember (value, "pressureValue");
    expectation.pressureValue = numberMember (value, "pressureValue", 0.0);
    expectation.hasEnergyComponent = hasNumberMember (value, "energyComponent");
    expectation.energyComponent = numberMember (value, "energyComponent", 0.0);
    expectation.hasAttackComponent = hasNumberMember (value, "attackComponent");
    expectation.attackComponent = numberMember (value, "attackComponent", 0.0);
    expectation.hasDensityComponent = hasNumberMember (value, "densityComponent");
    expectation.densityComponent = numberMember (value, "densityComponent", 0.0);
    expectation.hasSustainComponent = hasNumberMember (value, "sustainComponent");
    expectation.sustainComponent = numberMember (value, "sustainComponent", 0.0);
    expectation.hasResidueComponent = hasNumberMember (value, "residueComponent");
    expectation.residueComponent = numberMember (value, "residueComponent", 0.0);
    expectation.diagnostic = stringMember (value, "diagnostic");
    return expectation;
}

bool registryContainsAggregatePressure (const RuntimeRegistry& registry)
{
    return std::any_of (registry.entries.begin(), registry.entries.end(), [] (const auto& entry) {
        return entry.nodeType == aggregateNodeType;
    });
}

bool aggregatePressureRuntimeCoverageReady (const RuntimeRegistry& registry)
{
    const auto coverage = inspectRuntimeOpCoverage (registry);
    if (! coverage.ok)
        return false;

    const auto diagnostics = makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    return std::any_of (diagnostics.begin(), diagnostics.end(), [] (const auto& diagnostic) {
        return diagnostic.nodeType == aggregateNodeType
               && diagnostic.status == "runtime-op-ready"
               && runtimeOpDiagnosticAllowsCreation (diagnostic);
    });
}

AggregatePressureFrameInput makeFrameInput (const AggregatePressureFixtureFrame& frame)
{
    return {
        frame.hasRms,
        frame.rms,
        frame.hasAttackValue,
        frame.attackValue,
        frame.hasDensityValue,
        frame.densityValue,
        frame.hasSustainEnvelope,
        frame.sustainEnvelope,
        frame.hasResidueEnvelope,
        frame.residueEnvelope,
        frame.hasSilenceState,
        frame.silenceState
    };
}

bool nearEqual (const double actual, const double expected)
{
    return std::abs (actual - expected) < 0.000001;
}

AggregatePressureCaseResult runCase (const AggregatePressureFixtureCase& testCase,
                                     const AggregatePressureParameters& parameters)
{
    const AggregatePressure aggregate { parameters };
    const auto output = aggregate.processFrame (makeFrameInput (testCase.frame));

    AggregatePressureCaseResult result;
    result.id = testCase.id;
    result.aggregateOk = output.aggregateOk;
    result.pressureValue = output.pressureValue;
    result.energyComponent = output.energyComponent;
    result.attackComponent = output.attackComponent;
    result.densityComponent = output.densityComponent;
    result.sustainComponent = output.sustainComponent;
    result.residueComponent = output.residueComponent;
    result.confidence = output.confidence;
    result.diagnostic = output.diagnostic;

    const auto& expectation = testCase.expect;
    result.passed = result.aggregateOk == expectation.aggregateOk;
    if (! result.passed)
        result.message = "aggregateOk mismatch";

    if (result.passed && expectation.hasPressureValue)
    {
        result.passed = nearEqual (result.pressureValue, expectation.pressureValue);
        if (! result.passed)
            result.message = "pressureValue mismatch";
    }

    if (result.passed && expectation.hasEnergyComponent)
    {
        result.passed = nearEqual (result.energyComponent, expectation.energyComponent);
        if (! result.passed)
            result.message = "energyComponent mismatch";
    }

    if (result.passed && expectation.hasAttackComponent)
    {
        result.passed = nearEqual (result.attackComponent, expectation.attackComponent);
        if (! result.passed)
            result.message = "attackComponent mismatch";
    }

    if (result.passed && expectation.hasDensityComponent)
    {
        result.passed = nearEqual (result.densityComponent, expectation.densityComponent);
        if (! result.passed)
            result.message = "densityComponent mismatch";
    }

    if (result.passed && expectation.hasSustainComponent)
    {
        result.passed = nearEqual (result.sustainComponent, expectation.sustainComponent);
        if (! result.passed)
            result.message = "sustainComponent mismatch";
    }

    if (result.passed && expectation.hasResidueComponent)
    {
        result.passed = nearEqual (result.residueComponent, expectation.residueComponent);
        if (! result.passed)
            result.message = "residueComponent mismatch";
    }

    if (result.passed && ! expectation.diagnostic.empty())
    {
        result.passed = result.diagnostic == expectation.diagnostic;
        if (! result.passed)
            result.message = "diagnostic mismatch";
    }

    if (result.passed)
        result.message = "matched";

    return result;
}

void appendCaseResultJson (std::ostream& out, const AggregatePressureCaseResult& result)
{
    out << "    {\n";
    out << "      \"id\": " << jsonQuoted (result.id) << ",\n";
    out << "      \"passed\": " << (result.passed ? "true" : "false") << ",\n";
    out << "      \"aggregateOk\": " << (result.aggregateOk ? "true" : "false") << ",\n";
    out << "      \"pressureValue\": " << result.pressureValue << ",\n";
    out << "      \"energyComponent\": " << result.energyComponent << ",\n";
    out << "      \"attackComponent\": " << result.attackComponent << ",\n";
    out << "      \"densityComponent\": " << result.densityComponent << ",\n";
    out << "      \"sustainComponent\": " << result.sustainComponent << ",\n";
    out << "      \"residueComponent\": " << result.residueComponent << ",\n";
    out << "      \"confidence\": " << result.confidence << ",\n";
    out << "      \"diagnostic\": " << jsonQuoted (result.diagnostic) << ",\n";
    out << "      \"message\": " << jsonQuoted (result.message) << "\n";
    out << "    }";
}
}

AggregatePressureFixtureLoadResult loadAggregatePressureFixture (const std::string& fixturePath)
{
    const auto text = readTextFile (fixturePath);
    if (text.empty())
        return { false, {}, "could not read aggregate pressure fixture: " + fixturePath };

    JsonParser parser { text };
    const auto root = parser.parse();
    if (! parser.ok())
        return { false, {}, parser.error() };

    AggregatePressureFixture fixture;
    fixture.id = stringMember (root, "id");

    if (const auto* parameters = member (root, "parameters"))
    {
        fixture.parameters.energyScale = numberMember (*parameters, "energyScale", fixture.parameters.energyScale);
        fixture.parameters.energyWeight = numberMember (*parameters, "energyWeight", fixture.parameters.energyWeight);
        fixture.parameters.attackWeight = numberMember (*parameters, "attackWeight", fixture.parameters.attackWeight);
        fixture.parameters.densityWeight = numberMember (*parameters,
                                                         "densityWeight",
                                                         fixture.parameters.densityWeight);
        fixture.parameters.sustainWeight = numberMember (*parameters,
                                                         "sustainWeight",
                                                         fixture.parameters.sustainWeight);
        fixture.parameters.residueWeight = numberMember (*parameters,
                                                         "residueWeight",
                                                         fixture.parameters.residueWeight);
        fixture.parameters.silenceClears = boolMember (*parameters,
                                                       "silenceClears",
                                                       fixture.parameters.silenceClears);
        fixture.parameters.outputSmoothMs = numberMember (*parameters,
                                                          "outputSmoothMs",
                                                          fixture.parameters.outputSmoothMs);
    }

    const auto* cases = member (root, "cases");
    if (cases == nullptr || cases->kind != JsonValue::Kind::array)
        return { false, {}, "aggregate pressure fixture missing cases array" };

    for (const auto& caseValue : cases->arrayValue)
    {
        AggregatePressureFixtureCase testCase;
        testCase.id = stringMember (caseValue, "id");

        if (const auto* frame = member (caseValue, "frame"))
            testCase.frame = parseFrame (*frame);

        if (const auto* expectation = member (caseValue, "expect"))
            testCase.expect = parseExpectation (*expectation);

        fixture.cases.push_back (testCase);
    }

    return { true, fixture, {} };
}

AggregatePressureProofResult runAggregatePressureProof (const RuntimeRegistry& registry,
                                                        const std::string& fixturePath)
{
    AggregatePressureProofResult result;
    result.fixturePath = fixturePath;

    if (! registryContainsAggregatePressure (registry))
    {
        result.error = "runtime registry missing compound.aggregate-pressure";
        return result;
    }

    if (! aggregatePressureRuntimeCoverageReady (registry))
    {
        result.error = "compound.aggregate-pressure runtime coverage is not ready";
        return result;
    }

    const auto fixture = loadAggregatePressureFixture (fixturePath);
    if (! fixture.ok)
    {
        result.error = fixture.error;
        return result;
    }

    result.caseCount = fixture.fixture.cases.size();
    for (const auto& testCase : fixture.fixture.cases)
    {
        auto caseResult = runCase (testCase, fixture.fixture.parameters);
        if (caseResult.passed)
            ++result.passedCaseCount;

        result.cases.push_back (std::move (caseResult));
    }

    result.ok = result.caseCount > 0 && result.passedCaseCount == result.caseCount;
    result.error = result.ok ? std::string {} : "aggregate pressure fixture cases did not all pass";
    return result;
}

std::string makeAggregatePressureProofReportJson (const AggregatePressureProofResult& result)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"pvAggregatePressureProof\",\n";
    out << "  \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    out << "  \"operation\": " << jsonQuoted (result.operation) << ",\n";
    out << "  \"selectedAggregate\": " << jsonQuoted (result.selectedAggregate) << ",\n";
    out << "  \"fixturePath\": " << jsonQuoted (result.fixturePath) << ",\n";
    out << "  \"caseCount\": " << result.caseCount << ",\n";
    out << "  \"passedCaseCount\": " << result.passedCaseCount << ",\n";
    out << "  \"usesRawRms\": " << (result.usesRawRms ? "true" : "false") << ",\n";
    out << "  \"usesDetectorStates\": " << (result.usesDetectorStates ? "true" : "false") << ",\n";
    out << "  \"usesOutputSmoothingForAggregate\": "
        << (result.usesOutputSmoothingForAggregate ? "true" : "false") << ",\n";
    out << "  \"cases\": [\n";

    for (size_t index = 0; index < result.cases.size(); ++index)
    {
        appendCaseResultJson (out, result.cases[index]);
        out << (index + 1 == result.cases.size() ? "\n" : ",\n");
    }

    out << "  ],\n";
    out << "  \"error\": " << jsonQuoted (result.error) << "\n";
    out << "}\n";
    return out.str();
}

std::string makeAggregatePressureCookOrderJson()
{
    return "{\n"
           "  \"version\": 1,\n"
           "  \"mode\": \"pv-aggregate-pressure\",\n"
           "  \"cookOrder\": [\"raw-energy.rms\", \"analyzer.attack\", \"analyzer.density\", "
           "\"analyzer.sustain\", \"analyzer.residue\", \"analyzer.silence\", "
           "\"analyzer.aggregate_pressure\", \"aggregate_pressure_out\"]\n"
           "}\n";
}

std::string makeAggregatePressureNodeStatsJson (const AggregatePressureProofResult& result)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"mode\": \"pv-aggregate-pressure\",\n";
    out << "  \"renderer\": \"headless-analyzer\",\n";
    out << "  \"nodes\": [\n";
    out << "    { \"id\": \"raw-energy.rms\", \"type\": \"signal.float\", \"status\": \"fixture-source\" },\n";
    out << "    { \"id\": \"attack\", \"type\": \"analyzer.attack\", \"status\": \"fixture-source\" },\n";
    out << "    { \"id\": \"density\", \"type\": \"analyzer.density\", \"status\": \"fixture-source\" },\n";
    out << "    { \"id\": \"sustain\", \"type\": \"analyzer.sustain\", \"status\": \"fixture-source\" },\n";
    out << "    { \"id\": \"residue\", \"type\": \"analyzer.residue\", \"status\": \"fixture-source\" },\n";
    out << "    { \"id\": \"silence\", \"type\": \"analyzer.silence\", \"status\": \"fixture-source\" },\n";
    out << "    { \"id\": \"aggregate_pressure\", \"type\": \"analyzer.aggregate_pressure\", \"status\": "
        << jsonQuoted (result.ok ? "computed" : "failed") << " },\n";
    out << "    { \"id\": \"aggregate_pressure_out\", \"type\": \"analyzer.aggregate_pressure_out\", \"status\": "
        << jsonQuoted (result.ok ? "computed" : "failed") << " }\n";
    out << "  ],\n";
    out << "  \"caseCount\": " << result.caseCount << ",\n";
    out << "  \"passedCaseCount\": " << result.passedCaseCount << "\n";
    out << "}\n";
    return out.str();
}

std::string makeAggregatePressureErrorsJson (const AggregatePressureProofResult& result)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    out << "  \"errors\": ";
    if (result.error.empty())
        out << "[]\n";
    else
        appendJsonStringArray (out, { result.error });
    out << "}\n";
    return out.str();
}
}
