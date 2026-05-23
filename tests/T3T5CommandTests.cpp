#include "GraphContract.h"
#include "InteractionContract.h"

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

const myworld::GraphNode* findNode (const myworld::GraphContract& graph, const std::string& id)
{
    for (const auto& node : graph.editorGraph.nodes)
        if (node.id == id)
            return &node;

    return nullptr;
}

std::string paramValue (const myworld::GraphNode& node, const std::string& paramId)
{
    for (const auto& param : node.params)
        if (param.id == paramId)
            return param.value;

    return {};
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

    const std::vector<myworld::NodeCreationGate> creationGates {
        { "compound.loudness", true, "" },
        { "compound.loudness.missing-runtimeop", false, "missing RuntimeOp: debug.unsupported" }
    };
    auto gatedSpecs = myworld::makeSeedNodeSpecs();
    auto missingRuntimeSpec = *myworld::findNodeSpec (gatedSpecs, "compound.loudness");
    missingRuntimeSpec.type = "compound.loudness.missing-runtimeop";
    missingRuntimeSpec.displayName = "Loudness Missing RuntimeOp";
    gatedSpecs.push_back (missingRuntimeSpec);

    auto gatedSession = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    const auto blockedCreate = myworld::createNode (gatedSession,
                                                    gatedSpecs,
                                                    creationGates,
                                                    "compound.loudness.missing-runtimeop",
                                                    "blocked_loud1",
                                                    { 180.0, 260.0 });
    expect (! blockedCreate.ok, "missing RuntimeOp create should be blocked");
    expect (blockedCreate.message.find ("debug.unsupported") != std::string::npos,
            "blocked create names missing RuntimeOp");
    expect (findNode (gatedSession.graph, "blocked_loud1") == nullptr, "blocked create does not mutate graph");

    const auto blockedConnect = myworld::createNodeAndConnect (gatedSession,
                                                               gatedSpecs,
                                                               creationGates,
                                                               "shader1.output",
                                                               "compound.loudness.missing-runtimeop",
                                                               "blocked_loud2",
                                                               { 240.0, 280.0 });
    expect (! blockedConnect.ok, "missing RuntimeOp create-and-connect should be blocked");
    expect (blockedConnect.message.find ("debug.unsupported") != std::string::npos,
            "blocked create-and-connect names missing RuntimeOp");
    expect (findNode (gatedSession.graph, "blocked_loud2") == nullptr,
            "blocked create-and-connect does not mutate graph");

    const auto emptyOverride = myworld::createNodeWithDebugOverride (gatedSession,
                                                                     gatedSpecs,
                                                                     creationGates,
                                                                     "compound.loudness.missing-runtimeop",
                                                                     "override_empty",
                                                                     { 220.0, 300.0 },
                                                                     "");
    expect (! emptyOverride.ok, "debug override requires a visible reason");
    expect (findNode (gatedSession.graph, "override_empty") == nullptr,
            "empty debug override does not mutate graph");

    const auto overrideCreate = myworld::createNodeWithDebugOverride (gatedSession,
                                                                      gatedSpecs,
                                                                      creationGates,
                                                                      "compound.loudness.missing-runtimeop",
                                                                      "override_loud1",
                                                                      { 220.0, 300.0 },
                                                                      "repair missing RuntimeOp");
    expect (overrideCreate.ok, "debug override create succeeds");
    const auto* overrideNode = findNode (gatedSession.graph, "override_loud1");
    expect (overrideNode != nullptr, "debug override node exists");
    expect (paramValue (*overrideNode, "debug.creationOverride") == "true",
            "debug override flag stored");
    expect (paramValue (*overrideNode, "debug.creationOverrideReason") == "repair missing RuntimeOp",
            "debug override reason stored");
    expect (paramValue (*overrideNode, "debug.creationBlockedReason").find ("debug.unsupported") != std::string::npos,
            "debug override blocked reason stored");
    expect (gatedSession.commandLog.back() == "create_node_debug_override",
            "debug override command logged");
    expect (myworld::undo (gatedSession), "undo debug override create");
    expect (findNode (gatedSession.graph, "override_loud1") == nullptr,
            "undo removes debug override node");
    expect (gatedSession.commandLog.back() == "undo:create_node_debug_override",
            "undo debug override command logged");

    expect (myworld::createNode (gatedSession,
                                 gatedSpecs,
                                 "audio.input",
                                 "audio1",
                                 { 80.0, 320.0 }).ok,
            "create audio source for debug override connect");

    const auto overrideConnect = myworld::createNodeAndConnectWithDebugOverride (gatedSession,
                                                                                gatedSpecs,
                                                                                creationGates,
                                                                                "audio1.channels",
                                                                                "compound.loudness.missing-runtimeop",
                                                                                "override_loud2",
                                                                                { 240.0, 320.0 },
                                                                                "wire for RuntimeOp repair");
    expect (overrideConnect.ok, "debug override create-and-connect succeeds");
    const auto* overrideConnectNode = findNode (gatedSession.graph, "override_loud2");
    expect (overrideConnectNode != nullptr, "debug override connected node exists");
    expect (paramValue (*overrideConnectNode, "debug.creationOverrideReason") == "wire for RuntimeOp repair",
            "debug override connect reason stored");
    expect (gatedSession.graph.editorGraph.edges.back().to == "override_loud2.audio.in",
            "debug override node connected");
    expect (gatedSession.commandLog.back() == "create_node+connect_debug_override",
            "debug override connect command logged");

    const auto gatedCreate = myworld::createNode (gatedSession,
                                                  gatedSpecs,
                                                  creationGates,
                                                  "compound.loudness",
                                                  "gated_loud1",
                                                  { 180.0, 260.0 });
    expect (gatedCreate.ok, "runtime-ready module create is allowed");
    expect (findNode (gatedSession.graph, "gated_loud1") != nullptr, "gated create mutates graph");

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
