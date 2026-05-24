#include "AnalyzerDetectorFixture.h"

#include "JsonWriter.h"
#include "StorageContractJson.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace myworld
{
namespace
{
using storage_contract_internal::JsonValue;
using storage_contract_internal::JsonParser;
using storage_contract_internal::boolMember;
using storage_contract_internal::member;
using storage_contract_internal::numberMember;
using storage_contract_internal::stringMember;

constexpr const char* attackNodeType = "compound.attack";

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

AttackDetectorFixtureFrame parseFrame (const JsonValue& value)
{
    AttackDetectorFixtureFrame frame;
    frame.hasRms = hasNumberMember (value, "rms");
    frame.rms = numberMember (value, "rms", 0.0);
    frame.hasPeak = hasNumberMember (value, "peak");
    frame.peak = numberMember (value, "peak", 0.0);
    frame.hasSampleCount = hasNumberMember (value, "sampleCount");
    frame.sampleCount = numberMember (value, "sampleCount", 0.0);
    return frame;
}

AttackDetectorFixtureExpectation parseExpectation (const JsonValue& value)
{
    AttackDetectorFixtureExpectation expectation;
    expectation.detectorOk = boolMember (value, "detectorOk", true);
    expectation.hasOnsetEventCount = hasNumberMember (value, "onsetEventCount");
    expectation.onsetEventCount = static_cast<int> (numberMember (value, "onsetEventCount", 0.0));
    expectation.hasFirstOnsetFrame = hasNumberMember (value, "firstOnsetFrame");
    expectation.firstOnsetFrame = static_cast<int> (numberMember (value, "firstOnsetFrame", -1.0));
    expectation.hasConfidenceLast = hasNumberMember (value, "confidenceLast");
    expectation.confidenceLast = numberMember (value, "confidenceLast", 0.0);
    expectation.hasAttackValueMin = hasNumberMember (value, "attackValueMin");
    expectation.attackValueMin = numberMember (value, "attackValueMin", 0.0);
    expectation.diagnostic = stringMember (value, "diagnostic");
    expectation.reason = stringMember (value, "reason");
    return expectation;
}

bool registryContainsAttack (const RuntimeRegistry& registry)
{
    return std::any_of (registry.entries.begin(), registry.entries.end(), [] (const auto& entry) {
        return entry.nodeType == attackNodeType;
    });
}

bool attackRuntimeCoverageReady (const RuntimeRegistry& registry)
{
    const auto coverage = inspectRuntimeOpCoverage (registry);
    if (! coverage.ok)
        return false;

    const auto diagnostics = makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    return std::any_of (diagnostics.begin(), diagnostics.end(), [] (const auto& diagnostic) {
        return diagnostic.nodeType == attackNodeType
               && diagnostic.status == "runtime-op-ready"
               && runtimeOpDiagnosticAllowsCreation (diagnostic);
    });
}

AttackDetectorFrameInput makeFrameInput (const AttackDetectorFixtureFrame& frame,
                                         const size_t frameIndex,
                                         const double frameIntervalMs)
{
    return {
        frame.hasRms,
        frame.rms,
        frame.hasPeak,
        frame.peak,
        frame.hasSampleCount,
        frame.sampleCount,
        static_cast<double> (frameIndex) * frameIntervalMs
    };
}

AttackDetectorCaseResult runCase (const AttackDetectorFixtureCase& testCase,
                                  const AttackDetectorParameters& parameters,
                                  const double frameIntervalMs)
{
    AttackDetector detector { parameters };
    AttackDetectorCaseResult result;
    result.id = testCase.id;

    for (size_t frameIndex = 0; frameIndex < testCase.frames.size(); ++frameIndex)
    {
        const auto output = detector.processFrame (makeFrameInput (testCase.frames[frameIndex],
                                                                   frameIndex,
                                                                   frameIntervalMs));
        result.detectorOk = result.detectorOk && output.detectorOk;
        result.confidenceLast = output.confidence;
        result.attackValueMax = std::max (result.attackValueMax, output.attackValue);

        if (! output.diagnostic.empty() && result.diagnostic.empty())
            result.diagnostic = output.diagnostic;

        result.warnings.insert (result.warnings.end(), output.warnings.begin(), output.warnings.end());

        if (output.onsetEvent)
        {
            ++result.onsetEventCount;
            if (result.firstOnsetFrame < 0)
                result.firstOnsetFrame = static_cast<int> (frameIndex);
        }
    }

    const auto& expectation = testCase.expect;
    result.passed = result.detectorOk == expectation.detectorOk;
    if (! result.passed)
        result.message = "detectorOk mismatch";

    if (result.passed && expectation.hasOnsetEventCount)
    {
        result.passed = result.onsetEventCount == expectation.onsetEventCount;
        if (! result.passed)
            result.message = "onsetEventCount mismatch";
    }

    if (result.passed && expectation.hasFirstOnsetFrame)
    {
        result.passed = result.firstOnsetFrame == expectation.firstOnsetFrame;
        if (! result.passed)
            result.message = "firstOnsetFrame mismatch";
    }

    if (result.passed && expectation.hasConfidenceLast)
    {
        result.passed = std::abs (result.confidenceLast - expectation.confidenceLast) < 0.000001;
        if (! result.passed)
            result.message = "confidenceLast mismatch";
    }

    if (result.passed && expectation.hasAttackValueMin)
    {
        result.passed = result.attackValueMax >= expectation.attackValueMin;
        if (! result.passed)
            result.message = "attackValueMax below expected minimum";
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

void appendCaseResultJson (std::ostream& out, const AttackDetectorCaseResult& result)
{
    out << "    {\n";
    out << "      \"id\": " << jsonQuoted (result.id) << ",\n";
    out << "      \"passed\": " << (result.passed ? "true" : "false") << ",\n";
    out << "      \"detectorOk\": " << (result.detectorOk ? "true" : "false") << ",\n";
    out << "      \"onsetEventCount\": " << result.onsetEventCount << ",\n";
    out << "      \"firstOnsetFrame\": " << result.firstOnsetFrame << ",\n";
    out << "      \"attackValueMax\": " << result.attackValueMax << ",\n";
    out << "      \"confidenceLast\": " << result.confidenceLast << ",\n";
    out << "      \"diagnostic\": " << jsonQuoted (result.diagnostic) << ",\n";
    out << "      \"message\": " << jsonQuoted (result.message) << ",\n";
    out << "      \"warnings\": ";
    appendJsonStringArray (out, result.warnings);
    out << "\n";
    out << "    }";
}
}

AttackDetectorFixtureLoadResult loadAttackDetectorFixture (const std::string& fixturePath)
{
    const auto text = readTextFile (fixturePath);
    if (text.empty())
        return { false, {}, "could not read attack detector fixture: " + fixturePath };

    JsonParser parser { text };
    const auto root = parser.parse();
    if (! parser.ok())
        return { false, {}, parser.error() };

    AttackDetectorFixture fixture;
    fixture.id = stringMember (root, "id");
    fixture.frameIntervalMs = numberMember (root, "frameIntervalMs", 40.0);

    if (const auto* parameters = member (root, "parameters"))
    {
        fixture.parameters.floor = numberMember (*parameters, "floor", fixture.parameters.floor);
        fixture.parameters.riseThreshold = numberMember (*parameters,
                                                         "riseThreshold",
                                                         fixture.parameters.riseThreshold);
        fixture.parameters.debounceMs = numberMember (*parameters, "debounceMs", fixture.parameters.debounceMs);
        fixture.parameters.releaseMs = numberMember (*parameters, "releaseMs", fixture.parameters.releaseMs);
        fixture.parameters.outputSmoothMs = numberMember (*parameters,
                                                          "outputSmoothMs",
                                                          fixture.parameters.outputSmoothMs);
    }

    const auto* cases = member (root, "cases");
    if (cases == nullptr || cases->kind != JsonValue::Kind::array)
        return { false, {}, "attack detector fixture missing cases array" };

    for (const auto& caseValue : cases->arrayValue)
    {
        AttackDetectorFixtureCase testCase;
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

AttackDetectorProofResult runAttackDetectorProof (const RuntimeRegistry& registry, const std::string& fixturePath)
{
    AttackDetectorProofResult result;
    result.fixturePath = fixturePath;

    if (! registryContainsAttack (registry))
    {
        result.error = "runtime registry missing compound.attack";
        return result;
    }

    if (! attackRuntimeCoverageReady (registry))
    {
        result.error = "compound.attack runtime coverage is not ready";
        return result;
    }

    const auto fixture = loadAttackDetectorFixture (fixturePath);
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
    result.error = result.ok ? std::string {} : "attack detector fixture cases did not all pass";
    return result;
}

std::string makeAttackDetectorProofReportJson (const AttackDetectorProofResult& result)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"pvAttackDetectorProof\",\n";
    out << "  \"ok\": " << (result.ok ? "true" : "false") << ",\n";
    out << "  \"operation\": " << jsonQuoted (result.operation) << ",\n";
    out << "  \"selectedDetector\": " << jsonQuoted (result.selectedDetector) << ",\n";
    out << "  \"fixturePath\": " << jsonQuoted (result.fixturePath) << ",\n";
    out << "  \"caseCount\": " << result.caseCount << ",\n";
    out << "  \"passedCaseCount\": " << result.passedCaseCount << ",\n";
    out << "  \"usesRawEnergyFacts\": " << (result.usesRawEnergyFacts ? "true" : "false") << ",\n";
    out << "  \"usesOutputSmoothingForDetector\": "
        << (result.usesOutputSmoothingForDetector ? "true" : "false") << ",\n";
    out << "  \"densityImplemented\": " << (result.densityImplemented ? "true" : "false") << ",\n";
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

std::string makeAttackDetectorCookOrderJson()
{
    return "{\n"
           "  \"version\": 1,\n"
           "  \"mode\": \"pv-attack-detector\",\n"
           "  \"cookOrder\": [\"raw-energy\", \"analyzer.attack\", \"attack_out\"]\n"
           "}\n";
}

std::string makeAttackDetectorNodeStatsJson (const AttackDetectorProofResult& result)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"mode\": \"pv-attack-detector\",\n";
    out << "  \"renderer\": \"headless-analyzer\",\n";
    out << "  \"nodes\": [\n";
    out << "    { \"id\": \"raw-energy\", \"type\": \"compound.raw-energy\", \"status\": \"fixture-source\" },\n";
    out << "    { \"id\": \"attack\", \"type\": \"analyzer.attack\", \"status\": "
        << jsonQuoted (result.ok ? "computed" : "failed") << " },\n";
    out << "    { \"id\": \"attack_out\", \"type\": \"analyzer.attack_out\", \"status\": "
        << jsonQuoted (result.ok ? "computed" : "failed") << " }\n";
    out << "  ],\n";
    out << "  \"caseCount\": " << result.caseCount << ",\n";
    out << "  \"passedCaseCount\": " << result.passedCaseCount << "\n";
    out << "}\n";
    return out.str();
}

std::string makeAttackDetectorErrorsJson (const AttackDetectorProofResult& result)
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
