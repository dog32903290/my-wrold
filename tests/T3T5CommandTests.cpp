#include "GraphContract.h"
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

    expect (myworld::disconnectEdge (session, "edge.shader1.output.out1.input").ok, "clear default edge");
    const auto createAndConnect = myworld::createNodeAndConnect (session,
                                                                 "shader1.output",
                                                                 "output.preview",
                                                                 "out2",
                                                                 { 520.0, 160.0 });
    expect (createAndConnect.ok, "create node and connect: " + createAndConnect.message);
    expect (findNode (session.graph, "out2") != nullptr, "new output node exists");
    expect (session.graph.editorGraph.edges.back().to == "out2.input", "new node connected");
    expect (session.commandLog.back() == "create_node+connect", "macro command logged");

    expect (myworld::undo (session), "undo create node and connect");
    expect (findNode (session.graph, "out2") == nullptr, "undo removes new node");

    const auto createCompound = myworld::createNode (session, "compound.loudness", "loud1", { 160.0, 260.0 });
    expect (createCompound.ok, "create compound node");

    const auto enter = myworld::enterPatch (session, "loud1");
    expect (enter.ok, "enter compound patch");
    expect (session.currentPatchPath.size() == 1 && session.currentPatchPath[0] == "loud1", "compound path entered");

    const auto exit = myworld::exitPatch (session);
    expect (exit.ok, "exit compound patch");
    expect (session.currentPatchPath.empty(), "compound path exited");

    expect (myworld::setCollapsed (session, "loud1", false).ok, "expand compound");
    expect (! findNode (session.graph, "loud1")->collapsed, "compound expanded");
    expect (myworld::setCollapsed (session, "loud1", true).ok, "collapse compound");
    expect (findNode (session.graph, "loud1")->collapsed, "compound collapsed");

    expect (myworld::setParam (session, "shader1", "fragmentSource", "void main(){}").ok, "set shader param");
    expect (findNode (session.graph, "shader1")->params.size() == 1, "param stored");

    expect (myworld::setPortBinding (session, "shader1", "output", "connected", "out1.input").ok, "set port binding");
    expect (findNode (session.graph, "shader1")->portBindings.size() == 1, "port binding stored");

    std::cout << "t3 t5 commands ok\n";
    return 0;
}
