#include "AnalyzerSilenceDetectorFixture.h"

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

constexpr const char* silenceNodeType = "compound.silence";

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

SilenceDetectorFixtureFrame parseFrame (const JsonValue& value)
{
    SilenceDetectorFixtureFrame frame;
    frame.hasRms = hasNumberMember (value, "rms");
    frame.rms = numberMember (value, "rms", 0.0);
    frame.hasPeak = hasNumberMember (value, "peak");
    frame.peak = numberMember (value, "peak", 0.0);
    return frame;
}

SilenceDetectorFixtureExpectation parseExpectation (const JsonValue& value)
{
    SilenceDetectorFixtureExpectation expectation;
    expectation.detectorOk = boolMember (value, "detectorOk", true);
    expectation.hasSilenceStateLast = hasBoolMember (value, "silenceStateLast");
    expectation.silenceStateLast = boolMember (value, "silenceStateLast", false);
    expectation.hasSilenceTimerMsLast = hasNumberMember (value, "silenceTimerMsLast");
    expectation.silenceTimerMsLast = numberMember (value, "silenceTimerMsLast", 0.0);
    expectation.diagnostic = stringMember (value, "diagnostic");
    return expectation;
}

bool registryContainsSilence (const RuntimeRegistry& registry)
{
    return std::any_of (registry.entries.begin(), registry.entries.end(), [] (const auto& entry) {
        return entry.nodeType == silenceNodeType;
    });
}

bool silenceRuntimeCoverageReady (const RuntimeRegistry& registry)
{
    const auto coverage = inspectRuntimeOpCoverage (registry);
    if (! coverage.ok)
        return false;

    const auto diagnostics = makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    return std::any_of (diagnostics.begin(), diagnostics.end(), [] (const auto& diagnostic) {
        return diagnostic.nodeType == silenceNodeType
               && diagnostic.status == "runtime-op-ready"
               && runtimeOpDiagnosticAllowsCreation (diagnostic);
    });
}

SilenceDetectorFrameInput makeFrameInput (const SilenceDetectorFixtureFrame& frame,
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

SilenceDetectorCaseResult runCase (const SilenceDetectorFixtureCase& testCase,
                                   const SilenceDetectorParameters& parameters,
                                   const double frameIntervalMs)
{
    SilenceDetector detector { parameters };
    SilenceDetectorCaseResult result;
    result.id = testCase.id;

    for (size_t frameIndex = 0; frameIndex < testCase.frames.size(); ++frameIndex)
    {
        const auto output = detector.processFrame (makeFrameInput (testCase.frames[frameIndex],
                                                                   frameIndex,
                                                                   frameIntervalMs));
        result.detectorOk = result.detectorOk && output.detectorOk;
        result.silenceStateLast = output.silenceState;
        result.silenceTimerMsLast = output.silenceTimerMs;
        result.confidenceLast = output.confidence;

        if (! output.diagnostic.empty() && result.diagnostic.empty())
            result.diagnostic = output.diagnostic;
    }

    const auto& expectation = testCase.expect;
    result.passed = result.detectorOk == expectation.detectorOk;
    if (! result.passed)
        result.message = "detectorOk mismatch";

    if (result.passed && expectation.hasSilenceStateLast)
    {
        result.passed = result.silenceStateLast == expectation.silenceStateLast;
        if (! result.passed)
            result.message = "silenceStateLast mismatch";
    }

    if (result.passed && expectation.hasSilenceTimerMsLast)
    {
        result.passed = std::abs (result.silenceTimerMsLast - expectation.silenceTimerMsLast) < 0.000001;
        if (! result.passed)
            result.message = "silenceTimerMsLast mismatch";
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

void appendCaseResultJson (std::ostream& out, const SilenceDetectorCaseResult& result)
{
    out << "    {\n";
    out << "      \"id\": " << jsonQuoted (result.id) << ",\n";
    out << "      \"passed\": " << (result.passed ? "true" : "false") << ",\n";
    out << "      \"detectorOk\": " << (result.detectorOk ? "true" : "false") << ",\n";
    out << "      \"silenceStateLast\": " << (result.silenceStateLast ? "true" : "false") << ",\n";
    out << "      \"silenceTimerMsLast\": " << result.silenceTimerMsLast << ",\n";
    out << "      \"confidenceLast\": " << result.confidenceLast << ",\n";
    out << "      \"diagnostic\": " << jsonQuoted (result.diagnostic) << ",\n";
    out << "      \"message\": " << jsonQuoted (result.message) << "\n";
    out << "    }";
}
}

SilenceDetectorFixtureLoadResult loadSilenceDetectorFixture (const std::string& fixturePath)
{
    const auto text = readTextFile (fixturePath);
    if (text.empty())
        return { false, {}, "could not read silence detector fixture: " + fixturePath };

    JsonParser parser { text };
    const auto root = parser.parse();
    if (! parser.ok())
        return { false, {}, parser.error() };

    SilenceDetectorFixture fixture;
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
        return { false, {}, "silence detector fixture missing cases array" };

    for (const auto& caseValue : cases->arrayValue)
    {
        SilenceDetectorFixtureCase testCase;
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

SilenceDetectorProofResult runSilenceDetectorProof (const RuntimeRegistry& registry, const std::string& fixturePath)
{
    SilenceDetectorProofResult result;
    result.fixturePath = fixturePath;

    if (! registryContainsSilence (registry))
    {
        result.error = "runtime registry missing compound.silence";
        return result;
    }

    if (! silenceRuntimeCoverageReady (registry))
    {
        result.error = "compound.silence runtime coverage is not ready";
        return result;
    }

    const auto fixture = loadSilenceDetectorFixture (fixturePath);
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
    result.error = result.ok ? std::string {} : "silence detector fixture cases did not all pass";
    return result;
}

std::string makeSilenceDetectorProofReportJson (const SilenceDetectorProofResult& result)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"pvSilenceDetectorProof\",\n";
    out << "  \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    out << "  \"operation\": " << jsonQuoted (result.operation) << ",\n";
    out << "  \"selectedDetector\": " << jsonQuoted (result.selectedDetector) << ",\n";
    out << "  \"fixturePath\": " << jsonQuoted (result.fixturePath) << ",\n";
    out << "  \"caseCount\": " << result.caseCount << ",\n";
    out << "  \"passedCaseCount\": " << result.passedCaseCount << ",\n";
    out << "  \"usesRawRms\": " << (result.usesRawRms ? "true" : "false") << ",\n";
    out << "  \"attackImplemented\": " << (result.attackImplemented ? "true" : "false") << ",\n";
    out << "  \"densityImplemented\": " << (result.densityImplemented ? "true" : "false") << ",\n";
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

std::string makeSilenceDetectorCookOrderJson()
{
    return "{\n"
           "  \"version\": 1,\n"
           "  \"mode\": \"pv-silence-detector\",\n"
           "  \"cookOrder\": [\"raw-energy.rms\", \"analyzer.silence\", \"silence_out\"]\n"
           "}\n";
}

std::string makeSilenceDetectorNodeStatsJson (const SilenceDetectorProofResult& result)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"mode\": \"pv-silence-detector\",\n";
    out << "  \"renderer\": \"headless-analyzer\",\n";
    out << "  \"nodes\": [\n";
    out << "    { \"id\": \"raw-energy.rms\", \"type\": \"signal.float\", \"status\": \"fixture-source\" },\n";
    out << "    { \"id\": \"silence\", \"type\": \"analyzer.silence\", \"status\": "
        << jsonQuoted (result.ok ? "computed" : "failed") << " },\n";
    out << "    { \"id\": \"silence_out\", \"type\": \"analyzer.silence_out\", \"status\": "
        << jsonQuoted (result.ok ? "computed" : "failed") << " }\n";
    out << "  ],\n";
    out << "  \"caseCount\": " << result.caseCount << ",\n";
    out << "  \"passedCaseCount\": " << result.passedCaseCount << "\n";
    out << "}\n";
    return out.str();
}

std::string makeSilenceDetectorErrorsJson (const SilenceDetectorProofResult& result)
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
