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

    std::cout << "runtime registry ok\n";
    return 0;
}
