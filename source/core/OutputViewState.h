#pragma once

#include "GraphContract.h"

#include <string>
#include <vector>

namespace myworld
{
struct OutputViewState
{
    bool pinned = false;
    std::string followedNodeId;
    std::string pinnedNodeId;
};

struct OutputViewResult
{
    bool ok = false;
    std::string message;
};

std::string activeOutputNodeId (const OutputViewState& state);
bool outputViewTargetExists (const GraphContract& graph, const std::string& nodeId);
void followSelectedOutputNode (OutputViewState& state,
                               const GraphContract& graph,
                               const std::vector<std::string>& selectedNodeIds);
OutputViewResult pinOutputViewToNode (OutputViewState& state,
                                      const GraphContract& graph,
                                      const std::string& nodeId);
OutputViewResult pinOutputViewToSelection (OutputViewState& state,
                                           const GraphContract& graph,
                                           const std::vector<std::string>& selectedNodeIds);
void unpinOutputView (OutputViewState& state,
                      const GraphContract& graph,
                      const std::vector<std::string>& selectedNodeIds);
}
