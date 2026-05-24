#include "AnalyzerSustainDetectorFixture.h"
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

const myworld::SustainDetectorCaseResult* findCase (const myworld::SustainDetectorProofResult& result,
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
    const std::string libraryPath = "fixtures/module-libraries/pv-sustain-detector.module-library.json";
    const std::string fixturePath = "fixtures/analyzer/sustain_detector_cases.json";

    const auto fixture = myworld::loadSustainDetectorFixture (fixturePath);
    expect (fixture.ok, fixture.error);
    expect (fixture.fixture.cases.size() == 5, "sustain fixture case count");
    expectNear (fixture.fixture.parameters.floor, 0.02, 0.000001, "sustain fixture floor");
    expectNear (fixture.fixture.parameters.holdMs, 300.0, 0.000001, "sustain fixture hold");

    const auto runtime = myworld::loadRuntimeRegistryFromModuleLibrary (libraryPath);
    expect (runtime.ok, runtime.error);
    expect (runtime.registry.entries.size() == 1, "pv sustain registry entry count");

    const auto* sustainEntry = findRuntimeEntry (runtime.registry, "compound.sustain");
    expect (sustainEntry != nullptr, "runtime registry contains compound.sustain");
    expect (sustainEntry->children.size() == 5, "sustain child count");
    expect (sustainEntry->internalEdges.size() == 9, "sustain internal edge count");
    expect (sustainEntry->publicOutputs.size() == 4, "sustain output count");
    expectEqual (sustainEntry->publicOutputs.at (0), "sustain_state", "sustain first output");
    expectEqual (sustainEntry->publicOutputs.at (1), "sustain_timer_ms", "sustain second output");
    expectEqual (sustainEntry->publicOutputs.at (2), "sustain_envelope", "sustain third output");

    const auto coverage = myworld::inspectRuntimeOpCoverage (runtime.registry);
    expect (coverage.ok, coverage.error);
    expect (coverage.snapshot.missingChildCount == 0, "pv sustain coverage has no missing RuntimeOps");

    const auto proof = myworld::runSustainDetectorProof (runtime.registry, fixturePath);
    expect (proof.ok, proof.error);
    expectEqual (proof.operation, "pv_sustain_detector", "proof operation");
    expectEqual (proof.selectedDetector, "sustain", "selected detector");
    expect (proof.usesRawRms, "proof uses raw rms");
    expect (! proof.usesAttackOnsetForDetector, "proof does not use attack onset");
    expect (! proof.usesSilenceStateForDetector, "proof does not use silence state");
    expect (proof.caseCount == 5, "proof case count");
    expect (proof.passedCaseCount == 5, "proof passed case count");

    const auto* held = findCase (proof, "held_active_reaches_sustain");
    expect (held != nullptr, "held sustain case present");
    expect (held->passed, held->message);
    expect (held->sustainStateLast, "held active reaches sustain");
    expectNear (held->sustainTimerMsLast, 300.0, 0.000001, "held sustain timer");
    expectNear (held->sustainEnvelopeLast, 1.0, 0.000001, "held sustain envelope");

    const auto* shortActive = findCase (proof, "short_active_not_sustain");
    expect (shortActive != nullptr, "short active case present");
    expect (shortActive->passed, shortActive->message);
    expect (! shortActive->sustainStateLast, "short active is not sustain");
    expectNear (shortActive->sustainTimerMsLast, 100.0, 0.000001, "short active timer");

    const auto* reset = findCase (proof, "quiet_gap_resets_timer");
    expect (reset != nullptr, "quiet reset case present");
    expect (reset->passed, reset->message);
    expect (! reset->sustainStateLast, "quiet gap resets sustain state");
    expectNear (reset->sustainTimerMsLast, 100.0, 0.000001, "quiet reset timer");

    const auto* belowFloor = findCase (proof, "noise_below_floor_not_sustain");
    expect (belowFloor != nullptr, "below-floor case present");
    expect (belowFloor->passed, belowFloor->message);
    expect (! belowFloor->sustainStateLast, "below-floor noise is not sustain");
    expectNear (belowFloor->sustainTimerMsLast, 0.0, 0.000001, "below-floor timer");

    const auto* missing = findCase (proof, "missing_rms_failure");
    expect (missing != nullptr, "missing rms case present");
    expect (missing->passed, missing->message);
    expect (! missing->detectorOk, "missing rms detector reports failure");
    expectEqual (missing->diagnostic, "missing_input:rms", "missing rms diagnostic");

    const auto report = myworld::makeSustainDetectorProofReportJson (proof);
    expect (report.find ("\"operation\": \"pv_sustain_detector\"") != std::string::npos,
            "report operation json");
    expect (report.find ("\"usesRawRms\": true") != std::string::npos,
            "report raw rms json");
    expect (report.find ("\"usesAttackOnsetForDetector\": false") != std::string::npos,
            "report attack onset json");
    expect (report.find ("\"usesSilenceStateForDetector\": false") != std::string::npos,
            "report silence state json");

    std::cout << "analyzer sustain detector ok\n";
    return 0;
}
