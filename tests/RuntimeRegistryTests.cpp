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

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}

myworld::RuntimeRegistry makeRegistryWithUnsupportedRuntimeOp (myworld::RuntimeRegistry registry)
{
    auto& entry = registry.entries.front();
    entry.children.push_back ({ "unsupported_probe", "debug.unsupported", "Missing RuntimeOp fixture" });
    entry.cookOrder.push_back ("unsupported_probe");
    entry.childCount = entry.children.size();
    return registry;
}
}

int main()
{
    const auto registryResult = myworld::loadRuntimeRegistryFromModuleLibrary (
        "fixtures/module-libraries/default.module-library.json");
    expect (registryResult.ok, registryResult.error);
    expect (registryResult.registry.entries.size() == 1, "runtime registry entry count");

    const auto& entry = registryResult.registry.entries.front();
    expectEqual (entry.nodeType, "compound.loudness", "runtime entry node type");
    expectEqual (entry.displayName, "Loudness", "runtime entry display name");
    expectEqual (entry.runtimeDomain, "audioAnalysis", "runtime entry domain");
    expectEqual (entry.executionKind, "compound.patch", "runtime entry execution kind");
    expect (entry.childCount == 7, "runtime entry child count");
    expect (entry.internalEdgeCount == 9, "runtime entry internal edge count");
    expect (entry.internalEdges.size() == 9, "runtime entry internal edge metadata count");
    expectEqual (entry.internalEdges.at (2).from, "mono_mix.mono", "runtime internal edge from");
    expectEqual (entry.internalEdges.at (2).to, "rms.input", "runtime internal edge to");
    expectEqual (entry.internalEdges.at (2).dataType, "audio.mono", "runtime internal edge data type");
    expect (entry.publicInputs.size() == 1, "runtime entry public input count");
    expect (entry.publicOutputs.size() == 5, "runtime entry public output count");
    expect (entry.publicOutputMappings.size() == 5, "runtime entry public output mapping count");
    expectEqual (entry.publicOutputMappings.at (0).id, "out", "runtime public output mapping id");
    expectEqual (entry.publicOutputMappings.at (0).mapsTo, "loudness_out.out", "runtime public output mapping target");
    expect (entry.children.size() == 7, "runtime entry child metadata count");
    expectEqual (entry.children.front().id, "audio_in", "runtime entry first child id");
    expectEqual (entry.children.front().nodeType, "audio.input", "runtime entry first child type");
    expectEqual (entry.children.back().id, "loudness_out", "runtime entry last child id");
    expectEqual (entry.children.back().nodeType, "analyzer.loudness_out", "runtime entry last child type");

    const std::vector<std::string> expectedCookOrder {
        "audio_in",
        "mono_mix",
        "rms",
        "analysis_gain",
        "pre_gate",
        "output_smoother",
        "loudness_out"
    };
    expect (entry.cookOrder == expectedCookOrder, "runtime entry cook order");

    const auto json = myworld::makeRuntimeRegistryJson (registryResult.registry);
    expectContains (json, "\"kind\": \"runtimeRegistry\"", "runtime registry json");
    expectContains (json, "\"nodeType\": \"compound.loudness\"", "runtime registry json");
    expectContains (json, "\"executionKind\": \"compound.patch\"", "runtime registry json");
    expectContains (json, "\"internalEdges\"", "runtime registry json");
    expectContains (json, "\"from\": \"mono_mix.mono\"", "runtime registry json");
    expectContains (json, "\"to\": \"rms.input\"", "runtime registry json");
    expectContains (json, "\"publicOutputMappings\"", "runtime registry json");
    expectContains (json, "\"mapsTo\": \"loudness_out.out\"", "runtime registry json");
    expectContains (json, "\"cookOrder\"", "runtime registry json");
    expectContains (json, "\"loudness_out\"", "runtime registry json");

    const auto dryRun = myworld::dryRunRuntimeRegistry (registryResult.registry);
    expect (dryRun.ok, dryRun.error);
    expect (dryRun.snapshot.entries.size() == 1, "dry-run entry count");
    expectEqual (dryRun.snapshot.entries.front().nodeType, "compound.loudness", "dry-run entry node type");
    expectEqual (dryRun.snapshot.entries.front().status, "dry-run-ready", "dry-run entry status");
    expect (dryRun.snapshot.entries.front().children.size() == 7, "dry-run child count");
    expectEqual (dryRun.snapshot.entries.front().children.front().childId, "audio_in", "dry-run first child id");
    expectEqual (dryRun.snapshot.entries.front().children.front().nodeType, "audio.input", "dry-run first child type");
    expectEqual (dryRun.snapshot.entries.front().children.front().runtimeOp,
                 "synthetic.audio.input",
                 "dry-run first child runtime op");
    expectEqual (dryRun.snapshot.entries.front().children.front().status, "dry-run-ready", "dry-run first child status");
    expect (dryRun.snapshot.entries.front().children.back().cookIndex == 6, "dry-run last child cook index");
    expectEqual (dryRun.snapshot.entries.front().children.back().childId, "loudness_out", "dry-run last child id");

    const auto dryRunJson = myworld::makeRuntimeDryRunJson (dryRun.snapshot);
    expectContains (dryRunJson, "\"kind\": \"runtimeDryRun\"", "runtime dry-run json");
    expectContains (dryRunJson, "\"nodeType\": \"compound.loudness\"", "runtime dry-run json");
    expectContains (dryRunJson, "\"status\": \"dry-run-ready\"", "runtime dry-run json");
    expectContains (dryRunJson, "\"childId\": \"audio_in\"", "runtime dry-run json");
    expectContains (dryRunJson, "\"nodeType\": \"audio.input\"", "runtime dry-run json");
    expectContains (dryRunJson, "\"runtimeOp\": \"synthetic.analyzer.rms\"", "runtime dry-run json");
    expectContains (dryRunJson, "\"childId\": \"loudness_out\"", "runtime dry-run json");

    const auto unsupportedRegistry = makeRegistryWithUnsupportedRuntimeOp (registryResult.registry);
    const auto unsupportedDryRun = myworld::dryRunRuntimeRegistry (unsupportedRegistry);
    expect (! unsupportedDryRun.ok, "unsupported dry-run should fail runtime op coverage");
    expectContains (unsupportedDryRun.error, "missing RuntimeOp", "unsupported dry-run error");
    expect (unsupportedDryRun.snapshot.entries.size() == 1, "unsupported dry-run entry count");
    expectEqual (unsupportedDryRun.snapshot.entries.front().status,
                 "missing-runtime-op",
                 "unsupported dry-run entry status");
    expectEqual (unsupportedDryRun.snapshot.entries.front().children.back().childId,
                 "unsupported_probe",
                 "unsupported dry-run child id");
    expectEqual (unsupportedDryRun.snapshot.entries.front().children.back().nodeType,
                 "debug.unsupported",
                 "unsupported dry-run child type");
    expectEqual (unsupportedDryRun.snapshot.entries.front().children.back().runtimeOp,
                 "",
                 "unsupported dry-run runtime op");
    expectEqual (unsupportedDryRun.snapshot.entries.front().children.back().status,
                 "missing-runtime-op",
                 "unsupported dry-run child status");

    const auto unsupportedDryRunJson = myworld::makeRuntimeDryRunJson (unsupportedDryRun.snapshot);
    expectContains (unsupportedDryRunJson, "\"status\": \"missing-runtime-op\"", "unsupported dry-run json");
    expectContains (unsupportedDryRunJson, "\"nodeType\": \"debug.unsupported\"", "unsupported dry-run json");

    const std::vector<float> syntheticSamples { 0.0f, 1.0f, -1.0f, 0.0f };
    const auto unsupportedExecution = myworld::executeRuntimeRegistryWithSyntheticAudio (unsupportedRegistry,
                                                                                        syntheticSamples,
                                                                                        1.0f);
    expect (! unsupportedExecution.ok, "unsupported execution should fail runtime op coverage");
    expectContains (unsupportedExecution.error, "missing RuntimeOp", "unsupported execution error");
    expectEqual (unsupportedExecution.snapshot.entries.front().status,
                 "missing-runtime-op",
                 "unsupported execution entry status");
    expectEqual (unsupportedExecution.snapshot.entries.front().children.back().childId,
                 "unsupported_probe",
                 "unsupported execution child id");
    expectEqual (unsupportedExecution.snapshot.entries.front().children.back().runtimeOp,
                 "",
                 "unsupported execution runtime op");
    expectEqual (unsupportedExecution.snapshot.entries.front().children.back().status,
                 "missing-runtime-op",
                 "unsupported execution child status");
    expect (unsupportedExecution.snapshot.entries.front().publicOutputs.empty(),
            "unsupported execution should not publish public outputs");

    const auto unsupportedExecutionJson = myworld::makeRuntimeExecutionJson (unsupportedExecution.snapshot);
    expectContains (unsupportedExecutionJson, "\"status\": \"missing-runtime-op\"", "unsupported execution json");
    expectContains (unsupportedExecutionJson, "\"nodeType\": \"debug.unsupported\"", "unsupported execution json");

    const auto savedNegativeRegistry = myworld::loadRuntimeRegistryFromModuleLibrary (
        "fixtures/module-libraries/missing-runtimeop.module-library.json");
    expect (savedNegativeRegistry.ok, savedNegativeRegistry.error);
    expect (savedNegativeRegistry.registry.entries.size() == 1, "saved negative registry entry count");
    expectEqual (savedNegativeRegistry.registry.entries.front().nodeType,
                 "compound.loudness.missing-runtimeop",
                 "saved negative registry node type");
    expect (savedNegativeRegistry.registry.entries.front().children.size() == 8,
            "saved negative registry child metadata count");
    expectEqual (savedNegativeRegistry.registry.entries.front().children.back().id,
                 "unsupported_probe",
                 "saved negative registry unsupported child id");
    expectEqual (savedNegativeRegistry.registry.entries.front().children.back().nodeType,
                 "debug.unsupported",
                 "saved negative registry unsupported child type");
    expectEqual (savedNegativeRegistry.registry.entries.front().cookOrder.back(),
                 "unsupported_probe",
                 "saved negative registry unsupported cook order tail");

    const auto savedNegativeRegistryJson = myworld::makeRuntimeRegistryJson (savedNegativeRegistry.registry);
    expectContains (savedNegativeRegistryJson,
                    "\"nodeType\": \"compound.loudness.missing-runtimeop\"",
                    "saved negative registry json");
    expectContains (savedNegativeRegistryJson,
                    "\"nodeType\": \"debug.unsupported\"",
                    "saved negative registry json");

    const auto savedNegativeDryRun = myworld::dryRunRuntimeRegistry (savedNegativeRegistry.registry);
    expect (! savedNegativeDryRun.ok, "saved negative dry-run should fail runtime op coverage");
    expectContains (savedNegativeDryRun.error, "missing RuntimeOp", "saved negative dry-run error");
    expectEqual (savedNegativeDryRun.snapshot.entries.front().status,
                 "missing-runtime-op",
                 "saved negative dry-run entry status");
    expectEqual (savedNegativeDryRun.snapshot.entries.front().children.back().childId,
                 "unsupported_probe",
                 "saved negative dry-run child id");
    expectEqual (savedNegativeDryRun.snapshot.entries.front().children.back().nodeType,
                 "debug.unsupported",
                 "saved negative dry-run child type");
    expectEqual (savedNegativeDryRun.snapshot.entries.front().children.back().status,
                 "missing-runtime-op",
                 "saved negative dry-run child status");

    const auto savedNegativeDryRunJson = myworld::makeRuntimeDryRunJson (savedNegativeDryRun.snapshot);
    expectContains (savedNegativeDryRunJson, "\"status\": \"missing-runtime-op\"", "saved negative dry-run json");
    expectContains (savedNegativeDryRunJson, "\"nodeType\": \"debug.unsupported\"", "saved negative dry-run json");

    const auto savedNegativeExecution = myworld::executeRuntimeRegistryWithSyntheticAudio (
        savedNegativeRegistry.registry,
        syntheticSamples,
        1.0f);
    expect (! savedNegativeExecution.ok, "saved negative execution should fail runtime op coverage");
    expectContains (savedNegativeExecution.error, "missing RuntimeOp", "saved negative execution error");
    expectEqual (savedNegativeExecution.snapshot.entries.front().status,
                 "missing-runtime-op",
                 "saved negative execution entry status");
    expectEqual (savedNegativeExecution.snapshot.entries.front().children.back().childId,
                 "unsupported_probe",
                 "saved negative execution child id");
    expectEqual (savedNegativeExecution.snapshot.entries.front().children.back().status,
                 "missing-runtime-op",
                 "saved negative execution child status");
    expect (savedNegativeExecution.snapshot.entries.front().publicOutputs.empty(),
            "saved negative execution should not publish public outputs");

    const auto savedNegativeExecutionJson = myworld::makeRuntimeExecutionJson (savedNegativeExecution.snapshot);
    expectContains (savedNegativeExecutionJson,
                    "\"status\": \"missing-runtime-op\"",
                    "saved negative execution json");
    expectContains (savedNegativeExecutionJson,
                    "\"nodeType\": \"debug.unsupported\"",
                    "saved negative execution json");

    const auto execution = myworld::executeRuntimeRegistryWithSyntheticAudio (registryResult.registry,
                                                                              syntheticSamples,
                                                                              1.0f);
    expect (execution.ok, execution.error);
    expect (execution.snapshot.entries.size() == 1, "execution entry count");
    expectEqual (execution.snapshot.entries.front().nodeType, "compound.loudness", "execution entry node type");
    expectEqual (execution.snapshot.entries.front().status, "computed", "execution entry status");
    expect (execution.snapshot.entries.front().children.size() == 7, "execution child count");

    const auto& rmsChild = execution.snapshot.entries.front().children.at (2);
    expectEqual (execution.snapshot.entries.front().children.at (0).status,
                 "computed",
                 "audio input child status");
    expectEqual (execution.snapshot.entries.front().children.at (1).status,
                 "computed",
                 "mono mix child status");
    expectEqual (rmsChild.childId, "rms", "executed child id");
    expectEqual (rmsChild.nodeType, "analyzer.rms", "executed child type");
    expectEqual (rmsChild.runtimeOp, "synthetic.analyzer.rms", "executed child runtime op");
    expectEqual (rmsChild.status, "computed", "executed child status");
    expect (rmsChild.inputs.size() == 1, "executed child input count");
    expectEqual (rmsChild.inputs.front().id, "input", "rms input id");
    expectEqual (rmsChild.inputs.front().source, "mono_mix.mono", "rms input source port");
    expectNear (rmsChild.inputs.front().value, std::sqrt (0.5), 0.000001, "rms input source value");
    expect (rmsChild.outputs.size() == 2, "executed child output count");
    expectEqual (rmsChild.outputs.at (0).id, "rms", "rms output id");
    expectNear (rmsChild.outputs.at (0).value, std::sqrt (0.5), 0.000001, "rms output value");
    expectEqual (rmsChild.outputs.at (1).id, "peak", "peak output id");
    expectNear (rmsChild.outputs.at (1).value, 1.0, 0.000001, "peak output value");

    const auto& analysisGainChild = execution.snapshot.entries.front().children.at (3);
    expectEqual (analysisGainChild.childId, "analysis_gain", "analysis gain child id");
    expectEqual (analysisGainChild.runtimeOp, "synthetic.analyzer.analysis_gain", "analysis gain runtime op");
    expectEqual (analysisGainChild.status, "computed", "analysis gain child status");
    expect (analysisGainChild.inputs.size() == 2, "analysis gain input count");
    expectEqual (analysisGainChild.inputs.at (0).id, "input", "analysis gain input id");
    expectEqual (analysisGainChild.inputs.at (0).source, "rms.rms", "analysis gain input source");
    expectNear (analysisGainChild.inputs.at (0).value, std::sqrt (0.5), 0.000001, "analysis gain input value");
    expectEqual (analysisGainChild.inputs.at (1).id, "gain", "analysis gain gain input id");
    expectNear (analysisGainChild.inputs.at (1).value, 1.0, 0.000001, "analysis gain gain input value");
    expect (analysisGainChild.outputs.size() == 1, "analysis gain output count");
    expectEqual (analysisGainChild.outputs.at (0).id, "out", "analysis gain output id");
    expectNear (analysisGainChild.outputs.at (0).value, std::sqrt (0.5), 0.000001, "analysis gain output value");

    const auto& gatedChild = execution.snapshot.entries.front().children.at (4);
    expectEqual (gatedChild.childId, "pre_gate", "pre gate child id");
    expectEqual (gatedChild.runtimeOp, "synthetic.analyzer.pre_gate", "pre gate runtime op");
    expectEqual (gatedChild.status, "computed", "pre gate child status");
    expectEqual (gatedChild.inputs.at (0).id, "input", "pre gate input id");
    expectEqual (gatedChild.inputs.at (0).source, "analysis_gain.out", "pre gate input source");
    expectNear (gatedChild.inputs.at (0).value, std::sqrt (0.5), 0.000001, "pre gate input value");
    expectEqual (gatedChild.outputs.at (0).id, "out", "pre gate output id");
    expectNear (gatedChild.outputs.at (0).value, std::sqrt (0.5), 0.000001, "pre gate output value");
    expectEqual (gatedChild.outputs.at (1).id, "gate", "pre gate gate id");
    expectNear (gatedChild.outputs.at (1).value, 1.0, 0.000001, "pre gate gate value");
    expectEqual (gatedChild.outputs.at (2).id, "confidence", "pre gate confidence id");
    expectNear (gatedChild.outputs.at (2).value, 1.0, 0.000001, "pre gate confidence value");

    const auto& smootherChild = execution.snapshot.entries.front().children.at (5);
    expectEqual (smootherChild.childId, "output_smoother", "smoother child id");
    expectEqual (smootherChild.runtimeOp, "synthetic.signal.smoother", "smoother runtime op");
    expectEqual (smootherChild.status, "computed", "smoother child status");
    expectEqual (smootherChild.inputs.at (0).source, "pre_gate.out", "smoother input source");
    expectEqual (smootherChild.outputs.at (0).id, "out", "smoother output id");
    expectNear (smootherChild.outputs.at (0).value, std::sqrt (0.5), 0.000001, "smoother output value");

    const auto& loudnessOutChild = execution.snapshot.entries.front().children.at (6);
    expectEqual (loudnessOutChild.childId, "loudness_out", "loudness out child id");
    expectEqual (loudnessOutChild.runtimeOp, "synthetic.analyzer.loudness_out", "loudness out runtime op");
    expectEqual (loudnessOutChild.status, "computed", "loudness out child status");
    expectEqual (loudnessOutChild.inputs.at (0).source, "output_smoother.out", "loudness out input source");
    expectEqual (loudnessOutChild.inputs.at (2).source, "rms.peak", "loudness out peak source");
    expectEqual (loudnessOutChild.inputs.at (4).source, "pre_gate.confidence", "loudness out confidence source");
    expect (loudnessOutChild.outputs.size() == 5, "loudness out output count");
    expectEqual (loudnessOutChild.outputs.at (0).id, "out", "loudness out output id");
    expectNear (loudnessOutChild.outputs.at (0).value, std::sqrt (0.5), 0.000001, "loudness out output value");
    expectEqual (loudnessOutChild.outputs.at (1).id, "rms", "loudness out rms id");
    expectNear (loudnessOutChild.outputs.at (1).value, std::sqrt (0.5), 0.000001, "loudness out rms value");
    expectEqual (loudnessOutChild.outputs.at (2).id, "peak", "loudness out peak id");
    expectNear (loudnessOutChild.outputs.at (2).value, 1.0, 0.000001, "loudness out peak value");
    expectEqual (loudnessOutChild.outputs.at (3).id, "gate", "loudness out gate id");
    expectNear (loudnessOutChild.outputs.at (3).value, 1.0, 0.000001, "loudness out gate value");
    expectEqual (loudnessOutChild.outputs.at (4).id, "confidence", "loudness out confidence id");
    expectNear (loudnessOutChild.outputs.at (4).value, 1.0, 0.000001, "loudness out confidence value");

    expect (execution.snapshot.entries.front().publicOutputs.size() == 5, "entry public output count");
    expectEqual (execution.snapshot.entries.front().publicOutputs.at (0).id, "out", "entry public out id");
    expectEqual (execution.snapshot.entries.front().publicOutputs.at (0).source,
                 "loudness_out.out",
                 "entry public out source");
    expectEqual (execution.snapshot.entries.front().publicOutputs.at (3).source,
                 "pre_gate.gate",
                 "entry public gate source");
    expectNear (execution.snapshot.entries.front().publicOutputs.at (0).value,
                std::sqrt (0.5),
                0.000001,
                "entry public out value");

    myworld::RuntimeSyntheticAudioInput stereoExecutionInput;
    stereoExecutionInput.channels = {
        { 0.0f, 1.0f, -1.0f, 0.0f },
        { 0.0f, 0.5f, -0.5f, 0.0f }
    };
    stereoExecutionInput.analysisGain = 1.5f;

    const auto stereoExecution = myworld::executeRuntimeRegistryWithSyntheticAudio (registryResult.registry,
                                                                                    stereoExecutionInput);
    expect (stereoExecution.ok, stereoExecution.error);
    const auto& stereoMonoMix = stereoExecution.snapshot.entries.front().children.at (1);
    expectEqual (stereoMonoMix.status, "computed", "stereo mono mix status");
    expectEqual (stereoMonoMix.outputs.at (1).id, "rms", "stereo mono mix rms id");
    expectNear (stereoMonoMix.outputs.at (1).value, std::sqrt (0.28125), 0.000001, "stereo mono mix rms value");
    expectEqual (stereoMonoMix.outputs.at (2).id, "peak", "stereo mono mix peak id");
    expectNear (stereoMonoMix.outputs.at (2).value, 0.75, 0.000001, "stereo mono mix peak value");

    const auto& stereoAnalysisGain = stereoExecution.snapshot.entries.front().children.at (3);
    expectEqual (stereoAnalysisGain.status, "computed", "stereo analysis gain status");
    expectNear (stereoAnalysisGain.inputs.at (0).value,
                stereoExecution.snapshot.entries.front().children.at (2).outputs.at (0).value,
                0.000001,
                "stereo analysis gain receives rms output");
    expectNear (stereoAnalysisGain.outputs.at (0).value,
                std::sqrt (0.28125) * 1.5,
                0.000001,
                "stereo analysis gain output value");
    expectNear (stereoExecution.snapshot.entries.front().publicOutputs.at (0).value,
                std::sqrt (0.28125) * 1.5,
                0.000001,
                "stereo public output value");

    myworld::RuntimeSyntheticAudioInput silentExecutionInput;
    silentExecutionInput.channels = {
        { 0.0f, 0.0f, 0.0f, 0.0f }
    };
    silentExecutionInput.analysisGain = 2.0f;
    const auto silentExecution = myworld::executeRuntimeRegistryWithSyntheticAudio (registryResult.registry,
                                                                                    silentExecutionInput);
    expect (silentExecution.ok, silentExecution.error);
    expectEqual (silentExecution.snapshot.entries.front().status, "computed", "silent entry status");
    expectNear (silentExecution.snapshot.entries.front().publicOutputs.at (0).value,
                0.0,
                0.000001,
                "silent public out value");
    expectNear (silentExecution.snapshot.entries.front().publicOutputs.at (3).value,
                0.0,
                0.000001,
                "silent public gate value");
    expectNear (silentExecution.snapshot.entries.front().publicOutputs.at (4).value,
                0.0,
                0.000001,
                "silent public confidence value");

    const auto executionJson = myworld::makeRuntimeExecutionJson (execution.snapshot);
    expectContains (executionJson, "\"kind\": \"runtimeExecution\"", "runtime execution json");
    expectContains (executionJson, "\"mode\": \"synthetic-audio\"", "runtime execution json");
    expectContains (executionJson, "\"childId\": \"mono_mix\"", "runtime execution json");
    expectContains (executionJson, "\"inputs\": {", "runtime execution json");
    expectContains (executionJson, "\"inputSources\": {", "runtime execution json");
    expectContains (executionJson, "\"publicOutputSources\": {", "runtime execution json");
    expectContains (executionJson, "\"input\": \"rms.rms\"", "runtime execution json");
    expectContains (executionJson, "\"out\": \"loudness_out.out\"", "runtime execution json");
    expectContains (executionJson, "\"childId\": \"rms\"", "runtime execution json");
    expectContains (executionJson, "\"runtimeOp\": \"synthetic.audio.input\"", "runtime execution json");
    expectContains (executionJson, "\"runtimeOp\": \"synthetic.analyzer.rms\"", "runtime execution json");
    expectContains (executionJson, "\"runtimeOp\": \"synthetic.analyzer.loudness_out\"", "runtime execution json");
    expectContains (executionJson, "\"status\": \"computed\"", "runtime execution json");
    expectContains (executionJson, "\"childId\": \"analysis_gain\"", "runtime execution json");
    expectContains (executionJson, "\"childId\": \"pre_gate\"", "runtime execution json");
    expectContains (executionJson, "\"childId\": \"output_smoother\"", "runtime execution json");
    expectContains (executionJson, "\"childId\": \"loudness_out\"", "runtime execution json");
    expectContains (executionJson, "\"publicOutputs\": {", "runtime execution json");
    expectContains (executionJson, "\"out\": 0.707107", "runtime execution json");
    expectContains (executionJson, "\"gate\": 1.000000", "runtime execution json");
    expectContains (executionJson, "\"confidence\": 1.000000", "runtime execution json");
    expectContains (executionJson, "\"rms\": 0.707107", "runtime execution json");
    expectContains (executionJson, "\"peak\": 1.000000", "runtime execution json");

    std::cout << "runtime registry ok\n";
    return 0;
}
