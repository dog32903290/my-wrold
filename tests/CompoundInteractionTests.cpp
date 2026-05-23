#include "CanvasHands.h"
#include "CompoundModule.h"
#include "CompoundPatch.h"
#include "InteractionContract.h"

#include <algorithm>
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

bool contains (const std::vector<std::string>& values, const std::string& value)
{
    return std::find (values.begin(), values.end(), value) != values.end();
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

    const auto registry = myworld::loadCompoundModuleNodeSpecsFromLibrary (
        "fixtures/module-libraries/default.module-library.json");
    expect (registry.ok, registry.error);
    const auto visibleRegistry = myworld::mergeNodeSpecs (myworld::makeSeedNodeSpecs(), registry.specs);
    const myworld::CanvasHandViewport viewport { 800.0, 560.0 };

    auto rootSession = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    expect (myworld::createNode (rootSession,
                                 visibleRegistry,
                                 "compound.loudness",
                                 "library_loud1",
                                 { 220.0, 260.0 }).ok,
            "create loaded compound node for collapsed drag");
    expect (myworld::setCollapsed (rootSession, "library_loud1", true).ok, "collapse loaded compound node");

    const auto collapsedDrag = myworld::runCanvasHandTrace (
        rootSession,
        visibleRegistry,
        viewport,
        { myworld::canvasHandDrag (myworld::canvasHandNode ("library_loud1"),
                                  myworld::canvasHandPoint ({ 380.0, 340.0 })) });
    expect (collapsedDrag.ok, collapsedDrag.errors.empty() ? "collapsed drag" : collapsedDrag.errors.front());
    expect (contains (collapsedDrag.commandLogDelta, "move_node"), "collapsed compound drag logs move_node");
    expect (rootSession.selectedNodeIds.size() == 1 && rootSession.selectedNodeIds.front() == "library_loud1",
            "collapsed compound remains selected after drag");
    const auto* movedCompound = findNode (rootSession.graph, "library_loud1");
    expect (movedCompound != nullptr, "moved compound exists");
    expect (movedCompound->collapsed, "collapsed flag survives drag");
    expect (movedCompound->position.x > 300.0, "collapsed compound x moved");

    expect (myworld::enterPatch (rootSession, "library_loud1").ok, "enter loaded compound patch");
    expect (rootSession.currentPatchPath.size() == 1 && rootSession.currentPatchPath.front() == "library_loud1",
            "loaded compound patch path recorded");

    const auto encodedRoot = myworld::serializeInteractionState (rootSession);
    const auto restoredRoot = myworld::deserializeInteractionState (encodedRoot);
    const auto* restoredCompound = findNode (restoredRoot.graph, "library_loud1");
    expect (restoredCompound != nullptr, "root compound survives roundtrip");
    expect (restoredCompound->collapsed, "root collapsed state survives roundtrip");
    expect (restoredCompound->position.x == movedCompound->position.x, "root compound x roundtrips");
    expect (restoredRoot.currentPatchPath.size() == 1 && restoredRoot.currentPatchPath.front() == "library_loud1",
            "root patch path roundtrips");

    const auto expandedGraph = myworld::makeCompoundPatchInteractionGraph (loaded.spec, "library_loud1");
    expect (expandedGraph.editorGraph.nodes.size() == loaded.spec.children.size(), "expanded graph child count");
    expect (findNode (expandedGraph, "library_loud1/audio_in") != nullptr, "expanded audio_in node exists");
    expect (findNode (expandedGraph, "library_loud1/loudness_out") != nullptr, "expanded loudness_out node exists");
    expect (expandedGraph.editorGraph.edges.size() == 8, "expanded graph keeps child-to-child internal edges");
    expect (expandedGraph.editorGraph.edges.front().from == "library_loud1/audio_in.channels",
            "expanded edge source is parent-qualified");
    expect (expandedGraph.editorGraph.edges.front().to == "library_loud1/mono_mix.input",
            "expanded edge target is parent-qualified");

    const auto invariant = myworld::validateGraphInvariants (expandedGraph, myworld::makeSeedNodeSpecs());
    expect (invariant.ok, invariant.errors.empty() ? "expanded graph invariant" : invariant.errors.front());

    auto expandedSession = myworld::makeGraphSession (expandedGraph);
    const auto expandedDrag = myworld::runCanvasHandTrace (
        expandedSession,
        myworld::makeSeedNodeSpecs(),
        viewport,
        { myworld::canvasHandDrag (myworld::canvasHandNode ("library_loud1/mono_mix"),
                                  myworld::canvasHandPoint ({ 420.0, 230.0 })) });
    expect (expandedDrag.ok, expandedDrag.errors.empty() ? "expanded drag" : expandedDrag.errors.front());
    expect (contains (expandedDrag.commandLogDelta, "move_node"), "expanded child drag logs move_node");
    expect (expandedSession.selectedNodeIds.size() == 1
                && expandedSession.selectedNodeIds.front() == "library_loud1/mono_mix",
            "expanded child remains selected after drag");

    const auto encodedExpanded = myworld::serializeInteractionState (expandedSession);
    const auto restoredExpanded = myworld::deserializeInteractionState (encodedExpanded);
    const auto* restoredMonoMix = findNode (restoredExpanded.graph, "library_loud1/mono_mix");
    expect (restoredMonoMix != nullptr, "expanded child survives roundtrip");
    expect (restoredMonoMix->position.x == findNode (expandedSession.graph, "library_loud1/mono_mix")->position.x,
            "expanded child x roundtrips");
    expect (restoredExpanded.graph.editorGraph.edges.size() == expandedSession.graph.editorGraph.edges.size(),
            "expanded internal edges roundtrip");

    std::cout << "compound interaction ok\n";
    return 0;
}
