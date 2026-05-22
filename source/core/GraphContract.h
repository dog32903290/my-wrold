#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct GraphPoint
{
    double x = 0.0;
    double y = 0.0;
};

struct GraphNode
{
    struct ParamValue
    {
        std::string id;
        std::string value;
    };

    struct PortBindingValue
    {
        std::string portId;
        std::string bindingMode;
        std::string value;
    };

    std::string id;
    std::string type;
    std::vector<std::string> systemUniforms;
    GraphPoint position;
    bool collapsed = false;
    std::vector<ParamValue> params;
    std::vector<PortBindingValue> portBindings;
};

struct GraphEdge
{
    std::string from;
    std::string to;
    std::string id;
    std::string dataType;
    std::string streamKind;
};

struct EditorGraph
{
    std::vector<GraphNode> nodes;
    std::vector<GraphEdge> edges;
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
std::string makeCookOrderJson (const GraphContract& graph);
std::string makeNodeStatsJson (const GraphContract& graph,
                               int viewportWidth,
                               int viewportHeight,
                               unsigned int frameIndex,
                               double timeSeconds,
                               const std::string& renderer,
                               const std::string& shaderStatus);
}
