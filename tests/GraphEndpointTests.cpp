#include "GraphContract.h"
#include "GraphEndpoint.h"
#include "NodeSpec.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (condition)
        return;

    std::cerr << "FAIL: " << message << '\n';
    std::exit (1);
}
}

int main()
{
    const auto shaderOutput = myworld::splitGraphEndpoint ("shader1.output");
    expect (shaderOutput.nodeId == "shader1", "endpoint node id");
    expect (shaderOutput.portId == "output", "endpoint port id");
    expect (shaderOutput.hasPort(), "endpoint reports port");

    const auto compoundPublicInput = myworld::splitGraphEndpoint ("loud1.audio.in");
    expect (compoundPublicInput.nodeId == "loud1", "compound endpoint keeps parent node id");
    expect (compoundPublicInput.portId == "audio.in", "compound endpoint keeps dotted port id");

    const auto nodeOnly = myworld::splitGraphEndpoint ("shader1");
    expect (nodeOnly.nodeId == "shader1", "node-only endpoint node id");
    expect (nodeOnly.portId.empty(), "node-only endpoint has no port");
    expect (! nodeOnly.hasPort(), "node-only endpoint reports no port");

    auto graph = myworld::makeDefaultShaderOutputGraph();
    auto* mutableShader = myworld::findEditorNode (graph, "shader1");
    expect (mutableShader != nullptr, "mutable editor node lookup");
    mutableShader->position.x += 5.0;
    expect (myworld::findEditorNode (graph, "shader1")->position.x == 85.0, "mutable lookup returns graph node");
    expect (myworld::findEditorNode (graph, "missing") == nullptr, "missing mutable lookup");

    const auto specs = myworld::makeSeedNodeSpecs();
    const auto* shaderSpec = myworld::specForNode (graph, specs, "shader1");
    expect (shaderSpec != nullptr, "spec lookup for graph node");
    expect (shaderSpec->type == "shader.fragment", "spec lookup returns node type");
    expect (myworld::specForNode (graph, specs, "missing") == nullptr, "missing spec lookup");

    std::cout << "graph endpoint tests ok\n";
    return 0;
}
