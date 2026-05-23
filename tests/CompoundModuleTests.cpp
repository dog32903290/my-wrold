#include "CompoundPatch.h"
#include "InteractionContract.h"

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

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}

const myworld::GraphNode* findNode (const myworld::GraphContract& graph, const std::string& id)
{
    for (const auto& node : graph.editorGraph.nodes)
        if (node.id == id)
            return &node;

    return nullptr;
}
}

int main()
{
    const auto loaded = myworld::loadCompoundPatchSpec ("fixtures/compounds/loudness.compound.json");
    expect (loaded.ok, loaded.error);

    const auto& loudness = loaded.spec;
    expectEqual (loudness.type, "compound.loudness", "loaded compound type");
    expectEqual (loudness.displayName, "Loudness", "loaded display name");
    expect (loudness.children.size() == 7, "loaded child count");
    expect (loudness.internalEdges.size() == 9, "loaded internal edge count");
    expect (loudness.publicInputs.size() == 1, "loaded public input count");
    expect (loudness.publicOutputs.size() == 5, "loaded public output count");
    expect (myworld::isValidCompoundPatchSpec (loudness), "loaded loudness validates");
    expectEqual (loudness.publicInputs.front().mapsTo, "audio_in.input", "public input maps to inner child");
    expectEqual (myworld::findCompoundPublicOutput (loudness, "out")->mapsTo, "loudness_out.out", "public output maps to inner child");

    const auto json = myworld::makeCompoundPatchJson (loudness);
    expectContains (json, "\"publicInputs\"", "roundtrip json");
    expectContains (json, "\"mapsTo\": \"audio_in.input\"", "roundtrip json");
    expectContains (json, "\"label\": \"Loudness\"", "roundtrip json");

    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    expect (myworld::createNode (session, loudness.type, "loud1", { 180.0, 260.0 }).ok, "create loaded compound node");
    expect (session.selectedNodeIds.size() == 1 && session.selectedNodeIds.front() == "loud1", "created compound selected");
    expect (myworld::enterPatch (session, "loud1").ok, "enter loaded compound");
    expect (session.currentPatchPath.size() == 1 && session.currentPatchPath.front() == "loud1", "loaded compound patch path");
    expect (myworld::exitPatch (session).ok, "exit loaded compound");
    expect (myworld::setCollapsed (session, "loud1", loudness.collapsedByDefault).ok, "apply loaded collapse state");

    const auto encoded = myworld::serializeInteractionState (session);
    const auto restored = myworld::deserializeInteractionState (encoded);
    const auto* restoredLoudness = findNode (restored.graph, "loud1");
    expect (restoredLoudness != nullptr, "loaded compound survives interaction roundtrip");
    expect (restoredLoudness->collapsed == loudness.collapsedByDefault, "loaded collapse state survives roundtrip");

    std::cout << "compound module fixture ok\n";
    return 0;
}
