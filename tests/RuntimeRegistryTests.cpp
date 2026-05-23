#include "RuntimeRegistry.h"

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

    std::cout << "runtime registry ok\n";
    return 0;
}
