#include "AnalyzerDensityDetectorFixture.h"
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

const myworld::DensityDetectorCaseResult* findCase (const myworld::DensityDetectorProofResult& result,
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
    const std::string libraryPath = "fixtures/module-libraries/pv-density-detector.module-library.json";
    const std::string fixturePath = "fixtures/analyzer/density_detector_cases.json";

    const auto fixture = myworld::loadDensityDetectorFixture (fixturePath);
    expect (fixture.ok, fixture.error);
    expect (fixture.fixture.cases.size() == 5, "density fixture case count");
    expectNear (fixture.fixture.parameters.windowMs, 400.0, 0.000001, "density fixture window");
    expectNear (fixture.fixture.parameters.maxEvents, 4.0, 0.000001, "density fixture max events");

    const auto runtime = myworld::loadRuntimeRegistryFromModuleLibrary (libraryPath);
    expect (runtime.ok, runtime.error);
    expect (runtime.registry.entries.size() == 1, "pv density registry entry count");

    const auto* densityEntry = findRuntimeEntry (runtime.registry, "compound.density");
    expect (densityEntry != nullptr, "runtime registry contains compound.density");
    expect (densityEntry->children.size() == 2, "density child count");
    expect (densityEntry->internalEdges.size() == 6, "density internal edge count");
    expect (densityEntry->publicOutputs.size() == 4, "density output count");
    expectEqual (densityEntry->publicOutputs.at (0), "density_value", "density first output");
    expectEqual (densityEntry->publicOutputs.at (1), "event_count", "density second output");

    const auto coverage = myworld::inspectRuntimeOpCoverage (runtime.registry);
    expect (coverage.ok, coverage.error);
    expect (coverage.snapshot.missingChildCount == 0, "pv density coverage has no missing RuntimeOps");

    const auto proof = myworld::runDensityDetectorProof (runtime.registry, fixturePath);
    expect (proof.ok, proof.error);
    expectEqual (proof.operation, "pv_density_detector", "proof operation");
    expectEqual (proof.selectedDetector, "density", "selected detector");
    expect (proof.usesOnsetEvents, "proof uses onset events");
    expect (! proof.usesAttackEnvelopeForDetector, "proof does not use attack envelope for detector");
    expect (! proof.silenceImplemented, "proof does not implement silence");
    expect (proof.caseCount == 5, "proof case count");
    expect (proof.passedCaseCount == 5, "proof passed case count");

    const auto* noOnsets = findCase (proof, "no_onsets");
    expect (noOnsets != nullptr, "no onsets case present");
    expect (noOnsets->passed, noOnsets->message);
    expect (noOnsets->eventCountLast == 0, "no onsets event count");
    expectNear (noOnsets->densityValueLast, 0.0, 0.000001, "no onsets density");

    const auto* threeOnsets = findCase (proof, "three_onsets_inside_window");
    expect (threeOnsets != nullptr, "three onsets case present");
    expect (threeOnsets->passed, threeOnsets->message);
    expect (threeOnsets->eventCountLast == 3, "three onsets event count");
    expectNear (threeOnsets->densityValueLast, 0.75, 0.000001, "three onsets density");

    const auto* expires = findCase (proof, "old_onset_expires");
    expect (expires != nullptr, "old onset expiry case present");
    expect (expires->passed, expires->message);
    expect (expires->eventCountLast == 0, "old onset expiry count");

    const auto* ignoresEnvelope = findCase (proof, "ignores_attack_envelope_without_onset");
    expect (ignoresEnvelope != nullptr, "ignore envelope case present");
    expect (ignoresEnvelope->passed, ignoresEnvelope->message);
    expect (ignoresEnvelope->eventCountLast == 0, "ignore envelope count");

    const auto* missing = findCase (proof, "missing_onset_failure");
    expect (missing != nullptr, "missing onset case present");
    expect (missing->passed, missing->message);
    expect (! missing->detectorOk, "missing onset detector reports failure");
    expectEqual (missing->diagnostic, "missing_input:onset_event", "missing onset diagnostic");

    const auto report = myworld::makeDensityDetectorProofReportJson (proof);
    expect (report.find ("\"operation\": \"pv_density_detector\"") != std::string::npos,
            "report operation json");
    expect (report.find ("\"usesOnsetEvents\": true") != std::string::npos,
            "report onset json");
    expect (report.find ("\"usesAttackEnvelopeForDetector\": false") != std::string::npos,
            "report envelope json");
    expect (report.find ("\"silenceImplemented\": false") != std::string::npos,
            "report silence json");

    std::cout << "analyzer density detector ok\n";
    return 0;
}
