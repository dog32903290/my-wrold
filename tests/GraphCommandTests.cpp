#include "GraphContract.h"
#include "GraphLanguage.h"
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
    expect (myworld::isKnownCommandType ("move_node"), "move_node command vocabulary");
    expect (myworld::isKnownCommandType ("disconnect"), "disconnect command vocabulary");
    expect (myworld::isKnownCommandType ("reconnect"), "reconnect command vocabulary");

    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());

    const auto* shader = findNode (session.graph, "shader1");
    expect (shader != nullptr, "shader exists");
    const auto originalX = shader->position.x;
    const auto originalY = shader->position.y;

    auto move = myworld::moveNode (session, "shader1", 40.0, 20.0);
    expect (move.ok, "move_node succeeds");
    shader = findNode (session.graph, "shader1");
    expect (shader->position.x == originalX + 40.0, "shader x moved");
    expect (shader->position.y == originalY + 20.0, "shader y moved");
    expect (session.commandLog.back() == "move_node", "move command logged");

    expect (myworld::undo (session), "undo move");
    shader = findNode (session.graph, "shader1");
    expect (shader->position.x == originalX, "undo restores x");
    expect (shader->position.y == originalY, "undo restores y");

    expect (myworld::redo (session), "redo move");
    shader = findNode (session.graph, "shader1");
    expect (shader->position.x == originalX + 40.0, "redo restores moved x");

    const auto disconnect = myworld::disconnectEdge (session, "edge.shader1.output.out1.input");
    expect (disconnect.ok, "disconnect succeeds");
    expect (session.graph.editorGraph.edges.empty(), "editor edge removed");
    expect (session.graph.runtimeGraph.edges.empty(), "runtime edge removed");

    expect (myworld::undo (session), "undo disconnect");
    expect (session.graph.editorGraph.edges.size() == 1, "editor edge restored");
    expect (session.graph.runtimeGraph.edges.size() == 1, "runtime edge restored");

    expect (myworld::disconnectEdge (session, "edge.shader1.output.out1.input").ok, "disconnect again");
    const auto connect = myworld::connectPorts (session, "shader1.output", "out1.input");
    expect (connect.ok, "connect succeeds");
    expect (session.graph.editorGraph.edges.size() == 1, "editor edge added");
    expect (session.graph.runtimeGraph.edges.size() == 1, "runtime edge added");
    expect (session.graph.editorGraph.edges[0].id == "edge.shader1.output.out1.input", "edge id stable");

    const auto duplicate = myworld::connectPorts (session, "shader1.output", "out1.input");
    expect (! duplicate.ok, "duplicate edge rejected");
    expect (session.graph.editorGraph.edges.size() == 1, "duplicate leaves graph unchanged");

    std::cout << "graph commands ok\n";
    return 0;
}
