#pragma once

#include "GraphContract.h"
#include "NodeSpec.h"

#include <string>
#include <vector>

namespace myworld
{
struct GraphEndpointParts
{
    std::string nodeId;
    std::string portId;

    bool hasPort() const;
};

GraphEndpointParts splitGraphEndpoint (const std::string& endpoint);
std::string nodeIdFromEndpoint (const std::string& endpoint);
std::string portIdFromEndpoint (const std::string& endpoint);

GraphNode* findEditorNode (GraphContract& graph, const std::string& id);
const GraphNode* findEditorNode (const GraphContract& graph, const std::string& id);
const NodeSpec* specForNode (const GraphContract& graph,
                             const std::vector<NodeSpec>& specs,
                             const std::string& nodeId);
}
