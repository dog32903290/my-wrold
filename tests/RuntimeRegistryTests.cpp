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
    expect (entry.publicInputs.size() == 1, "runtime entry public input count");
    expect (entry.publicOutputs.size() == 5, "runtime entry public output count");
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
    expectEqual (dryRun.snapshot.entries.front().children.front().status, "dry-run-ready", "dry-run first child status");
    expect (dryRun.snapshot.entries.front().children.back().cookIndex == 6, "dry-run last child cook index");
    expectEqual (dryRun.snapshot.entries.front().children.back().childId, "loudness_out", "dry-run last child id");

    const auto dryRunJson = myworld::makeRuntimeDryRunJson (dryRun.snapshot);
    expectContains (dryRunJson, "\"kind\": \"runtimeDryRun\"", "runtime dry-run json");
    expectContains (dryRunJson, "\"nodeType\": \"compound.loudness\"", "runtime dry-run json");
    expectContains (dryRunJson, "\"status\": \"dry-run-ready\"", "runtime dry-run json");
    expectContains (dryRunJson, "\"childId\": \"audio_in\"", "runtime dry-run json");
    expectContains (dryRunJson, "\"nodeType\": \"audio.input\"", "runtime dry-run json");
    expectContains (dryRunJson, "\"childId\": \"loudness_out\"", "runtime dry-run json");

    const std::vector<float> syntheticSamples { 0.0f, 1.0f, -1.0f, 0.0f };
    const auto execution = myworld::executeRuntimeRegistryWithSyntheticAudio (registryResult.registry,
                                                                              syntheticSamples,
                                                                              1.0f);
    expect (execution.ok, execution.error);
    expect (execution.snapshot.entries.size() == 1, "execution entry count");
    expectEqual (execution.snapshot.entries.front().nodeType, "compound.loudness", "execution entry node type");
    expectEqual (execution.snapshot.entries.front().status, "partial-execution", "execution entry status");
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
    expectEqual (rmsChild.status, "computed", "executed child status");
    expect (rmsChild.inputs.size() == 3, "executed child input count");
    expectEqual (rmsChild.inputs.at (1).id, "sourceRms", "rms input source id");
    expectNear (rmsChild.inputs.at (1).value, std::sqrt (0.5), 0.000001, "rms input source value");
    expect (rmsChild.outputs.size() == 2, "executed child output count");
    expectEqual (rmsChild.outputs.at (0).id, "rms", "rms output id");
    expectNear (rmsChild.outputs.at (0).value, std::sqrt (0.5), 0.000001, "rms output value");
    expectEqual (rmsChild.outputs.at (1).id, "peak", "peak output id");
    expectNear (rmsChild.outputs.at (1).value, 1.0, 0.000001, "peak output value");

    const auto& analysisGainChild = execution.snapshot.entries.front().children.at (3);
    expectEqual (analysisGainChild.childId, "analysis_gain", "analysis gain child id");
    expectEqual (analysisGainChild.status, "computed", "analysis gain child status");
    expect (analysisGainChild.inputs.size() == 2, "analysis gain input count");
    expectEqual (analysisGainChild.inputs.at (0).id, "input", "analysis gain input id");
    expectNear (analysisGainChild.inputs.at (0).value, std::sqrt (0.5), 0.000001, "analysis gain input value");
    expectEqual (analysisGainChild.inputs.at (1).id, "gain", "analysis gain gain input id");
    expectNear (analysisGainChild.inputs.at (1).value, 1.0, 0.000001, "analysis gain gain input value");
    expect (analysisGainChild.outputs.size() == 1, "analysis gain output count");
    expectEqual (analysisGainChild.outputs.at (0).id, "out", "analysis gain output id");
    expectNear (analysisGainChild.outputs.at (0).value, std::sqrt (0.5), 0.000001, "analysis gain output value");

    const auto& gatedChild = execution.snapshot.entries.front().children.at (4);
    expectEqual (gatedChild.status, "not-executed", "first parked child status");
    expectContains (gatedChild.reason, "RuntimeOp not implemented", "first parked child reason");

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

    const auto executionJson = myworld::makeRuntimeExecutionJson (execution.snapshot);
    expectContains (executionJson, "\"kind\": \"runtimeExecution\"", "runtime execution json");
    expectContains (executionJson, "\"mode\": \"synthetic-audio\"", "runtime execution json");
    expectContains (executionJson, "\"childId\": \"mono_mix\"", "runtime execution json");
    expectContains (executionJson, "\"inputs\": {", "runtime execution json");
    expectContains (executionJson, "\"childId\": \"rms\"", "runtime execution json");
    expectContains (executionJson, "\"status\": \"computed\"", "runtime execution json");
    expectContains (executionJson, "\"childId\": \"analysis_gain\"", "runtime execution json");
    expectContains (executionJson, "\"out\": 0.707107", "runtime execution json");
    expectContains (executionJson, "\"rms\": 0.707107", "runtime execution json");
    expectContains (executionJson, "\"peak\": 1.000000", "runtime execution json");

    std::cout << "runtime registry ok\n";
    return 0;
}
