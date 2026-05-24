#include "AnalyzerDetectorFixture.h"
#include "RuntimeRegistry.h"

#include <algorithm>
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

const myworld::RuntimeOpModuleDiagnostic* findDiagnostic (
    const std::vector<myworld::RuntimeOpModuleDiagnostic>& diagnostics,
    const std::string& nodeType)
{
    for (const auto& diagnostic : diagnostics)
        if (diagnostic.nodeType == nodeType)
            return &diagnostic;

    return nullptr;
}

const myworld::AttackDetectorCaseResult* findCase (const myworld::AttackDetectorProofResult& result,
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
    const std::string libraryPath = "fixtures/module-libraries/pv-attack-detector.module-library.json";
    const std::string fixturePath = "fixtures/analyzer/attack_detector_cases.json";

    const auto fixture = myworld::loadAttackDetectorFixture (fixturePath);
    expect (fixture.ok, fixture.error);
    expect (fixture.fixture.cases.size() == 5, "attack fixture case count");
    expectNear (fixture.fixture.parameters.floor, 0.02, 0.000001, "attack fixture floor");
    expectNear (fixture.fixture.parameters.riseThreshold, 0.08, 0.000001, "attack fixture rise threshold");

    const auto runtime = myworld::loadRuntimeRegistryFromModuleLibrary (libraryPath);
    expect (runtime.ok, runtime.error);
    expect (runtime.registry.entries.size() == 2, "pv attack registry entry count");

    const auto* attackEntry = findRuntimeEntry (runtime.registry, "compound.attack");
    expect (attackEntry != nullptr, "runtime registry contains compound.attack");
    expect (attackEntry->children.size() == 5, "attack child count");
    expect (attackEntry->internalEdges.size() == 10, "attack internal edge count");
    expect (attackEntry->publicOutputs.size() == 4, "attack output count");
    expectEqual (attackEntry->publicOutputs.at (0), "onset_event", "attack first output");
    expectEqual (attackEntry->publicOutputs.at (1), "attack_value", "attack second output");

    const auto coverage = myworld::inspectRuntimeOpCoverage (runtime.registry);
    expect (coverage.ok, coverage.error);
    expect (coverage.snapshot.missingChildCount == 0, "pv attack coverage has no missing RuntimeOps");

    const auto diagnostics = myworld::makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    const auto* attackDiagnostic = findDiagnostic (diagnostics, "compound.attack");
    expect (attackDiagnostic != nullptr, "attack runtime diagnostic exists");
    expectEqual (attackDiagnostic->status, "runtime-op-ready", "attack diagnostic status");
    expect (myworld::runtimeOpDiagnosticAllowsCreation (*attackDiagnostic), "attack diagnostic allows creation");

    const auto proof = myworld::runAttackDetectorProof (runtime.registry, fixturePath);
    expect (proof.ok, proof.error);
    expectEqual (proof.operation, "pv_attack_detector", "proof operation");
    expectEqual (proof.selectedDetector, "attack", "selected detector");
    expect (proof.usesRawEnergyFacts, "proof uses raw energy facts");
    expect (! proof.usesOutputSmoothingForDetector, "proof does not use output smoothing for detector");
    expect (! proof.densityImplemented, "proof does not implement density");
    expect (! proof.silenceImplemented, "proof does not implement silence");
    expect (proof.caseCount == 5, "proof case count");
    expect (proof.passedCaseCount == 5, "proof passed case count");

    const auto* quiet = findCase (proof, "quiet_no_input");
    expect (quiet != nullptr, "quiet case present");
    expect (quiet->passed, quiet->message);
    expect (quiet->onsetEventCount == 0, "quiet case onset count");
    expectNear (quiet->confidenceLast, 0.0, 0.000001, "quiet confidence");

    const auto* cleanEntry = findCase (proof, "single_clean_entry");
    expect (cleanEntry != nullptr, "clean entry case present");
    expect (cleanEntry->passed, cleanEntry->message);
    expect (cleanEntry->onsetEventCount == 1, "clean entry onset count");
    expect (cleanEntry->firstOnsetFrame == 2, "clean entry first onset frame");
    expect (cleanEntry->attackValueMax >= 0.10, "clean entry attack value");

    const auto* steady = findCase (proof, "steady_loud_no_retrigger");
    expect (steady != nullptr, "steady case present");
    expect (steady->passed, steady->message);
    expect (steady->onsetEventCount == 0, "steady case onset count");

    const auto* wiggle = findCase (proof, "micro_wiggle_below_threshold");
    expect (wiggle != nullptr, "wiggle case present");
    expect (wiggle->passed, wiggle->message);
    expect (wiggle->onsetEventCount == 0, "wiggle case onset count");

    const auto* missing = findCase (proof, "missing_rms_failure");
    expect (missing != nullptr, "missing rms case present");
    expect (missing->passed, missing->message);
    expect (! missing->detectorOk, "missing rms detector reports failure");
    expectEqual (missing->diagnostic, "missing_input:rms", "missing rms diagnostic");

    const auto report = myworld::makeAttackDetectorProofReportJson (proof);
    expect (report.find ("\"operation\": \"pv_attack_detector\"") != std::string::npos,
            "report operation json");
    expect (report.find ("\"usesRawEnergyFacts\": true") != std::string::npos,
            "report raw facts json");
    expect (report.find ("\"usesOutputSmoothingForDetector\": false") != std::string::npos,
            "report smoothing json");
    expect (report.find ("\"densityImplemented\": false") != std::string::npos,
            "report density json");
    expect (report.find ("\"silenceImplemented\": false") != std::string::npos,
            "report silence json");

    std::cout << "analyzer detector ok\n";
    return 0;
}
