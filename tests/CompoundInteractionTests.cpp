#include "CanvasHands.h"
#include "CompoundModule.h"
#include "CompoundPatch.h"
#include "GraphEndpoint.h"
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
    const auto* movedCompound = myworld::findEditorNode (rootSession.graph, "library_loud1");
    expect (movedCompound != nullptr, "moved compound exists");
    expect (movedCompound->collapsed, "collapsed flag survives drag");
    expect (movedCompound->position.x > 300.0, "collapsed compound x moved");
    const auto movedCompoundX = movedCompound->position.x;

    expect (myworld::createNode (rootSession,
                                 visibleRegistry,
                                 "audio.input",
                                 "live_audio",
                                 { 80.0, 260.0 }).ok,
            "create audio input for compound public input");
    expect (myworld::createNode (rootSession,
                                 visibleRegistry,
                                 "io.midi.cc_out",
                                 "midi_loudness",
                                 { 560.0, 280.0 }).ok,
            "create midi output for compound public output");
    const auto publicInput = myworld::portCenter (rootSession.graph, visibleRegistry, "library_loud1.audio.in");
    const auto publicOutput = myworld::portCenter (rootSession.graph, visibleRegistry, "library_loud1.out");
    expect (publicInput.ok, "collapsed compound public input has port center");
    expect (publicOutput.ok, "collapsed compound public output has port center");

    const auto connectPublicInput = myworld::connectPorts (rootSession,
                                                           visibleRegistry,
                                                           "live_audio.channels",
                                                           "library_loud1.audio.in");
    expect (connectPublicInput.ok, connectPublicInput.message);
    const auto connectPublicOutput = myworld::connectPorts (rootSession,
                                                            visibleRegistry,
                                                            "library_loud1.out",
                                                            "midi_loudness.value");
    expect (connectPublicOutput.ok, connectPublicOutput.message);
    expect (contains (rootSession.commandLog, "connect"), "compound public port connect logs command");
    expect (rootSession.graph.editorGraph.edges.size() >= 2, "compound public port edges inserted");
    const auto publicInputEdge = rootSession.graph.editorGraph.edges[rootSession.graph.editorGraph.edges.size() - 2];
    const auto publicOutputEdge = rootSession.graph.editorGraph.edges.back();
    expect (rootSession.graph.editorGraph.edges[rootSession.graph.editorGraph.edges.size() - 2].dataType == "audio.channels",
            "compound public input edge keeps audio channel type");
    expect (rootSession.graph.editorGraph.edges.back().dataType == "signal.float",
            "compound public output edge keeps signal type");

    expect (myworld::enterPatch (rootSession, "library_loud1").ok, "enter loaded compound patch");
    expect (rootSession.currentPatchPath.size() == 1 && rootSession.currentPatchPath.front() == "library_loud1",
            "loaded compound patch path recorded");

    const auto encodedRoot = myworld::serializeInteractionState (rootSession);
    const auto restoredRoot = myworld::deserializeInteractionState (encodedRoot);
    const auto* restoredCompound = myworld::findEditorNode (restoredRoot.graph, "library_loud1");
    expect (restoredCompound != nullptr, "root compound survives roundtrip");
    expect (restoredCompound->collapsed, "root collapsed state survives roundtrip");
    expect (restoredCompound->position.x == movedCompoundX, "root compound x roundtrips");
    expect (restoredRoot.currentPatchPath.size() == 1 && restoredRoot.currentPatchPath.front() == "library_loud1",
            "root patch path roundtrips");

    const auto expandedGraph = myworld::makeCompoundPatchInteractionGraph (loaded.spec, "library_loud1");
    expect (expandedGraph.editorGraph.nodes.size() == loaded.spec.children.size(), "expanded graph child count");
    expect (myworld::findEditorNode (expandedGraph, "library_loud1/audio_in") != nullptr, "expanded audio_in node exists");
    expect (myworld::findEditorNode (expandedGraph, "library_loud1/loudness_out") != nullptr, "expanded loudness_out node exists");
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
    const auto* restoredMonoMix = myworld::findEditorNode (restoredExpanded.graph, "library_loud1/mono_mix");
    expect (restoredMonoMix != nullptr, "expanded child survives roundtrip");
    expect (restoredMonoMix->position.x == myworld::findEditorNode (expandedSession.graph, "library_loud1/mono_mix")->position.x,
            "expanded child x roundtrips");
    expect (restoredExpanded.graph.editorGraph.edges.size() == expandedSession.graph.editorGraph.edges.size(),
            "expanded internal edges roundtrip");

    const auto storeLayout = myworld::storeExpandedPatchLayout (rootSession,
                                                                "library_loud1",
                                                                expandedSession.graph);
    expect (storeLayout.ok, storeLayout.message);
    const auto restoredLayoutRoot = myworld::deserializeInteractionState (myworld::serializeInteractionState (rootSession));
    const auto relayoutGraph = myworld::makeCompoundPatchInteractionGraph (loaded.spec,
                                                                           "library_loud1",
                                                                           restoredLayoutRoot.graph);
    const auto* relayoutMonoMix = myworld::findEditorNode (relayoutGraph, "library_loud1/mono_mix");
    expect (relayoutMonoMix != nullptr, "relayout mono_mix exists");
    expect (relayoutMonoMix->position.x == myworld::findEditorNode (expandedSession.graph, "library_loud1/mono_mix")->position.x,
            "expanded child layout x persists per compound instance");
    expect (relayoutMonoMix->position.y == myworld::findEditorNode (expandedSession.graph, "library_loud1/mono_mix")->position.y,
            "expanded child layout y persists per compound instance");
    expect (contains (rootSession.commandLog, "store_expanded_patch_layout"), "layout store logs command");
    expect (std::find_if (restoredLayoutRoot.graph.editorGraph.edges.begin(),
                          restoredLayoutRoot.graph.editorGraph.edges.end(),
                          [&] (const auto& edge) {
                              return edge.id == publicInputEdge.id;
                          }) != restoredLayoutRoot.graph.editorGraph.edges.end(),
            "public input edge survives layout store");
    expect (std::find_if (restoredLayoutRoot.graph.editorGraph.edges.begin(),
                          restoredLayoutRoot.graph.editorGraph.edges.end(),
                          [&] (const auto& edge) {
                              return edge.id == publicOutputEdge.id;
                          }) != restoredLayoutRoot.graph.editorGraph.edges.end(),
            "public output edge survives layout store");

    std::cout << "compound interaction ok\n";
    return 0;
}
