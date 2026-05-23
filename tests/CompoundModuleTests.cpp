#include "CompoundModule.h"
#include "CompoundPatch.h"
#include "InteractionContract.h"
#include "StorageContract.h"

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

    const auto module = myworld::loadModulePackageManifest ("fixtures/modules/loudness/module.json");
    expect (module.ok, module.error);
    expectEqual (module.manifest.id, "module.loudness", "module id");
    expectEqual (module.manifest.nodeType, "compound.loudness", "module node type");
    expectEqual (module.manifest.patchPath, "fixtures/compounds/loudness.compound.json", "module patch path");
    expect (module.manifest.publicPorts.size() == 6, "module public port list includes input and outputs");

    const auto moduleSpec = myworld::makeCompoundModuleNodeSpec (module.manifest, loudness);
    expectEqual (moduleSpec.type, "compound.loudness", "module node spec type");
    expectEqual (moduleSpec.category, "compound", "module node spec category");
    expect (moduleSpec.inputs.size() == 1, "module node spec input count");
    expect (moduleSpec.outputs.size() == 5, "module node spec output count");
    expectEqual (moduleSpec.inputs.front().id, "audio.in", "module node input id");
    expectEqual (moduleSpec.outputs.front().id, "out", "module node output id");

    auto limitedManifest = module.manifest;
    limitedManifest.publicPorts = { "confidence", "audio.in" };
    const auto limitedSpec = myworld::makeCompoundModuleNodeSpec (limitedManifest, loudness);
    expect (limitedSpec.inputs.size() == 1, "limited module keeps requested input");
    expect (limitedSpec.outputs.size() == 1, "limited module exposes requested output only");
    expectEqual (limitedSpec.outputs.front().id, "confidence", "limited module output follows manifest");

    const std::vector<myworld::NodeSpec> moduleRegistry { moduleSpec };
    auto moduleSession = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    expect (myworld::createNode (moduleSession, moduleRegistry, moduleSpec.type, "loud_module1", { 220.0, 280.0 }).ok,
            "create compound from module registry");
    expect (findNode (moduleSession.graph, "loud_module1") != nullptr, "module-created compound node exists");
    expect (myworld::enterPatch (moduleSession, "loud_module1").ok, "enter module-created compound");

    const auto loadedRegistry = myworld::loadCompoundModuleNodeSpecs ({ "fixtures/modules/loudness/module.json" });
    expect (loadedRegistry.ok, loadedRegistry.error);
    expect (loadedRegistry.specs.size() == 1, "loaded module registry count");
    expectEqual (loadedRegistry.specs.front().displayName, "Loudness", "loaded module registry display name");

    const auto visibleRegistry = myworld::mergeNodeSpecs (myworld::makeSeedNodeSpecs(), loadedRegistry.specs);
    const auto* visibleLoudness = myworld::findNodeSpec (visibleRegistry, "compound.loudness");
    expect (visibleLoudness != nullptr, "visible registry contains module loudness");
    expectEqual (visibleLoudness->displayName, "Loudness", "module registry overrides seed display name");
    expect (visibleLoudness->inputs.size() == 1, "visible module registry input count");
    expect (visibleLoudness->outputs.size() == 5, "visible module registry output count");

    auto visibleSession = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    const auto visibleCreate = myworld::createNode (visibleSession,
                                                    visibleRegistry,
                                                    "compound.loudness",
                                                    "visible_loud1",
                                                    { 260.0, 300.0 });
    expect (visibleCreate.ok, "visible registry creates module compound");
    expect (visibleSession.commandLog.back() == "create_node", "visible registry create command logged");

    std::cout << "compound module fixture ok\n";
    return 0;
}
