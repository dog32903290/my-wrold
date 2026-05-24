#include "AnalyzerSustainDetectorFixture.h"

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

constexpr const char* sustainNodeType = "compound.sustain";

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

SustainDetectorFixtureFrame parseFrame (const JsonValue& value)
{
    SustainDetectorFixtureFrame frame;
    frame.hasRms = hasNumberMember (value, "rms");
    frame.rms = numberMember (value, "rms", 0.0);
    frame.hasPeak = hasNumberMember (value, "peak");
    frame.peak = numberMember (value, "peak", 0.0);
    return frame;
}

SustainDetectorFixtureExpectation parseExpectation (const JsonValue& value)
{
    SustainDetectorFixtureExpectation expectation;
    expectation.detectorOk = boolMember (value, "detectorOk", true);
    expectation.hasSustainStateLast = hasBoolMember (value, "sustainStateLast");
    expectation.sustainStateLast = boolMember (value, "sustainStateLast", false);
    expectation.hasSustainTimerMsLast = hasNumberMember (value, "sustainTimerMsLast");
    expectation.sustainTimerMsLast = numberMember (value, "sustainTimerMsLast", 0.0);
    expectation.hasSustainEnvelopeLast = hasNumberMember (value, "sustainEnvelopeLast");
    expectation.sustainEnvelopeLast = numberMember (value, "sustainEnvelopeLast", 0.0);
    expectation.diagnostic = stringMember (value, "diagnostic");
    return expectation;
}

bool registryContainsSustain (const RuntimeRegistry& registry)
{
    return std::any_of (registry.entries.begin(), registry.entries.end(), [] (const auto& entry) {
        return entry.nodeType == sustainNodeType;
    });
}

bool sustainRuntimeCoverageReady (const RuntimeRegistry& registry)
{
    const auto coverage = inspectRuntimeOpCoverage (registry);
    if (! coverage.ok)
        return false;

    const auto diagnostics = makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    return std::any_of (diagnostics.begin(), diagnostics.end(), [] (const auto& diagnostic) {
        return diagnostic.nodeType == sustainNodeType
               && diagnostic.status == "runtime-op-ready"
               && runtimeOpDiagnosticAllowsCreation (diagnostic);
    });
}

SustainDetectorFrameInput makeFrameInput (const SustainDetectorFixtureFrame& frame,
                                          const size_t frameIndex,
                                          const double frameIntervalMs)
{
    return {
        frame.hasRms,
        frame.rms,
        frame.hasPeak,
        frame.peak,
        static_cast<double> (frameIndex) * frameIntervalMs
    };
}

SustainDetectorCaseResult runCase (const SustainDetectorFixtureCase& testCase,
                                   const SustainDetectorParameters& parameters,
                                   const double frameIntervalMs)
{
    SustainDetector detector { parameters };
    SustainDetectorCaseResult result;
    result.id = testCase.id;

    for (size_t frameIndex = 0; frameIndex < testCase.frames.size(); ++frameIndex)
    {
        const auto output = detector.processFrame (makeFrameInput (testCase.frames[frameIndex],
                                                                   frameIndex,
                                                                   frameIntervalMs));
        result.detectorOk = result.detectorOk && output.detectorOk;
        result.sustainStateLast = output.sustainState;
        result.sustainTimerMsLast = output.sustainTimerMs;
        result.sustainEnvelopeLast = output.sustainEnvelope;
        result.confidenceLast = output.confidence;

        if (! output.diagnostic.empty() && result.diagnostic.empty())
            result.diagnostic = output.diagnostic;
    }

    const auto& expectation = testCase.expect;
    result.passed = result.detectorOk == expectation.detectorOk;
    if (! result.passed)
        result.message = "detectorOk mismatch";

    if (result.passed && expectation.hasSustainStateLast)
    {
        result.passed = result.sustainStateLast == expectation.sustainStateLast;
        if (! result.passed)
            result.message = "sustainStateLast mismatch";
    }

    if (result.passed && expectation.hasSustainTimerMsLast)
    {
        result.passed = std::abs (result.sustainTimerMsLast - expectation.sustainTimerMsLast) < 0.000001;
        if (! result.passed)
            result.message = "sustainTimerMsLast mismatch";
    }

    if (result.passed && expectation.hasSustainEnvelopeLast)
    {
        result.passed = std::abs (result.sustainEnvelopeLast - expectation.sustainEnvelopeLast) < 0.000001;
        if (! result.passed)
            result.message = "sustainEnvelopeLast mismatch";
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

void appendCaseResultJson (std::ostream& out, const SustainDetectorCaseResult& result)
{
    out << "    {\n";
    out << "      \"id\": " << jsonQuoted (result.id) << ",\n";
    out << "      \"passed\": " << (result.passed ? "true" : "false") << ",\n";
    out << "      \"detectorOk\": " << (result.detectorOk ? "true" : "false") << ",\n";
    out << "      \"sustainStateLast\": " << (result.sustainStateLast ? "true" : "false") << ",\n";
    out << "      \"sustainTimerMsLast\": " << result.sustainTimerMsLast << ",\n";
    out << "      \"sustainEnvelopeLast\": " << result.sustainEnvelopeLast << ",\n";
    out << "      \"confidenceLast\": " << result.confidenceLast << ",\n";
    out << "      \"diagnostic\": " << jsonQuoted (result.diagnostic) << ",\n";
    out << "      \"message\": " << jsonQuoted (result.message) << "\n";
    out << "    }";
}
}

SustainDetectorFixtureLoadResult loadSustainDetectorFixture (const std::string& fixturePath)
{
    const auto text = readTextFile (fixturePath);
    if (text.empty())
        return { false, {}, "could not read sustain detector fixture: " + fixturePath };

    JsonParser parser { text };
    const auto root = parser.parse();
    if (! parser.ok())
        return { false, {}, parser.error() };

    SustainDetectorFixture fixture;
    fixture.id = stringMember (root, "id");
    fixture.frameIntervalMs = numberMember (root, "frameIntervalMs", 100.0);

    if (const auto* parameters = member (root, "parameters"))
    {
        fixture.parameters.floor = numberMember (*parameters, "floor", fixture.parameters.floor);
        fixture.parameters.holdMs = numberMember (*parameters, "holdMs", fixture.parameters.holdMs);
        fixture.parameters.releaseMs = numberMember (*parameters, "releaseMs", fixture.parameters.releaseMs);
        fixture.parameters.outputSmoothMs = numberMember (*parameters,
                                                          "outputSmoothMs",
                                                          fixture.parameters.outputSmoothMs);
    }

    const auto* cases = member (root, "cases");
    if (cases == nullptr || cases->kind != JsonValue::Kind::array)
        return { false, {}, "sustain detector fixture missing cases array" };

    for (const auto& caseValue : cases->arrayValue)
    {
        SustainDetectorFixtureCase testCase;
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

SustainDetectorProofResult runSustainDetectorProof (const RuntimeRegistry& registry, const std::string& fixturePath)
{
    SustainDetectorProofResult result;
    result.fixturePath = fixturePath;

    if (! registryContainsSustain (registry))
    {
        result.error = "runtime registry missing compound.sustain";
        return result;
    }

    if (! sustainRuntimeCoverageReady (registry))
    {
        result.error = "compound.sustain runtime coverage is not ready";
        return result;
    }

    const auto fixture = loadSustainDetectorFixture (fixturePath);
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
    result.error = result.ok ? std::string {} : "sustain detector fixture cases did not all pass";
    return result;
}

std::string makeSustainDetectorProofReportJson (const SustainDetectorProofResult& result)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"pvSustainDetectorProof\",\n";
    out << "  \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    out << "  \"operation\": " << jsonQuoted (result.operation) << ",\n";
    out << "  \"selectedDetector\": " << jsonQuoted (result.selectedDetector) << ",\n";
    out << "  \"fixturePath\": " << jsonQuoted (result.fixturePath) << ",\n";
    out << "  \"caseCount\": " << result.caseCount << ",\n";
    out << "  \"passedCaseCount\": " << result.passedCaseCount << ",\n";
    out << "  \"usesRawRms\": " << (result.usesRawRms ? "true" : "false") << ",\n";
    out << "  \"usesAttackOnsetForDetector\": " << (result.usesAttackOnsetForDetector ? "true" : "false") << ",\n";
    out << "  \"usesSilenceStateForDetector\": " << (result.usesSilenceStateForDetector ? "true" : "false") << ",\n";
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

std::string makeSustainDetectorCookOrderJson()
{
    return "{\n"
           "  \"version\": 1,\n"
           "  \"mode\": \"pv-sustain-detector\",\n"
           "  \"cookOrder\": [\"raw-energy.rms\", \"analyzer.sustain\", \"sustain_out\"]\n"
           "}\n";
}

std::string makeSustainDetectorNodeStatsJson (const SustainDetectorProofResult& result)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"mode\": \"pv-sustain-detector\",\n";
    out << "  \"renderer\": \"headless-analyzer\",\n";
    out << "  \"nodes\": [\n";
    out << "    { \"id\": \"raw-energy.rms\", \"type\": \"signal.float\", \"status\": \"fixture-source\" },\n";
    out << "    { \"id\": \"sustain\", \"type\": \"analyzer.sustain\", \"status\": "
        << jsonQuoted (result.ok ? "computed" : "failed") << " },\n";
    out << "    { \"id\": \"sustain_out\", \"type\": \"analyzer.sustain_out\", \"status\": "
        << jsonQuoted (result.ok ? "computed" : "failed") << " }\n";
    out << "  ],\n";
    out << "  \"caseCount\": " << result.caseCount << ",\n";
    out << "  \"passedCaseCount\": " << result.passedCaseCount << "\n";
    out << "}\n";
    return out.str();
}

std::string makeSustainDetectorErrorsJson (const SustainDetectorProofResult& result)
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
