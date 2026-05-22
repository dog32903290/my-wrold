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
    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    expect (! session.dirty, "new session starts clean");

    expect (myworld::moveNode (session, "shader1", 12.0, 8.0).ok, "move marks dirty");
    expect (session.dirty, "session dirty after move");

    expect (myworld::createNode (session, "compound.loudness", "loud1", { 200.0, 220.0 }).ok, "create loudness");
    expect (myworld::setCollapsed (session, "loud1", true).ok, "collapse loudness");
    expect (myworld::enterPatch (session, "loud1").ok, "enter loudness");
    expect (myworld::setParam (session, "shader1", "fragmentSource", "void main(){}").ok, "set param");
    expect (myworld::setPortBinding (session, "shader1", "output", "connected", "out1.input").ok, "set binding");

    const auto saveStatus = myworld::markSavedAndCommitted (session);
    expect (saveStatus == "saved-and-committed", "save status");
    expect (myworld::isKnownSaveStatus (saveStatus), "save status known");
    expect (! session.dirty, "session clean after save");

    const auto encoded = myworld::serializeInteractionState (session);
    auto loaded = myworld::deserializeInteractionState (encoded);

    expect (! loaded.dirty, "loaded session clean");
    expect (loaded.currentPatchPath.size() == 1 && loaded.currentPatchPath[0] == "loud1", "patch path roundtrip");
    expect (loaded.graph.editorGraph.edges.size() == session.graph.editorGraph.edges.size(), "edges roundtrip");

    const auto* shader = findNode (loaded.graph, "shader1");
    expect (shader != nullptr, "shader roundtrip");
    expect (shader->position.x == findNode (session.graph, "shader1")->position.x, "position x roundtrip");
    expect (shader->params.size() == 1, "param roundtrip");
    expect (shader->portBindings.size() == 1, "binding roundtrip");

    const auto* loudness = findNode (loaded.graph, "loud1");
    expect (loudness != nullptr, "loudness roundtrip");
    expect (loudness->collapsed, "collapsed roundtrip");

    std::cout << "interaction storage roundtrip ok\n";
    return 0;
}
