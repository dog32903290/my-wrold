#include "AnalyzerAggregatePressureFixture.h"
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

const myworld::AggregatePressureCaseResult* findCase (const myworld::AggregatePressureProofResult& result,
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
    const std::string libraryPath = "fixtures/module-libraries/pv-aggregate-pressure.module-library.json";
    const std::string fixturePath = "fixtures/analyzer/aggregate_pressure_cases.json";

    const auto fixture = myworld::loadAggregatePressureFixture (fixturePath);
    expect (fixture.ok, fixture.error);
    expect (fixture.fixture.cases.size() == 7, "aggregate fixture case count");
    expectNear (fixture.fixture.parameters.energyScale, 4.0, 0.000001, "aggregate fixture energy scale");
    expectNear (fixture.fixture.parameters.attackWeight, 0.25, 0.000001, "aggregate fixture attack weight");

    const auto runtime = myworld::loadRuntimeRegistryFromModuleLibrary (libraryPath);
    expect (runtime.ok, runtime.error);
    expect (runtime.registry.entries.size() == 1, "pv aggregate registry entry count");

    const auto* aggregateEntry = findRuntimeEntry (runtime.registry, "compound.aggregate-pressure");
    expect (aggregateEntry != nullptr, "runtime registry contains compound.aggregate-pressure");
    expect (aggregateEntry->children.size() == 10, "aggregate child count");
    expect (aggregateEntry->internalEdges.size() == 28, "aggregate internal edge count");
    expect (aggregateEntry->publicOutputs.size() == 7, "aggregate output count");
    expectEqual (aggregateEntry->publicOutputs.at (0), "pressure_value", "aggregate first output");
    expectEqual (aggregateEntry->publicOutputs.at (1), "energy_component", "aggregate second output");
    expectEqual (aggregateEntry->publicOutputs.at (6), "confidence", "aggregate seventh output");

    const auto coverage = myworld::inspectRuntimeOpCoverage (runtime.registry);
    expect (coverage.ok, coverage.error);
    expect (coverage.snapshot.missingChildCount == 0, "pv aggregate coverage has no missing RuntimeOps");

    const auto proof = myworld::runAggregatePressureProof (runtime.registry, fixturePath);
    expect (proof.ok, proof.error);
    expectEqual (proof.operation, "pv_aggregate_pressure", "proof operation");
    expectEqual (proof.selectedAggregate, "aggregate_pressure", "selected aggregate");
    expect (proof.usesRawRms, "proof uses raw rms");
    expect (proof.usesDetectorStates, "proof uses detector states");
    expect (! proof.usesOutputSmoothingForAggregate, "proof does not use output smoothing");
    expect (proof.caseCount == 7, "proof case count");
    expect (proof.passedCaseCount == 7, "proof passed case count");

    const auto* high = findCase (proof, "high_pressure_all_components");
    expect (high != nullptr, "high pressure case present");
    expect (high->passed, high->message);
    expectNear (high->pressureValue, 0.705, 0.000001, "high pressure value");
    expectNear (high->energyComponent, 0.20, 0.000001, "high energy component");
    expectNear (high->attackComponent, 0.225, 0.000001, "high attack component");

    const auto* attack = findCase (proof, "attack_spike_pressure");
    expect (attack != nullptr, "attack pressure case present");
    expect (attack->passed, attack->message);
    expectNear (attack->pressureValue, 0.35, 0.000001, "attack pressure value");

    const auto* residue = findCase (proof, "residue_tail_pressure");
    expect (residue != nullptr, "residue pressure case present");
    expect (residue->passed, residue->message);
    expectNear (residue->pressureValue, 0.11, 0.000001, "residue pressure value");

    const auto* silence = findCase (proof, "silence_clears_pressure");
    expect (silence != nullptr, "silence pressure case present");
    expect (silence->passed, silence->message);
    expectNear (silence->pressureValue, 0.0, 0.000001, "silence clears pressure");
    expectNear (silence->attackComponent, 0.0, 0.000001, "silence clears components");

    const auto* clamp = findCase (proof, "overrange_inputs_clamp");
    expect (clamp != nullptr, "clamp case present");
    expect (clamp->passed, clamp->message);
    expectNear (clamp->pressureValue, 0.95, 0.000001, "clamp pressure value");

    const auto* missingRms = findCase (proof, "missing_rms_failure");
    expect (missingRms != nullptr, "missing rms case present");
    expect (missingRms->passed, missingRms->message);
    expect (! missingRms->aggregateOk, "missing rms aggregate reports failure");
    expectEqual (missingRms->diagnostic, "missing_input:rms", "missing rms diagnostic");

    const auto* missingAttack = findCase (proof, "missing_attack_value_failure");
    expect (missingAttack != nullptr, "missing attack case present");
    expect (missingAttack->passed, missingAttack->message);
    expect (! missingAttack->aggregateOk, "missing attack aggregate reports failure");
    expectEqual (missingAttack->diagnostic, "missing_input:attack_value", "missing attack diagnostic");

    const auto report = myworld::makeAggregatePressureProofReportJson (proof);
    expect (report.find ("\"operation\": \"pv_aggregate_pressure\"") != std::string::npos,
            "report operation json");
    expect (report.find ("\"usesRawRms\": true") != std::string::npos,
            "report raw rms json");
    expect (report.find ("\"usesDetectorStates\": true") != std::string::npos,
            "report detector state json");
    expect (report.find ("\"usesOutputSmoothingForAggregate\": false") != std::string::npos,
            "report smoothing json");

    std::cout << "analyzer aggregate pressure ok\n";
    return 0;
}
