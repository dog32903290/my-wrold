#include "AnalyzerResidueDetectorFixture.h"
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

const myworld::ResidueDetectorCaseResult* findCase (const myworld::ResidueDetectorProofResult& result,
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
    const std::string libraryPath = "fixtures/module-libraries/pv-residue-detector.module-library.json";
    const std::string fixturePath = "fixtures/analyzer/residue_detector_cases.json";

    const auto fixture = myworld::loadResidueDetectorFixture (fixturePath);
    expect (fixture.ok, fixture.error);
    expect (fixture.fixture.cases.size() == 6, "residue fixture case count");
    expectNear (fixture.fixture.parameters.floor, 0.02, 0.000001, "residue fixture floor");
    expectNear (fixture.fixture.parameters.armThreshold, 0.6, 0.000001, "residue fixture arm threshold");
    expectNear (fixture.fixture.parameters.decayMs, 200.0, 0.000001, "residue fixture decay");

    const auto runtime = myworld::loadRuntimeRegistryFromModuleLibrary (libraryPath);
    expect (runtime.ok, runtime.error);
    expect (runtime.registry.entries.size() == 1, "pv residue registry entry count");

    const auto* residueEntry = findRuntimeEntry (runtime.registry, "compound.residue");
    expect (residueEntry != nullptr, "runtime registry contains compound.residue");
    expect (residueEntry->children.size() == 7, "residue child count");
    expect (residueEntry->internalEdges.size() == 14, "residue internal edge count");
    expect (residueEntry->publicOutputs.size() == 4, "residue output count");
    expectEqual (residueEntry->publicOutputs.at (0), "residue_state", "residue first output");
    expectEqual (residueEntry->publicOutputs.at (1), "residue_envelope", "residue second output");
    expectEqual (residueEntry->publicOutputs.at (2), "residue_timer_ms", "residue third output");

    const auto coverage = myworld::inspectRuntimeOpCoverage (runtime.registry);
    expect (coverage.ok, coverage.error);
    expect (coverage.snapshot.missingChildCount == 0, "pv residue coverage has no missing RuntimeOps");

    const auto proof = myworld::runResidueDetectorProof (runtime.registry, fixturePath);
    expect (proof.ok, proof.error);
    expectEqual (proof.operation, "pv_residue_detector", "proof operation");
    expectEqual (proof.selectedDetector, "residue", "selected detector");
    expect (proof.usesRawRms, "proof uses raw rms");
    expect (proof.usesSustainEnvelopeForDetector, "proof uses sustain envelope");
    expect (proof.usesSilenceStateForClear, "proof uses silence state for clear");
    expect (! proof.usesOutputSmoothingForDetector, "proof does not use output smoothing");
    expect (proof.caseCount == 6, "proof case count");
    expect (proof.passedCaseCount == 6, "proof passed case count");

    const auto* drop = findCase (proof, "sustained_drop_creates_residue");
    expect (drop != nullptr, "sustained drop case present");
    expect (drop->passed, drop->message);
    expect (drop->residueStateLast, "sustained drop creates residue");
    expectNear (drop->residueTimerMsLast, 100.0, 0.000001, "drop residue timer");
    expectNear (drop->residueEnvelopeLast, 0.5, 0.000001, "drop residue envelope");

    const auto* decay = findCase (proof, "residue_decays_below_threshold");
    expect (decay != nullptr, "decay case present");
    expect (decay->passed, decay->message);
    expect (! decay->residueStateLast, "residue decays below state threshold");
    expectNear (decay->residueTimerMsLast, 400.0, 0.000001, "decay timer");
    expectNear (decay->residueEnvelopeLast, 0.0625, 0.000001, "decay envelope");

    const auto* clear = findCase (proof, "silence_state_clears_residue");
    expect (clear != nullptr, "silence clear case present");
    expect (clear->passed, clear->message);
    expect (! clear->residueStateLast, "silence clears residue state");
    expectNear (clear->residueTimerMsLast, 0.0, 0.000001, "silence clear timer");
    expectNear (clear->residueEnvelopeLast, 0.0, 0.000001, "silence clear envelope");

    const auto* shortDrop = findCase (proof, "short_unsustained_drop_no_residue");
    expect (shortDrop != nullptr, "short unsustained case present");
    expect (shortDrop->passed, shortDrop->message);
    expect (! shortDrop->residueStateLast, "short unsustained drop has no residue");

    const auto* missingRms = findCase (proof, "missing_rms_failure");
    expect (missingRms != nullptr, "missing rms case present");
    expect (missingRms->passed, missingRms->message);
    expect (! missingRms->detectorOk, "missing rms detector reports failure");
    expectEqual (missingRms->diagnostic, "missing_input:rms", "missing rms diagnostic");

    const auto* missingSustain = findCase (proof, "missing_sustain_envelope_failure");
    expect (missingSustain != nullptr, "missing sustain case present");
    expect (missingSustain->passed, missingSustain->message);
    expect (! missingSustain->detectorOk, "missing sustain detector reports failure");
    expectEqual (missingSustain->diagnostic,
                 "missing_input:sustain_envelope",
                 "missing sustain diagnostic");

    const auto report = myworld::makeResidueDetectorProofReportJson (proof);
    expect (report.find ("\"operation\": \"pv_residue_detector\"") != std::string::npos,
            "report operation json");
    expect (report.find ("\"usesRawRms\": true") != std::string::npos,
            "report raw rms json");
    expect (report.find ("\"usesSustainEnvelopeForDetector\": true") != std::string::npos,
            "report sustain json");
    expect (report.find ("\"usesSilenceStateForClear\": true") != std::string::npos,
            "report silence json");
    expect (report.find ("\"usesOutputSmoothingForDetector\": false") != std::string::npos,
            "report smoothing json");

    std::cout << "analyzer residue detector ok\n";
    return 0;
}
