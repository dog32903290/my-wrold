#include "AnalyzerDensityDetectorFixture.h"

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

constexpr const char* densityNodeType = "compound.density";

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

DensityDetectorFixtureFrame parseFrame (const JsonValue& value)
{
    DensityDetectorFixtureFrame frame;
    frame.hasOnsetEvent = hasNumberMember (value, "onset_event");
    frame.onsetEvent = numberMember (value, "onset_event", 0.0) != 0.0;
    frame.hasAttackValue = hasNumberMember (value, "attack_value");
    frame.attackValue = numberMember (value, "attack_value", 0.0);
    frame.hasAttackEnvelope = hasNumberMember (value, "attack_envelope");
    frame.attackEnvelope = numberMember (value, "attack_envelope", 0.0);
    return frame;
}

DensityDetectorFixtureExpectation parseExpectation (const JsonValue& value)
{
    DensityDetectorFixtureExpectation expectation;
    expectation.detectorOk = boolMember (value, "detectorOk", true);
    expectation.hasEventCountLast = hasNumberMember (value, "eventCountLast");
    expectation.eventCountLast = static_cast<int> (numberMember (value, "eventCountLast", 0.0));
    expectation.hasDensityValueLast = hasNumberMember (value, "densityValueLast");
    expectation.densityValueLast = numberMember (value, "densityValueLast", 0.0);
    expectation.diagnostic = stringMember (value, "diagnostic");
    return expectation;
}

bool registryContainsDensity (const RuntimeRegistry& registry)
{
    return std::any_of (registry.entries.begin(), registry.entries.end(), [] (const auto& entry) {
        return entry.nodeType == densityNodeType;
    });
}

bool densityRuntimeCoverageReady (const RuntimeRegistry& registry)
{
    const auto coverage = inspectRuntimeOpCoverage (registry);
    if (! coverage.ok)
        return false;

    const auto diagnostics = makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    return std::any_of (diagnostics.begin(), diagnostics.end(), [] (const auto& diagnostic) {
        return diagnostic.nodeType == densityNodeType
               && diagnostic.status == "runtime-op-ready"
               && runtimeOpDiagnosticAllowsCreation (diagnostic);
    });
}

DensityDetectorFrameInput makeFrameInput (const DensityDetectorFixtureFrame& frame,
                                          const size_t frameIndex,
                                          const double frameIntervalMs)
{
    return {
        frame.hasOnsetEvent,
        frame.onsetEvent,
        frame.hasAttackValue,
        frame.attackValue,
        frame.hasAttackEnvelope,
        frame.attackEnvelope,
        static_cast<double> (frameIndex) * frameIntervalMs
    };
}

DensityDetectorCaseResult runCase (const DensityDetectorFixtureCase& testCase,
                                   const DensityDetectorParameters& parameters,
                                   const double frameIntervalMs)
{
    DensityDetector detector { parameters };
    DensityDetectorCaseResult result;
    result.id = testCase.id;

    for (size_t frameIndex = 0; frameIndex < testCase.frames.size(); ++frameIndex)
    {
        const auto output = detector.processFrame (makeFrameInput (testCase.frames[frameIndex],
                                                                   frameIndex,
                                                                   frameIntervalMs));
        result.detectorOk = result.detectorOk && output.detectorOk;
        result.eventCountLast = output.eventCount;
        result.densityValueLast = output.densityValue;
        result.densityEnvelopeLast = output.densityEnvelope;
        result.confidenceLast = output.confidence;

        if (! output.diagnostic.empty() && result.diagnostic.empty())
            result.diagnostic = output.diagnostic;
    }

    const auto& expectation = testCase.expect;
    result.passed = result.detectorOk == expectation.detectorOk;
    if (! result.passed)
        result.message = "detectorOk mismatch";

    if (result.passed && expectation.hasEventCountLast)
    {
        result.passed = result.eventCountLast == expectation.eventCountLast;
        if (! result.passed)
            result.message = "eventCountLast mismatch";
    }

    if (result.passed && expectation.hasDensityValueLast)
    {
        result.passed = std::abs (result.densityValueLast - expectation.densityValueLast) < 0.000001;
        if (! result.passed)
            result.message = "densityValueLast mismatch";
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

void appendCaseResultJson (std::ostream& out, const DensityDetectorCaseResult& result)
{
    out << "    {\n";
    out << "      \"id\": " << jsonQuoted (result.id) << ",\n";
    out << "      \"passed\": " << (result.passed ? "true" : "false") << ",\n";
    out << "      \"detectorOk\": " << (result.detectorOk ? "true" : "false") << ",\n";
    out << "      \"eventCountLast\": " << result.eventCountLast << ",\n";
    out << "      \"densityValueLast\": " << result.densityValueLast << ",\n";
    out << "      \"densityEnvelopeLast\": " << result.densityEnvelopeLast << ",\n";
    out << "      \"confidenceLast\": " << result.confidenceLast << ",\n";
    out << "      \"diagnostic\": " << jsonQuoted (result.diagnostic) << ",\n";
    out << "      \"message\": " << jsonQuoted (result.message) << "\n";
    out << "    }";
}
}

DensityDetectorFixtureLoadResult loadDensityDetectorFixture (const std::string& fixturePath)
{
    const auto text = readTextFile (fixturePath);
    if (text.empty())
        return { false, {}, "could not read density detector fixture: " + fixturePath };

    JsonParser parser { text };
    const auto root = parser.parse();
    if (! parser.ok())
        return { false, {}, parser.error() };

    DensityDetectorFixture fixture;
    fixture.id = stringMember (root, "id");
    fixture.frameIntervalMs = numberMember (root, "frameIntervalMs", 100.0);

    if (const auto* parameters = member (root, "parameters"))
    {
        fixture.parameters.windowMs = numberMember (*parameters, "windowMs", fixture.parameters.windowMs);
        fixture.parameters.maxEvents = numberMember (*parameters, "maxEvents", fixture.parameters.maxEvents);
        fixture.parameters.releaseMs = numberMember (*parameters, "releaseMs", fixture.parameters.releaseMs);
        fixture.parameters.outputSmoothMs = numberMember (*parameters,
                                                          "outputSmoothMs",
                                                          fixture.parameters.outputSmoothMs);
    }

    const auto* cases = member (root, "cases");
    if (cases == nullptr || cases->kind != JsonValue::Kind::array)
        return { false, {}, "density detector fixture missing cases array" };

    for (const auto& caseValue : cases->arrayValue)
    {
        DensityDetectorFixtureCase testCase;
        testCase.id = stringMember (caseValue, "id");

        if (const auto* frames = member (caseValue, "frames"))
        {
            for (const auto& frame : frames->arrayValue)
                testCase.frames.push_back (parseFrame (frame));
        }

        if (const auto* expectation = member (caseValue, "expect"))
            testCase.expect = parseExpectation (*expectation);

        fixture.cases.push_back (testCase);
    }

    return { true, fixture, {} };
}

DensityDetectorProofResult runDensityDetectorProof (const RuntimeRegistry& registry, const std::string& fixturePath)
{
    DensityDetectorProofResult result;
    result.fixturePath = fixturePath;

    if (! registryContainsDensity (registry))
    {
        result.error = "runtime registry missing compound.density";
        return result;
    }

    if (! densityRuntimeCoverageReady (registry))
    {
        result.error = "compound.density runtime coverage is not ready";
        return result;
    }

    const auto fixture = loadDensityDetectorFixture (fixturePath);
    if (! fixture.ok)
    {
        result.error = fixture.error;
        return result;
    }

    result.caseCount = fixture.fixture.cases.size();
    for (const auto& testCase : fixture.fixture.cases)
    {
        auto caseResult = runCase (testCase,
                                   fixture.fixture.parameters,
                                   fixture.fixture.frameIntervalMs);
        if (caseResult.passed)
            ++result.passedCaseCount;

        result.cases.push_back (std::move (caseResult));
    }

    result.ok = result.caseCount > 0 && result.passedCaseCount == result.caseCount;
    result.error = result.ok ? std::string {} : "density detector fixture cases did not all pass";
    return result;
}

std::string makeDensityDetectorProofReportJson (const DensityDetectorProofResult& result)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"pvDensityDetectorProof\",\n";
    out << "  \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    out << "  \"operation\": " << jsonQuoted (result.operation) << ",\n";
    out << "  \"selectedDetector\": " << jsonQuoted (result.selectedDetector) << ",\n";
    out << "  \"fixturePath\": " << jsonQuoted (result.fixturePath) << ",\n";
    out << "  \"caseCount\": " << result.caseCount << ",\n";
    out << "  \"passedCaseCount\": " << result.passedCaseCount << ",\n";
    out << "  \"usesOnsetEvents\": " << (result.usesOnsetEvents ? "true" : "false") << ",\n";
    out << "  \"usesAttackEnvelopeForDetector\": "
        << (result.usesAttackEnvelopeForDetector ? "true" : "false") << ",\n";
    out << "  \"silenceImplemented\": " << (result.silenceImplemented ? "true" : "false") << ",\n";
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

std::string makeDensityDetectorCookOrderJson()
{
    return "{\n"
           "  \"version\": 1,\n"
           "  \"mode\": \"pv-density-detector\",\n"
           "  \"cookOrder\": [\"attack.onset_event\", \"analyzer.density\", \"density_out\"]\n"
           "}\n";
}

std::string makeDensityDetectorNodeStatsJson (const DensityDetectorProofResult& result)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"mode\": \"pv-density-detector\",\n";
    out << "  \"renderer\": \"headless-analyzer\",\n";
    out << "  \"nodes\": [\n";
    out << "    { \"id\": \"attack.onset_event\", \"type\": \"event.trigger\", \"status\": \"fixture-source\" },\n";
    out << "    { \"id\": \"density\", \"type\": \"analyzer.density\", \"status\": "
        << jsonQuoted (result.ok ? "computed" : "failed") << " },\n";
    out << "    { \"id\": \"density_out\", \"type\": \"analyzer.density_out\", \"status\": "
        << jsonQuoted (result.ok ? "computed" : "failed") << " }\n";
    out << "  ],\n";
    out << "  \"caseCount\": " << result.caseCount << ",\n";
    out << "  \"passedCaseCount\": " << result.passedCaseCount << "\n";
    out << "}\n";
    return out.str();
}

std::string makeDensityDetectorErrorsJson (const DensityDetectorProofResult& result)
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
