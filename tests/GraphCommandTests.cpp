#include "GraphContract.h"
#include "GraphEndpoint.h"
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

}

int main()
{
    expect (myworld::isKnownCommandType ("move_node"), "move_node command vocabulary");
    expect (myworld::isKnownCommandType ("delete_node"), "delete_node command vocabulary");
    expect (myworld::isKnownCommandType ("disconnect"), "disconnect command vocabulary");
    expect (myworld::isKnownCommandType ("reconnect"), "reconnect command vocabulary");
    expect (myworld::isKnownCommandType ("split_edge_create_node"), "split edge command vocabulary");

    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());

    const auto* shader = myworld::findEditorNode (session.graph, "shader1");
    expect (shader != nullptr, "shader exists");
    const auto originalX = shader->position.x;
    const auto originalY = shader->position.y;

    auto move = myworld::moveNode (session, "shader1", 40.0, 20.0);
    expect (move.ok, "move_node succeeds");
    shader = myworld::findEditorNode (session.graph, "shader1");
    expect (shader->position.x == originalX + 40.0, "shader x moved");
    expect (shader->position.y == originalY + 20.0, "shader y moved");
    expect (session.commandLog.back() == "move_node", "move command logged");

    expect (myworld::undo (session), "undo move");
    shader = myworld::findEditorNode (session.graph, "shader1");
    expect (shader->position.x == originalX, "undo restores x");
    expect (shader->position.y == originalY, "undo restores y");

    expect (myworld::redo (session), "redo move");
    shader = myworld::findEditorNode (session.graph, "shader1");
    expect (shader->position.x == originalX + 40.0, "redo restores moved x");

    session.selectedEdgeIds = { "edge.shader1.output.out1.input" };
    const auto disconnect = myworld::disconnectEdge (session, "edge.shader1.output.out1.input");
    expect (disconnect.ok, "disconnect succeeds");
    expect (session.graph.editorGraph.edges.empty(), "editor edge removed");
    expect (session.graph.runtimeGraph.edges.empty(), "runtime edge removed");
    expect (session.selectedEdgeIds.empty(), "deleted edge selection cleared");

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

    const auto deleteShader = myworld::deleteNode (session, "shader1");
    expect (deleteShader.ok, "delete node succeeds");
    expect (myworld::findEditorNode (session.graph, "shader1") == nullptr, "deleted node removed from editor graph");
    expect (session.graph.editorGraph.edges.empty(), "incident editor edges removed");
    expect (session.graph.runtimeGraph.edges.empty(), "incident runtime edges removed");
    expect (session.selectedNodeIds.empty(), "deleted node selection cleared");
    expect (session.commandLog.back() == "delete_node", "delete node command logged");

    expect (myworld::undo (session), "undo delete node");
    expect (myworld::findEditorNode (session.graph, "shader1") != nullptr, "undo restores deleted node");
    expect (session.graph.editorGraph.edges.size() == 1, "undo restores incident edge");

    expect (myworld::redo (session), "redo delete node");
    expect (myworld::findEditorNode (session.graph, "shader1") == nullptr, "redo removes node again");

    std::cout << "graph commands ok\n";
    return 0;
}
