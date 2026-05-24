#include "AnalyzerSilenceDetectorFixture.h"
#include "RuntimeRegistry.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}

void expectEqual (const std::string& actual, const std::string& expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + expected + " got " + actual);
}

void expectNear (double actual, double expected, double tolerance, const std::string& message)
{
    expect (std::abs (actual - expected) <= tolerance,
            message + " expected near " + std::to_string (expected) + " got " + std::to_string (actual));
}

const myworld::RuntimeRegistryEntry* findRuntimeEntry (const myworld::RuntimeRegistry& registry,
                                                       const std::string& nodeType)
{
    for (const auto& entry : registry.entries)
        if (entry.nodeType == nodeType)
            return &entry;

    return nullptr;
}

const myworld::SilenceDetectorCaseResult* findCase (const myworld::SilenceDetectorProofResult& result,
                                                    const std::string& id)
{
    for (const auto& testCase : result.cases)
        if (testCase.id == id)
            return &testCase;

    return nullptr;
}
}

int main()
{
    const std::string libraryPath = "fixtures/module-libraries/pv-silence-detector.module-library.json";
    const std::string fixturePath = "fixtures/analyzer/silence_detector_cases.json";

    const auto fixture = myworld::loadSilenceDetectorFixture (fixturePath);
    expect (fixture.ok, fixture.error);
    expect (fixture.fixture.cases.size() == 5, "silence fixture case count");
    expectNear (fixture.fixture.parameters.floor, 0.02, 0.000001, "silence fixture floor");
    expectNear (fixture.fixture.parameters.holdMs, 300.0, 0.000001, "silence fixture hold");

    const auto runtime = myworld::loadRuntimeRegistryFromModuleLibrary (libraryPath);
    expect (runtime.ok, runtime.error);
    expect (runtime.registry.entries.size() == 1, "pv silence registry entry count");

    const auto* silenceEntry = findRuntimeEntry (runtime.registry, "compound.silence");
    expect (silenceEntry != nullptr, "runtime registry contains compound.silence");
    expect (silenceEntry->children.size() == 5, "silence child count");
    expect (silenceEntry->internalEdges.size() == 8, "silence internal edge count");
    expect (silenceEntry->publicOutputs.size() == 3, "silence output count");
    expectEqual (silenceEntry->publicOutputs.at (0), "silence_state", "silence first output");
    expectEqual (silenceEntry->publicOutputs.at (1), "silence_timer_ms", "silence second output");

    const auto coverage = myworld::inspectRuntimeOpCoverage (runtime.registry);
    expect (coverage.ok, coverage.error);
    expect (coverage.snapshot.missingChildCount == 0, "pv silence coverage has no missing RuntimeOps");

    const auto proof = myworld::runSilenceDetectorProof (runtime.registry, fixturePath);
    expect (proof.ok, proof.error);
    expectEqual (proof.operation, "pv_silence_detector", "proof operation");
    expectEqual (proof.selectedDetector, "silence", "selected detector");
    expect (proof.usesRawRms, "proof uses raw rms");
    expect (! proof.attackImplemented, "proof does not implement attack");
    expect (! proof.densityImplemented, "proof does not implement density");
    expect (proof.caseCount == 5, "proof case count");
    expect (proof.passedCaseCount == 5, "proof passed case count");

    const auto* quiet = findCase (proof, "quiet_holds_until_silence");
    expect (quiet != nullptr, "quiet silence case present");
    expect (quiet->passed, quiet->message);
    expect (quiet->silenceStateLast, "quiet reaches silence");
    expectNear (quiet->silenceTimerMsLast, 300.0, 0.000001, "quiet timer");

    const auto* activeReset = findCase (proof, "active_resets_timer");
    expect (activeReset != nullptr, "active reset case present");
    expect (activeReset->passed, activeReset->message);
    expect (! activeReset->silenceStateLast, "active reset is not silence");
    expectNear (activeReset->silenceTimerMsLast, 100.0, 0.000001, "active reset timer");

    const auto* briefGap = findCase (proof, "brief_gap_not_silence");
    expect (briefGap != nullptr, "brief gap case present");
    expect (briefGap->passed, briefGap->message);
    expect (! briefGap->silenceStateLast, "brief gap is not silence");

    const auto* noiseFloor = findCase (proof, "noise_floor_above_floor");
    expect (noiseFloor != nullptr, "noise floor case present");
    expect (noiseFloor->passed, noiseFloor->message);
    expect (! noiseFloor->silenceStateLast, "noise floor is not silence");

    const auto* missing = findCase (proof, "missing_rms_failure");
    expect (missing != nullptr, "missing rms case present");
    expect (missing->passed, missing->message);
    expect (! missing->detectorOk, "missing rms detector reports failure");
    expectEqual (missing->diagnostic, "missing_input:rms", "missing rms diagnostic");

    const auto report = myworld::makeSilenceDetectorProofReportJson (proof);
    expect (report.find ("\"operation\": \"pv_silence_detector\"") != std::string::npos,
            "report operation json");
    expect (report.find ("\"usesRawRms\": true") != std::string::npos,
            "report raw rms json");
    expect (report.find ("\"attackImplemented\": false") != std::string::npos,
            "report attack json");
    expect (report.find ("\"densityImplemented\": false") != std::string::npos,
            "report density json");

    std::cout << "analyzer silence detector ok\n";
    return 0;
}
