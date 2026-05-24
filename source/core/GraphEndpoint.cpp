#include "GraphEndpoint.h"

namespace myworld
{
bool GraphEndpointParts::hasPort() const
{
    return ! portId.empty();
}

GraphEndpointParts splitGraphEndpoint (const std::string& endpoint)
{
    const auto dot = endpoint.find ('.');
    if (dot == std::string::npos)
        return { endpoint, {} };

    return { endpoint.substr (0, dot), endpoint.substr (dot + 1) };
}

std::string nodeIdFromEndpoint (const std::string& endpoint)
{
    return splitGraphEndpoint (endpoint).nodeId;
}

std::string portIdFromEndpoint (const std::string& endpoint)
{
    return splitGraphEndpoint (endpoint).portId;
}

GraphNode* findEditorNode (GraphContract& graph, const std::string& id)
{
    for (auto& node : graph.editorGraph.nodes)
        if (node.id == id)
            return &node;

    return nullptr;
}

const GraphNode* findEditorNode (const GraphContract& graph, const std::string& id)
{
    for (const auto& node : graph.editorGraph.nodes)
        if (node.id == id)
            return &node;

    return nullptr;
}

const NodeSpec* specForNode (const GraphContract& graph,
                             const std::vector<NodeSpec>& specs,
                             const std::string& nodeId)
{
    const auto* node = findEditorNode (graph, nodeId);
    return node == nullptr ? nullptr : findNodeSpec (specs, node->type);
}
}
