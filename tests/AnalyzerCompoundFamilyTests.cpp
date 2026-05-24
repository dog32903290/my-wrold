#include "CompoundModule.h"
#include "InteractionContract.h"
#include "RuntimeRegistry.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

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

const myworld::RuntimeEntryExecutionStatus* findExecutionEntry (const myworld::RuntimeExecutionSnapshot& snapshot,
                                                                const std::string& nodeType)
{
    for (const auto& entry : snapshot.entries)
        if (entry.nodeType == nodeType)
            return &entry;

    return nullptr;
}

const myworld::RuntimeOutputValue* findOutput (const std::vector<myworld::RuntimeOutputValue>& outputs,
                                               const std::string& id)
{
    for (const auto& output : outputs)
        if (output.id == id)
            return &output;

    return nullptr;
}
}

int main()
{
    const std::string familyLibrary = "fixtures/module-libraries/analyzer-family.module-library.json";

    const auto loadedSpecs = myworld::loadCompoundModuleNodeSpecsFromLibrary (familyLibrary);
    expect (loadedSpecs.ok, loadedSpecs.error);
    expect (loadedSpecs.specs.size() == 2, "analyzer family visible registry count");
    expect (myworld::findNodeSpec (loadedSpecs.specs, "compound.loudness") != nullptr,
            "analyzer family keeps loudness");

    const auto* rawEnergySpec = myworld::findNodeSpec (loadedSpecs.specs, "compound.raw-energy");
    expect (rawEnergySpec != nullptr, "analyzer family contains raw-energy");
    expectEqual (rawEnergySpec->displayName, "Raw Energy", "raw-energy display name");
    expectEqual (rawEnergySpec->runtimeDomain, "audioAnalysis", "raw-energy runtime domain");
    expect (rawEnergySpec->inputs.size() == 1, "raw-energy input count");
    expect (rawEnergySpec->outputs.size() == 3, "raw-energy output count");
    expectEqual (rawEnergySpec->outputs.at (0).id, "rms", "raw-energy first output");
    expectEqual (rawEnergySpec->outputs.at (1).id, "peak", "raw-energy second output");
    expectEqual (rawEnergySpec->outputs.at (2).id, "sampleCount", "raw-energy third output");

    const auto runtime = myworld::loadRuntimeRegistryFromModuleLibrary (familyLibrary);
    expect (runtime.ok, runtime.error);
    expect (runtime.registry.entries.size() == 2, "analyzer family runtime entry count");

    const auto* rawEnergyEntry = findRuntimeEntry (runtime.registry, "compound.raw-energy");
    expect (rawEnergyEntry != nullptr, "runtime registry contains raw-energy");
    expectEqual (rawEnergyEntry->displayName, "Raw Energy", "raw-energy runtime display name");
    expectEqual (rawEnergyEntry->runtimeDomain, "audioAnalysis", "raw-energy runtime domain");
    expect (rawEnergyEntry->childCount == 4, "raw-energy child count");
    expect (rawEnergyEntry->internalEdgeCount == 6, "raw-energy internal edge count");
    expect (rawEnergyEntry->publicInputs.size() == 1, "raw-energy public input count");
    expect (rawEnergyEntry->publicOutputs.size() == 3, "raw-energy public output count");

    const std::vector<std::string> expectedCookOrder {
        "audio_in",
        "mono_mix",
        "rms",
        "raw_energy_out"
    };
    expect (rawEnergyEntry->cookOrder == expectedCookOrder, "raw-energy cook order");

    const auto coverage = myworld::inspectRuntimeOpCoverage (runtime.registry);
    expect (coverage.ok, coverage.error);
    expect (coverage.snapshot.missingChildCount == 0, "analyzer family coverage has no missing RuntimeOps");

    const auto diagnostics = myworld::makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    const auto* rawEnergyDiagnostic = findDiagnostic (diagnostics, "compound.raw-energy");
    expect (rawEnergyDiagnostic != nullptr, "raw-energy runtime diagnostic exists");
    expectEqual (rawEnergyDiagnostic->status, "runtime-op-ready", "raw-energy runtime diagnostic status");
    expectEqual (rawEnergyDiagnostic->creationStatus, "create-enabled", "raw-energy creation status");
    expect (myworld::runtimeOpDiagnosticAllowsCreation (*rawEnergyDiagnostic),
            "raw-energy diagnostic allows creation");

    myworld::RuntimeSyntheticAudioInput input;
    input.channels = {
        { 0.0f, 1.0f, -1.0f, 0.0f },
        { 0.0f, 0.5f, -0.5f, 0.0f }
    };
    input.analysisGain = 1.5f;

    const auto execution = myworld::executeRuntimeRegistryWithSyntheticAudio (runtime.registry, input);
    expect (execution.ok, execution.error);
    const auto* rawEnergyExecution = findExecutionEntry (execution.snapshot, "compound.raw-energy");
    expect (rawEnergyExecution != nullptr, "raw-energy execution entry exists");
    expectEqual (rawEnergyExecution->status, "computed", "raw-energy execution status");
    expect (rawEnergyExecution->publicOutputs.size() == 3, "raw-energy public output count after execution");

    const auto* rms = findOutput (rawEnergyExecution->publicOutputs, "rms");
    const auto* peak = findOutput (rawEnergyExecution->publicOutputs, "peak");
    const auto* sampleCount = findOutput (rawEnergyExecution->publicOutputs, "sampleCount");
    expect (rms != nullptr, "raw-energy rms output exists");
    expect (peak != nullptr, "raw-energy peak output exists");
    expect (sampleCount != nullptr, "raw-energy sampleCount output exists");
    expectNear (rms->value, std::sqrt (0.28125), 0.000001, "raw-energy rms value");
    expectNear (peak->value, 0.75, 0.000001, "raw-energy peak value");
    expectNear (sampleCount->value, 4.0, 0.000001, "raw-energy sampleCount value");
    expectEqual (rms->source, "raw_energy_out.rms", "raw-energy rms source");
    expectEqual (peak->source, "raw_energy_out.peak", "raw-energy peak source");
    expectEqual (sampleCount->source, "raw_energy_out.sampleCount", "raw-energy sampleCount source");

    const auto visibleRegistry = myworld::mergeNodeSpecs (myworld::makeSeedNodeSpecs(), loadedSpecs.specs);
    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    const auto createResult = myworld::createNode (session,
                                                   visibleRegistry,
                                                   "compound.raw-energy",
                                                   "raw_energy1",
                                                   { 260.0, 300.0 });
    expect (createResult.ok, createResult.message);
    expect (session.commandLog.back() == "create_node", "raw-energy create command logged");

    std::cout << "analyzer compound family ok\n";
    return 0;
}
