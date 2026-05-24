#include "CompoundModule.h"
#include "CompoundPatch.h"
#include "GraphEndpoint.h"
#include "InteractionContract.h"
#include "StorageContract.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
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

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}

bool hasEdgeId (const myworld::GraphContract& graph, const std::string& edgeId)
{
    return std::find_if (graph.editorGraph.edges.begin(),
                         graph.editorGraph.edges.end(),
                         [&edgeId] (const auto& edge) {
                             return edge.id == edgeId;
                         }) != graph.editorGraph.edges.end();
}
}

int main()
{
    const auto loadedCompound = myworld::loadCompoundPatchSpec ("fixtures/compounds/loudness.compound.json");
    expect (loadedCompound.ok, loadedCompound.error);

    const auto moduleRegistry = myworld::loadCompoundModuleNodeSpecsFromLibrary (
        "fixtures/module-libraries/default.module-library.json");
    expect (moduleRegistry.ok, moduleRegistry.error);
    const auto visibleRegistry = myworld::mergeNodeSpecs (myworld::makeSeedNodeSpecs(), moduleRegistry.specs);

    auto rootSession = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    expect (myworld::createNode (rootSession,
                                 visibleRegistry,
                                 "audio.input",
                                 "live_audio",
                                 { 80.0, 260.0 }).ok,
            "create audio input");
    expect (myworld::createNode (rootSession,
                                 visibleRegistry,
                                 "compound.loudness",
                                 "library_loud1",
                                 { 220.0, 260.0 }).ok,
            "create compound instance");
    expect (myworld::createNode (rootSession,
                                 visibleRegistry,
                                 "io.midi.cc_out",
                                 "midi_loudness",
                                 { 560.0, 280.0 }).ok,
            "create midi output");
    expect (myworld::setCollapsed (rootSession, "library_loud1", true).ok, "collapse compound");

    const auto publicInput = myworld::connectPorts (rootSession,
                                                    visibleRegistry,
                                                    "live_audio.channels",
                                                    "library_loud1.audio.in");
    expect (publicInput.ok, publicInput.message);

    const auto publicOutput = myworld::connectPorts (rootSession,
                                                     visibleRegistry,
                                                     "library_loud1.out",
                                                     "midi_loudness.value");
    expect (publicOutput.ok, publicOutput.message);

    auto expandedSession = myworld::makeGraphSession (
        myworld::makeCompoundPatchInteractionGraph (loadedCompound.spec, "library_loud1"));
    expect (myworld::moveNode (expandedSession, "library_loud1/mono_mix", 48.0, 36.0).ok,
            "move expanded mono_mix");

    const auto movedMonoMix = myworld::findEditorNode (expandedSession.graph, "library_loud1/mono_mix");
    expect (movedMonoMix != nullptr, "moved mono_mix exists");

    const auto storeLayout = myworld::storeExpandedPatchLayout (rootSession,
                                                                "library_loud1",
                                                                expandedSession.graph);
    expect (storeLayout.ok, storeLayout.message);

    const auto document = myworld::makePatchDocument ("patch.c2-main", "C2 Main", rootSession.graph);
    const auto json = myworld::toJson (document);
    expectContains (json, "\"kind\": \"patchDocument\"", "patch document json");
    expectContains (json, "\"editorGraph\"", "patch document json");
    expectContains (json, "\"runtimeGraph\"", "patch document json");
    expectContains (json, "\"library_loud1.audio.in\"", "patch document json");
    expect (json.find ("interaction-state-v1") == std::string::npos,
            "patch document must not use temporary interaction serializer");

    const auto parsed = myworld::parsePatchDocument (json);
    expect (parsed.ok, parsed.error);
    expect (parsed.document.id == "patch.c2-main", "patch document id roundtrip");
    expect (parsed.document.graph.editorGraph.nodes.size() == rootSession.graph.editorGraph.nodes.size(),
            "editor nodes roundtrip");
    expect (parsed.document.graph.runtimeGraph.nodes.size() == rootSession.graph.runtimeGraph.nodes.size(),
            "runtime nodes roundtrip");

    const auto* reloadedCompound = myworld::findEditorNode (parsed.document.graph, "library_loud1");
    expect (reloadedCompound != nullptr, "compound node reloads from patch document");
    expect (reloadedCompound->collapsed, "compound collapsed state reloads");
    expect (hasEdgeId (parsed.document.graph, "edge.live_audio.channels.library_loud1.audio.in"),
            "public input edge reloads");
    expect (hasEdgeId (parsed.document.graph, "edge.library_loud1.out.midi_loudness.value"),
            "public output edge reloads");

    const auto relayoutGraph = myworld::makeCompoundPatchInteractionGraph (loadedCompound.spec,
                                                                           "library_loud1",
                                                                           parsed.document.graph);
    const auto* reloadedMonoMix = myworld::findEditorNode (relayoutGraph, "library_loud1/mono_mix");
    expect (reloadedMonoMix != nullptr, "relayout mono_mix reloads");
    expect (reloadedMonoMix->position.x == movedMonoMix->position.x,
            "expanded child layout x reloads from patch document");
    expect (reloadedMonoMix->position.y == movedMonoMix->position.y,
            "expanded child layout y reloads from patch document");

    const auto loadedFixture = myworld::loadPatchDocument (
        "fixtures/storage/c2-compound-work/patches/main.patch.json");
    expect (loadedFixture.ok, loadedFixture.error);
    expect (hasEdgeId (loadedFixture.document.graph, "edge.live_audio.channels.library_loud1.audio.in"),
            "fixture public input edge reloads");
    expect (hasEdgeId (loadedFixture.document.graph, "edge.library_loud1.out.midi_loudness.value"),
            "fixture public output edge reloads");
    expect (loadedFixture.document.graph.runtimeGraph.edges.size()
                == loadedFixture.document.graph.editorGraph.edges.size(),
            "fixture runtime graph matches editor graph edge count");

    const auto fixtureRelayoutGraph = myworld::makeCompoundPatchInteractionGraph (
        loadedCompound.spec,
        "library_loud1",
        loadedFixture.document.graph);
    const auto* fixtureMonoMix = myworld::findEditorNode (fixtureRelayoutGraph, "library_loud1/mono_mix");
    expect (fixtureMonoMix != nullptr, "fixture relayout mono_mix reloads");
    expect (fixtureMonoMix->position.x == 358.0, "fixture expanded child layout x reloads");
    expect (fixtureMonoMix->position.y == 146.0, "fixture expanded child layout y reloads");

    const auto activePatchPath = std::filesystem::temp_directory_path() / "my-world-c2-active-main.patch.json";
    std::filesystem::remove (activePatchPath);

    const auto saveResult = myworld::savePatchDocument (activePatchPath.string(), document);
    expect (saveResult.ok, saveResult.error);
    expect (saveResult.status == "save-ok commit-pending", "patch document save status");
    expect (myworld::isKnownSaveStatus (saveResult.status), "patch document save status is known");
    expect (std::filesystem::exists (activePatchPath), "active patch document file exists");

    const auto activeReload = myworld::loadPatchDocument (activePatchPath.string());
    expect (activeReload.ok, activeReload.error);

    auto reloadedSession = myworld::makeGraphSession (activeReload.document.graph);
    expect (! reloadedSession.dirty, "reloaded patch document session starts clean");
    expect (hasEdgeId (reloadedSession.graph, "edge.live_audio.channels.library_loud1.audio.in"),
            "active file public input edge reloads into GraphSession");
    expect (hasEdgeId (reloadedSession.graph, "edge.library_loud1.out.midi_loudness.value"),
            "active file public output edge reloads into GraphSession");
    expect (reloadedSession.graph.runtimeGraph.edges.size() == reloadedSession.graph.editorGraph.edges.size(),
            "active file runtime graph reloads with editor edges");

    const auto activeRelayoutGraph = myworld::makeCompoundPatchInteractionGraph (
        loadedCompound.spec,
        "library_loud1",
        reloadedSession.graph);
    const auto* activeMonoMix = myworld::findEditorNode (activeRelayoutGraph, "library_loud1/mono_mix");
    expect (activeMonoMix != nullptr, "active file relayout mono_mix reloads");
    expect (activeMonoMix->position.x == movedMonoMix->position.x,
            "active file expanded child layout x reloads");
    expect (activeMonoMix->position.y == movedMonoMix->position.y,
            "active file expanded child layout y reloads");

    const auto loadedWork = myworld::loadWorkProjectManifest (
        "fixtures/storage/c2-compound-work/myworld.work.json");
    expect (loadedWork.ok, loadedWork.error);
    expect (loadedWork.manifest.id == "work.c2-compound", "work manifest id reloads");
    expect (loadedWork.manifest.mainPatchPath == "patches/main.patch.json", "work main patch path reloads");
    expect (loadedWork.manifest.moduleLibraries.size() == 1, "work module library refs reload");
    expect (loadedWork.manifest.moduleLibraries.front() == "../../module-libraries/default.module-library.json",
            "work module library ref remains work-level data");

    const auto workMainPatch = myworld::loadMainPatchDocumentForWork (
        "fixtures/storage/c2-compound-work/myworld.work.json");
    expect (workMainPatch.ok, workMainPatch.error);
    auto workSession = myworld::makeGraphSession (workMainPatch.document.graph);
    expect (hasEdgeId (workSession.graph, "edge.live_audio.channels.library_loud1.audio.in"),
            "work main patch public input edge reloads into GraphSession");
    expect (hasEdgeId (workSession.graph, "edge.library_loud1.out.midi_loudness.value"),
            "work main patch public output edge reloads into GraphSession");

    std::filesystem::remove (activePatchPath);

    std::cout << "patch document boundary ok\n";
    return 0;
}
