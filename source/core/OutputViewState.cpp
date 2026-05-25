#include "OutputViewState.h"

#include <algorithm>

namespace myworld
{
std::string activeOutputNodeId (const OutputViewState& state)
{
    return state.pinned ? state.pinnedNodeId : state.followedNodeId;
}

bool outputViewTargetExists (const GraphContract& graph, const std::string& nodeId)
{
    if (nodeId.empty())
        return false;

    const auto found = std::find_if (graph.editorGraph.nodes.begin(),
                                     graph.editorGraph.nodes.end(),
                                     [&nodeId] (const auto& node) {
                                         return node.id == nodeId;
                                     });

    return found != graph.editorGraph.nodes.end();
}

void followSelectedOutputNode (OutputViewState& state,
                               const GraphContract& graph,
                               const std::vector<std::string>& selectedNodeIds)
{
    if (state.pinned)
        return;

    state.followedNodeId.clear();

    if (selectedNodeIds.empty())
        return;

    const auto& selectedNodeId = selectedNodeIds.front();
    if (outputViewTargetExists (graph, selectedNodeId))
        state.followedNodeId = selectedNodeId;
}

OutputViewResult pinOutputViewToNode (OutputViewState& state,
                                      const GraphContract& graph,
                                      const std::string& nodeId)
{
    if (! outputViewTargetExists (graph, nodeId))
        return { false, "missing output target: " + nodeId };

    state.pinned = true;
    state.pinnedNodeId = nodeId;
    state.followedNodeId = nodeId;
    return { true, "pinned output: " + nodeId };
}

OutputViewResult pinOutputViewToSelection (OutputViewState& state,
                                           const GraphContract& graph,
                                           const std::vector<std::string>& selectedNodeIds)
{
    if (selectedNodeIds.empty())
        return { false, "select a node to pin output" };

    return pinOutputViewToNode (state, graph, selectedNodeIds.front());
}

void unpinOutputView (OutputViewState& state,
                      const GraphContract& graph,
                      const std::vector<std::string>& selectedNodeIds)
{
    state.pinned = false;
    state.pinnedNodeId.clear();
    followSelectedOutputNode (state, graph, selectedNodeIds);
}
}
