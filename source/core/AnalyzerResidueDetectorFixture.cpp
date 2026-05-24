#include "AnalyzerResidueDetectorFixture.h"

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

constexpr const char* residueNodeType = "compound.residue";

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

ResidueDetectorFixtureFrame parseFrame (const JsonValue& value)
{
    ResidueDetectorFixtureFrame frame;
    frame.hasRms = hasNumberMember (value, "rms");
    frame.rms = numberMember (value, "rms", 0.0);
    frame.hasSustainEnvelope = hasNumberMember (value, "sustain_envelope");
    frame.sustainEnvelope = numberMember (value, "sustain_envelope", 0.0);
    frame.hasSilenceState = hasBoolMember (value, "silence_state");
    frame.silenceState = boolMember (value, "silence_state", false);
    return frame;
}

ResidueDetectorFixtureExpectation parseExpectation (const JsonValue& value)
{
    ResidueDetectorFixtureExpectation expectation;
    expectation.detectorOk = boolMember (value, "detectorOk", true);
    expectation.hasResidueStateLast = hasBoolMember (value, "residueStateLast");
    expectation.residueStateLast = boolMember (value, "residueStateLast", false);
    expectation.hasResidueEnvelopeLast = hasNumberMember (value, "residueEnvelopeLast");
    expectation.residueEnvelopeLast = numberMember (value, "residueEnvelopeLast", 0.0);
    expectation.hasResidueTimerMsLast = hasNumberMember (value, "residueTimerMsLast");
    expectation.residueTimerMsLast = numberMember (value, "residueTimerMsLast", 0.0);
    expectation.diagnostic = stringMember (value, "diagnostic");
    return expectation;
}

bool registryContainsResidue (const RuntimeRegistry& registry)
{
    return std::any_of (registry.entries.begin(), registry.entries.end(), [] (const auto& entry) {
        return entry.nodeType == residueNodeType;
    });
}

bool residueRuntimeCoverageReady (const RuntimeRegistry& registry)
{
    const auto coverage = inspectRuntimeOpCoverage (registry);
    if (! coverage.ok)
        return false;

    const auto diagnostics = makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    return std::any_of (diagnostics.begin(), diagnostics.end(), [] (const auto& diagnostic) {
        return diagnostic.nodeType == residueNodeType
               && diagnostic.status == "runtime-op-ready"
               && runtimeOpDiagnosticAllowsCreation (diagnostic);
    });
}

ResidueDetectorFrameInput makeFrameInput (const ResidueDetectorFixtureFrame& frame,
                                          const size_t frameIndex,
                                          const double frameIntervalMs)
{
    return {
        frame.hasRms,
        frame.rms,
        frame.hasSustainEnvelope,
        frame.sustainEnvelope,
        frame.hasSilenceState,
        frame.silenceState,
        static_cast<double> (frameIndex) * frameIntervalMs
    };
}

ResidueDetectorCaseResult runCase (const ResidueDetectorFixtureCase& testCase,
                                   const ResidueDetectorParameters& parameters,
                                   const double frameIntervalMs)
{
    ResidueDetector detector { parameters };
    ResidueDetectorCaseResult result;
    result.id = testCase.id;

    for (size_t frameIndex = 0; frameIndex < testCase.frames.size(); ++frameIndex)
    {
        const auto output = detector.processFrame (makeFrameInput (testCase.frames[frameIndex],
                                                                   frameIndex,
                                                                   frameIntervalMs));
        result.detectorOk = result.detectorOk && output.detectorOk;
        result.residueStateLast = output.residueState;
        result.residueEnvelopeLast = output.residueEnvelope;
        result.residueTimerMsLast = output.residueTimerMs;
        result.confidenceLast = output.confidence;

        if (! output.diagnostic.empty() && result.diagnostic.empty())
            result.diagnostic = output.diagnostic;
    }

    const auto& expectation = testCase.expect;
    result.passed = result.detectorOk == expectation.detectorOk;
    if (! result.passed)
        result.message = "detectorOk mismatch";

    if (result.passed && expectation.hasResidueStateLast)
    {
        result.passed = result.residueStateLast == expectation.residueStateLast;
        if (! result.passed)
            result.message = "residueStateLast mismatch";
    }

    if (result.passed && expectation.hasResidueEnvelopeLast)
    {
        result.passed = std::abs (result.residueEnvelopeLast - expectation.residueEnvelopeLast) < 0.000001;
        if (! result.passed)
            result.message = "residueEnvelopeLast mismatch";
    }

    if (result.passed && expectation.hasResidueTimerMsLast)
    {
        result.passed = std::abs (result.residueTimerMsLast - expectation.residueTimerMsLast) < 0.000001;
        if (! result.passed)
            result.message = "residueTimerMsLast mismatch";
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

void appendCaseResultJson (std::ostream& out, const ResidueDetectorCaseResult& result)
{
    out << "    {\n";
    out << "      \"id\": " << jsonQuoted (result.id) << ",\n";
    out << "      \"passed\": " << (result.passed ? "true" : "false") << ",\n";
    out << "      \"detectorOk\": " << (result.detectorOk ? "true" : "false") << ",\n";
    out << "      \"residueStateLast\": " << (result.residueStateLast ? "true" : "false") << ",\n";
    out << "      \"residueEnvelopeLast\": " << result.residueEnvelopeLast << ",\n";
    out << "      \"residueTimerMsLast\": " << result.residueTimerMsLast << ",\n";
    out << "      \"confidenceLast\": " << result.confidenceLast << ",\n";
    out << "      \"diagnostic\": " << jsonQuoted (result.diagnostic) << ",\n";
    out << "      \"message\": " << jsonQuoted (result.message) << "\n";
    out << "    }";
}
}

ResidueDetectorFixtureLoadResult loadResidueDetectorFixture (const std::string& fixturePath)
{
    const auto text = readTextFile (fixturePath);
    if (text.empty())
        return { false, {}, "could not read residue detector fixture: " + fixturePath };

    JsonParser parser { text };
    const auto root = parser.parse();
    if (! parser.ok())
        return { false, {}, parser.error() };

    ResidueDetectorFixture fixture;
    fixture.id = stringMember (root, "id");
    fixture.frameIntervalMs = numberMember (root, "frameIntervalMs", 100.0);

    if (const auto* parameters = member (root, "parameters"))
    {
        fixture.parameters.floor = numberMember (*parameters, "floor", fixture.parameters.floor);
        fixture.parameters.armThreshold = numberMember (*parameters,
                                                        "armThreshold",
                                                        fixture.parameters.armThreshold);
        fixture.parameters.residueThreshold = numberMember (*parameters,
                                                            "residueThreshold",
                                                            fixture.parameters.residueThreshold);
        fixture.parameters.decayMs = numberMember (*parameters, "decayMs", fixture.parameters.decayMs);
        fixture.parameters.silenceClears = boolMember (*parameters,
                                                       "silenceClears",
                                                       fixture.parameters.silenceClears);
        fixture.parameters.outputSmoothMs = numberMember (*parameters,
                                                          "outputSmoothMs",
                                                          fixture.parameters.outputSmoothMs);
    }

    const auto* cases = member (root, "cases");
    if (cases == nullptr || cases->kind != JsonValue::Kind::array)
        return { false, {}, "residue detector fixture missing cases array" };

    for (const auto& caseValue : cases->arrayValue)
    {
        ResidueDetectorFixtureCase testCase;
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

ResidueDetectorProofResult runResidueDetectorProof (const RuntimeRegistry& registry, const std::string& fixturePath)
{
    ResidueDetectorProofResult result;
    result.fixturePath = fixturePath;

    if (! registryContainsResidue (registry))
    {
        result.error = "runtime registry missing compound.residue";
        return result;
    }

    if (! residueRuntimeCoverageReady (registry))
    {
        result.error = "compound.residue runtime coverage is not ready";
        return result;
    }

    const auto fixture = loadResidueDetectorFixture (fixturePath);
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
    result.error = result.ok ? std::string {} : "residue detector fixture cases did not all pass";
    return result;
}

std::string makeResidueDetectorProofReportJson (const ResidueDetectorProofResult& result)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"pvResidueDetectorProof\",\n";
    out << "  \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    out << "  \"operation\": " << jsonQuoted (result.operation) << ",\n";
    out << "  \"selectedDetector\": " << jsonQuoted (result.selectedDetector) << ",\n";
    out << "  \"fixturePath\": " << jsonQuoted (result.fixturePath) << ",\n";
    out << "  \"caseCount\": " << result.caseCount << ",\n";
    out << "  \"passedCaseCount\": " << result.passedCaseCount << ",\n";
    out << "  \"usesRawRms\": " << (result.usesRawRms ? "true" : "false") << ",\n";
    out << "  \"usesSustainEnvelopeForDetector\": "
        << (result.usesSustainEnvelopeForDetector ? "true" : "false") << ",\n";
    out << "  \"usesSilenceStateForClear\": " << (result.usesSilenceStateForClear ? "true" : "false") << ",\n";
    out << "  \"usesOutputSmoothingForDetector\": "
        << (result.usesOutputSmoothingForDetector ? "true" : "false") << ",\n";
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

std::string makeResidueDetectorCookOrderJson()
{
    return "{\n"
           "  \"version\": 1,\n"
           "  \"mode\": \"pv-residue-detector\",\n"
           "  \"cookOrder\": [\"raw-energy.rms\", \"analyzer.sustain\", \"analyzer.silence\", "
           "\"analyzer.residue\", \"residue_out\"]\n"
           "}\n";
}

std::string makeResidueDetectorNodeStatsJson (const ResidueDetectorProofResult& result)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"mode\": \"pv-residue-detector\",\n";
    out << "  \"renderer\": \"headless-analyzer\",\n";
    out << "  \"nodes\": [\n";
    out << "    { \"id\": \"raw-energy.rms\", \"type\": \"signal.float\", \"status\": \"fixture-source\" },\n";
    out << "    { \"id\": \"sustain\", \"type\": \"analyzer.sustain\", \"status\": \"fixture-source\" },\n";
    out << "    { \"id\": \"silence\", \"type\": \"analyzer.silence\", \"status\": \"fixture-source\" },\n";
    out << "    { \"id\": \"residue\", \"type\": \"analyzer.residue\", \"status\": "
        << jsonQuoted (result.ok ? "computed" : "failed") << " },\n";
    out << "    { \"id\": \"residue_out\", \"type\": \"analyzer.residue_out\", \"status\": "
        << jsonQuoted (result.ok ? "computed" : "failed") << " }\n";
    out << "  ],\n";
    out << "  \"caseCount\": " << result.caseCount << ",\n";
    out << "  \"passedCaseCount\": " << result.passedCaseCount << "\n";
    out << "}\n";
    return out.str();
}

std::string makeResidueDetectorErrorsJson (const ResidueDetectorProofResult& result)
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
