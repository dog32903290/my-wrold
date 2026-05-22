#include "CompoundPatch.h"

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
    const auto loudness = myworld::makeLoudnessCompoundPatchSpec();

    expectEqual (loudness.type, "compound.loudness", "compound type");
    expectEqual (loudness.displayName, "Loudness", "compound display name");
    expect (loudness.collapsedByDefault, "loudness starts collapsed");
    expect (loudness.children.size() == 7, "loudness has seven child patchers");
    expect (loudness.internalEdges.size() == 9, "loudness has nine internal edges");
    expect (loudness.publicInputs.size() == 1, "loudness has one public input");
    expect (loudness.publicOutputs.size() == 5, "loudness exposes five public outputs");

    const std::vector<std::string> expectedCookOrder {
        "audio_in",
        "mono_mix",
        "rms",
        "analysis_gain",
        "pre_gate",
        "output_smoother",
        "loudness_out"
    };
    expect (myworld::makeCompoundCookOrder (loudness) == expectedCookOrder, "cook order follows child pipeline");

    const auto* audioIn = myworld::findCompoundChild (loudness, "audio_in");
    const auto* monoMix = myworld::findCompoundChild (loudness, "mono_mix");
    const auto* rms = myworld::findCompoundChild (loudness, "rms");
    const auto* smoother = myworld::findCompoundChild (loudness, "output_smoother");

    expect (audioIn != nullptr, "audio_in child exists");
    expect (monoMix != nullptr, "mono_mix child exists");
    expect (rms != nullptr, "rms child exists");
    expect (smoother != nullptr, "output_smoother child exists");
    expectEqual (audioIn->nodeType, "audio.input", "audio_in type");
    expectEqual (monoMix->nodeType, "audio.mono_mix", "mono_mix type");
    expectEqual (rms->nodeType, "analyzer.rms", "rms type");
    expectEqual (smoother->nodeType, "signal.smoother", "smoother type");

    expectEqual (loudness.publicInputs[0].id, "audio.in", "public audio input id");
    expectEqual (loudness.publicInputs[0].mapsTo, "audio_in.input", "public input maps to audio adapter");

    expectEqual (myworld::findCompoundPublicOutput (loudness, "out")->mapsTo, "loudness_out.out", "out exposed source");
    expectEqual (myworld::findCompoundPublicOutput (loudness, "rms")->mapsTo, "rms.rms", "rms exposed source");
    expectEqual (myworld::findCompoundPublicOutput (loudness, "peak")->mapsTo, "rms.peak", "peak exposed source");
    expectEqual (myworld::findCompoundPublicOutput (loudness, "gate")->mapsTo, "pre_gate.gate", "gate exposed source");
    expectEqual (myworld::findCompoundPublicOutput (loudness, "confidence")->mapsTo, "pre_gate.confidence", "confidence exposed source");

    expect (myworld::isValidCompoundPatchSpec (loudness), "loudness compound validates");

    const auto json = myworld::makeCompoundPatchJson (loudness);
    expectContains (json, "\"type\": \"compound.loudness\"", "compound json");
    expectContains (json, "\"children\"", "compound json");
    expectContains (json, "\"audio.mono_mix\"", "compound json");
    expectContains (json, "\"publicOutputs\"", "compound json");
    expectContains (json, "\"loudness_out.out\"", "compound json");

    std::cout << "compound patch contract ok\n";
    return 0;
}
