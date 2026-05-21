#include "GraphContract.h"

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

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}
}

int main()
{
    const auto graph = myworld::makeDefaultShaderOutputGraph();

    expect (graph.version == 1, "graph version should start at 1");
    expect (graph.editorGraph.nodes.size() == 2, "editor graph should contain Shader and Output nodes");
    expect (graph.runtimeGraph.nodes.size() == 2, "runtime graph should contain Shader and Output nodes");
    expect (graph.runtimeGraph.edges.size() == 1, "runtime graph should connect Shader to Output");

    const auto& shader = graph.runtimeGraph.nodes.at (0);
    const auto& output = graph.runtimeGraph.nodes.at (1);
    const auto& edge = graph.runtimeGraph.edges.at (0);

    expect (shader.id == "shader1", "first runtime node should be shader1");
    expect (shader.type == "shader.fragment", "shader node type should be shader.fragment");
    expect (output.id == "out1", "second runtime node should be out1");
    expect (output.type == "output.preview", "output node type should be output.preview");
    expect (edge.from == "shader1.output", "edge should leave shader output port");
    expect (edge.to == "out1.input", "edge should enter output input port");

    expect (myworld::usesSystemUniform (shader, "u_time"), "default shader declares u_time");
    expect (myworld::usesSystemUniform (shader, "u_resolution"), "default shader declares u_resolution");
    expect (myworld::usesSystemUniform (shader, "u_frame"), "default shader declares u_frame");
    expect (myworld::usesSystemUniform (shader, "u_loudness"), "default shader declares u_loudness");

    const auto cookOrderJson = myworld::makeCookOrderJson (graph);
    expectContains (cookOrderJson, "\"version\": 1", "cook order json");
    expectContains (cookOrderJson, "\"cookOrder\"", "cook order json");
    expectContains (cookOrderJson, "\"shader1\"", "cook order json");
    expectContains (cookOrderJson, "\"out1\"", "cook order json");
    expect (cookOrderJson.find ("\"shader1\"") < cookOrderJson.find ("\"out1\""),
            "shader should cook before output");

    const auto nodeStatsJson = myworld::makeNodeStatsJson (graph,
                                                           640,
                                                           360,
                                                           12,
                                                           0.5,
                                                           "OpenGL",
                                                           "compiled: GLSL v4.10");
    expectContains (nodeStatsJson, "\"viewport\"", "node stats json");
    expectContains (nodeStatsJson, "\"width\": 640", "node stats json");
    expectContains (nodeStatsJson, "\"height\": 360", "node stats json");
    expectContains (nodeStatsJson, "\"frameIndex\": 12", "node stats json");
    expectContains (nodeStatsJson, "\"renderer\": \"OpenGL\"", "node stats json");
    expectContains (nodeStatsJson, "\"shaderStatus\": \"compiled: GLSL v4.10\"", "node stats json");
    expectContains (nodeStatsJson, "\"type\": \"shader.fragment\"", "node stats json");
    expectContains (nodeStatsJson, "\"systemUniforms\"", "node stats json");

    std::cout << "graph contract ok\n";
    return 0;
}
