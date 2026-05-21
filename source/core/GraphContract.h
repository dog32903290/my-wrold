#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct GraphNode
{
    std::string id;
    std::string type;
    std::vector<std::string> systemUniforms;
};

struct GraphEdge
{
    std::string from;
    std::string to;
};

struct EditorGraph
{
    std::vector<GraphNode> nodes;
};

struct RuntimeGraph
{
    std::vector<GraphNode> nodes;
    std::vector<GraphEdge> edges;
};

struct GraphContract
{
    int version = 1;
    EditorGraph editorGraph;
    RuntimeGraph runtimeGraph;
};

GraphContract makeDefaultShaderOutputGraph();
bool usesSystemUniform (const GraphNode& node, const std::string& uniformName);
std::string defaultFragmentShader();
}
