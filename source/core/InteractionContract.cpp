#include "InteractionContract.h"

#include "CanvasGeometry.h"
#include "CompoundPatch.h"
#include "GraphEndpoint.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <utility>

namespace myworld
{
namespace
{
bool isValidScale (double scale)
{
    return std::isfinite (scale) && scale > 0.01 && scale < 100.0;
}

GraphSession::Snapshot snapshotOf (const GraphSession& session)
{
    return { session.graph,
             session.view,
             session.selectedNodeIds,
             session.selectedEdgeIds,
             session.currentPatchPath,
             session.dirty };
}

void restoreSnapshot (GraphSession& session, const GraphSession::Snapshot& snapshot)
{
    session.graph = snapshot.graph;
    session.view = snapshot.view;
    session.selectedNodeIds = snapshot.selectedNodeIds;
    session.selectedEdgeIds = snapshot.selectedEdgeIds;
    session.currentPatchPath = snapshot.currentPatchPath;
    session.dirty = snapshot.dirty;
}

std::string makeEdgeId (const std::string& from, const std::string& to)
{
    return "edge." + from + "." + to;
}

const PortSpec* findPort (const NodeSpec& spec, const std::string& portId, const std::string& direction)
{
    const auto& ports = direction == "out" ? spec.outputs : spec.inputs;

    for (const auto& port : ports)
        if (port.id == portId)
            return &port;

    return nullptr;
}

const NodeCreationGate* creationGateForNodeType (const std::vector<NodeCreationGate>& creationGates,
                                                 const std::string& nodeType)
{
    const auto found = std::find_if (creationGates.begin(), creationGates.end(), [&] (const auto& gate) {
        return gate.nodeType == nodeType;
    });

    return found == creationGates.end() ? nullptr : &*found;
}

CommandResult checkCreationGate (const std::vector<NodeCreationGate>& creationGates, const std::string& nodeType)
{
    const auto* gate = creationGateForNodeType (creationGates, nodeType);
    if (gate == nullptr || gate->canCreate)
        return { true, "create allowed" };

    return { false, gate->reason.empty() ? "create blocked: " + nodeType : gate->reason };
}

bool isBlank (const std::string& text)
{
    return text.find_first_not_of (" \t\r\n") == std::string::npos;
}

CommandResult checkDebugOverrideGate (const std::vector<NodeCreationGate>& creationGates,
                                      const std::string& nodeType,
                                      const std::string& overrideReason)
{
    if (isBlank (overrideReason))
        return { false, "debug override requires reason" };

    const auto* gate = creationGateForNodeType (creationGates, nodeType);
    if (gate == nullptr || gate->canCreate)
        return { false, "debug override requires blocked node type: " + nodeType };

    return { true, gate->reason };
}

void appendDebugOverrideParams (GraphNode& node,
                                const std::string& overrideReason,
                                const std::string& blockedReason)
{
    node.params.push_back ({ "debug.creationOverride", "true" });
    node.params.push_back ({ "debug.creationOverrideReason", overrideReason });
    node.params.push_back ({ "debug.creationBlockedReason", blockedReason });
}

GraphEdge makeEdge (const GraphContract& graph,
                    const std::vector<NodeSpec>& specs,
                    const std::string& from,
                    const std::string& to)
{
    const auto* sourceSpec = specForNode (graph, specs, nodeIdFromEndpoint (from));
    const auto* sourcePort = sourceSpec == nullptr ? nullptr : findPort (*sourceSpec, portIdFromEndpoint (from), "out");

    return { from,
             to,
             makeEdgeId (from, to),
             sourcePort == nullptr ? std::string {} : sourcePort->dataType,
             "continuous" };
}

void syncRuntimeFromEditor (GraphContract& graph)
{
    graph.runtimeGraph.nodes = graph.editorGraph.nodes;
    graph.runtimeGraph.edges = graph.editorGraph.edges;
}

CommandResult commitCommand (GraphSession& session,
                             const std::string& commandName,
                             const GraphSession::Snapshot& before)
{
    syncRuntimeFromEditor (session.graph);
    session.dirty = true;

    const auto after = snapshotOf (session);
    session.undoStack.push_back ({ commandName, before, after });
    session.redoStack.clear();
    session.commandLog.push_back (commandName);
    return { true, commandName };
}

bool hasEdge (const std::vector<GraphEdge>& edges, const std::string& from, const std::string& to)
{
    return std::any_of (edges.begin(), edges.end(), [&] (const auto& edge) {
        return edge.from == from && edge.to == to;
    });
}

bool eraseEdgeById (std::vector<GraphEdge>& edges, const std::string& edgeId)
{
    const auto originalSize = edges.size();
    edges.erase (std::remove_if (edges.begin(), edges.end(), [&] (const auto& edge) {
        return edge.id == edgeId;
    }), edges.end());
    return edges.size() != originalSize;
}

bool edgeTouchesNode (const GraphEdge& edge, const std::string& nodeId)
{
    return nodeIdFromEndpoint (edge.from) == nodeId || nodeIdFromEndpoint (edge.to) == nodeId;
}

void eraseIncidentEdges (std::vector<GraphEdge>& edges, const std::string& nodeId)
{
    edges.erase (std::remove_if (edges.begin(), edges.end(), [&] (const auto& edge) {
        return edgeTouchesNode (edge, nodeId);
    }), edges.end());
}

void eraseSelectionValue (std::vector<std::string>& values, const std::string& value)
{
    values.erase (std::remove (values.begin(), values.end(), value), values.end());
}

double distanceSquared (CanvasPoint a, CanvasPoint b)
{
    const auto dx = a.x - b.x;
    const auto dy = a.y - b.y;
    return dx * dx + dy * dy;
}

double distanceToSegmentSquared (CanvasPoint point, CanvasPoint a, CanvasPoint b)
{
    const auto vx = b.x - a.x;
    const auto vy = b.y - a.y;
    const auto lengthSquared = vx * vx + vy * vy;

    if (lengthSquared <= 0.000001)
        return distanceSquared (point, a);

    const auto t = std::clamp (((point.x - a.x) * vx + (point.y - a.y) * vy) / lengthSquared, 0.0, 1.0);
    return distanceSquared (point, { a.x + vx * t, a.y + vy * t });
}

bool pointInNodeBody (CanvasPoint point, const GraphNode& node, const NodeSpec* spec)
{
    return canvasPointInNodeBody ({ point.x, point.y }, node, spec);
}

CanvasPoint portCenterForIndex (const GraphNode& node, const NodeSpec* spec, const std::string& direction, size_t index)
{
    const auto point = canvasPortCenterForIndex (node, spec, direction, index);
    return { point.x, point.y };
}

PortCenterResult portCenterInternal (const GraphContract& graph,
                                     const std::vector<NodeSpec>& specs,
                                     const std::string& endpoint)
{
    const auto nodeId = nodeIdFromEndpoint (endpoint);
    const auto portId = portIdFromEndpoint (endpoint);
    const auto* node = findEditorNode (graph, nodeId);
    const auto* spec = node == nullptr ? nullptr : findNodeSpec (specs, node->type);

    if (node == nullptr || spec == nullptr)
        return {};

    for (size_t index = 0; index < spec->outputs.size(); ++index)
        if (spec->outputs[index].id == portId)
            return { true, portCenterForIndex (*node, spec, "out", index) };

    for (size_t index = 0; index < spec->inputs.size(); ++index)
        if (spec->inputs[index].id == portId)
            return { true, portCenterForIndex (*node, spec, "in", index) };

    return {};
}

bool containsNode (const GraphContract& graph, const std::string& id)
{
    return findEditorNode (graph, id) != nullptr;
}

void upsertParam (GraphNode& node, const std::string& paramId, const std::string& value)
{
    for (auto& param : node.params)
    {
        if (param.id == paramId)
        {
            param.value = value;
            return;
        }
    }

    node.params.push_back ({ paramId, value });
}

void upsertPortBinding (GraphNode& node,
                        const std::string& portId,
                        const std::string& bindingMode,
                        const std::string& value)
{
    for (auto& binding : node.portBindings)
    {
        if (binding.portId == portId)
        {
            binding.bindingMode = bindingMode;
            binding.value = value;
            return;
        }
    }

    node.portBindings.push_back ({ portId, bindingMode, value });
}

std::string readTextFile (const std::string& path)
{
    std::ifstream input (path);
    if (! input)
        return {};

    std::ostringstream text;
    text << input.rdbuf();
    return text.str();
}

bool fixtureContainsTrace (const std::string& fixtureText, const std::string& name)
{
    return fixtureText.find ("\"name\": \"" + name + "\"") != std::string::npos;
}

void expectCommandOk (BehaviorTraceReport& report, const std::string& traceName, const CommandResult& result)
{
    if (! result.ok)
        report.errors.push_back (traceName + ": " + result.message);
}

void expectBoolOk (BehaviorTraceReport& report, const std::string& traceName, bool ok, const std::string& message)
{
    if (! ok)
        report.errors.push_back (traceName + ": " + message);
}

void expectCommands (BehaviorTraceReport& report,
                     const std::string& traceName,
                     const GraphSession& session,
                     const std::vector<std::string>& expectedCommands)
{
    if (session.commandLog == expectedCommands)
        return;

    std::ostringstream message;
    message << traceName << ": command mismatch, got";
    for (const auto& command : session.commandLog)
        message << ' ' << command;

    message << " expected";
    for (const auto& command : expectedCommands)
        message << ' ' << command;

    report.errors.push_back (message.str());
}

void appendObservedCommands (BehaviorTraceReport& report, const GraphSession& session)
{
    for (const auto& command : session.commandLog)
        report.commandsObserved.push_back (command);
}
}

ScreenPoint canvasToScreen (const CanvasViewState& view, CanvasPoint point)
{
    return { point.x * view.scale + view.scrollX,
             point.y * view.scale + view.scrollY };
}

CanvasPoint screenToCanvas (const CanvasViewState& view, ScreenPoint point)
{
    if (! isValidScale (view.scale))
        return {};

    return { (point.x - view.scrollX) / view.scale,
             (point.y - view.scrollY) / view.scale };
}

CanvasViewState panView (const CanvasViewState& view, double deltaX, double deltaY)
{
    if (! std::isfinite (deltaX) || ! std::isfinite (deltaY))
        return view;

    auto next = view;
    next.scrollX += deltaX;
    next.scrollY += deltaY;
    return next;
}

CanvasViewState zoomViewAround (const CanvasViewState& view, double zoomFactor, ScreenPoint focus)
{
    if (! std::isfinite (zoomFactor) || zoomFactor <= 0.0)
        return view;

    auto next = view;
    next.scale = view.scale * zoomFactor;

    if (! isValidScale (next.scale))
        return view;

    const auto focusCanvas = screenToCanvas (view, focus);
    next.scrollX = focus.x - focusCanvas.x * next.scale;
    next.scrollY = focus.y - focusCanvas.y * next.scale;
    return next;
}

GraphSession makeGraphSession (GraphContract graph)
{
    syncRuntimeFromEditor (graph);
    return { graph };
}

CommandResult moveNode (GraphSession& session, const std::string& nodeId, double deltaX, double deltaY)
{
    if (! std::isfinite (deltaX) || ! std::isfinite (deltaY))
        return { false, "invalid move delta" };

    auto* node = findEditorNode (session.graph, nodeId);
    if (node == nullptr)
        return { false, "missing node: " + nodeId };

    const auto before = snapshotOf (session);
    node->position.x += deltaX;
    node->position.y += deltaY;
    session.selectedNodeIds = { nodeId };
    return commitCommand (session, "move_node", before);
}

CommandResult deleteNode (GraphSession& session, const std::string& nodeId)
{
    auto candidate = session.graph;
    const auto originalNodeCount = candidate.editorGraph.nodes.size();
    candidate.editorGraph.nodes.erase (std::remove_if (candidate.editorGraph.nodes.begin(),
                                                       candidate.editorGraph.nodes.end(),
                                                       [&] (const auto& node) {
                                                           return node.id == nodeId;
                                                       }),
                                       candidate.editorGraph.nodes.end());

    if (candidate.editorGraph.nodes.size() == originalNodeCount)
        return { false, "missing node: " + nodeId };

    eraseIncidentEdges (candidate.editorGraph.edges, nodeId);
    syncRuntimeFromEditor (candidate);

    const auto report = validateGraphInvariants (candidate, makeSeedNodeSpecs());
    if (! report.ok)
        return { false, report.errors.empty() ? "invalid graph" : report.errors.front() };

    const auto before = snapshotOf (session);
    session.graph = candidate;
    eraseSelectionValue (session.selectedNodeIds, nodeId);
    session.selectedEdgeIds.clear();
    return commitCommand (session, "delete_node", before);
}

CommandResult connectPorts (GraphSession& session, const std::string& from, const std::string& to)
{
    return connectPorts (session, makeSeedNodeSpecs(), from, to);
}

CommandResult connectPorts (GraphSession& session,
                            const std::vector<NodeSpec>& specs,
                            const std::string& from,
                            const std::string& to)
{
    if (hasEdge (session.graph.editorGraph.edges, from, to))
        return { false, "duplicate edge" };

    const auto edge = makeEdge (session.graph, specs, from, to);
    if (edge.dataType.empty())
        return { false, "missing source port" };

    auto candidate = session.graph;
    candidate.editorGraph.edges.push_back (edge);
    syncRuntimeFromEditor (candidate);

    const auto report = validateGraphInvariants (candidate, specs);
    if (! report.ok)
        return { false, report.errors.empty() ? "invalid graph" : report.errors.front() };

    const auto before = snapshotOf (session);
    session.graph = candidate;
    return commitCommand (session, "connect", before);
}

CommandResult disconnectEdge (GraphSession& session, const std::string& edgeId)
{
    const auto before = snapshotOf (session);

    if (! eraseEdgeById (session.graph.editorGraph.edges, edgeId))
        return { false, "missing edge: " + edgeId };

    eraseSelectionValue (session.selectedEdgeIds, edgeId);
    return commitCommand (session, "disconnect", before);
}

CommandResult createNode (GraphSession& session,
                          const std::vector<NodeSpec>& specs,
                          const std::string& nodeType,
                          const std::string& nodeId,
                          CanvasPoint position)
{
    if (containsNode (session.graph, nodeId))
        return { false, "duplicate node: " + nodeId };

    if (findNodeSpec (specs, nodeType) == nullptr)
        return { false, "unknown node type: " + nodeType };

    const auto before = snapshotOf (session);
    session.graph.editorGraph.nodes.push_back ({ nodeId, nodeType, {}, { position.x, position.y } });
    session.selectedNodeIds = { nodeId };
    return commitCommand (session, "create_node", before);
}

CommandResult createNode (GraphSession& session, const std::string& nodeType, const std::string& nodeId, CanvasPoint position)
{
    return createNode (session, makeSeedNodeSpecs(), nodeType, nodeId, position);
}

CommandResult createNode (GraphSession& session,
                          const std::vector<NodeSpec>& specs,
                          const std::vector<NodeCreationGate>& creationGates,
                          const std::string& nodeType,
                          const std::string& nodeId,
                          CanvasPoint position)
{
    const auto gate = checkCreationGate (creationGates, nodeType);
    if (! gate.ok)
        return gate;

    return createNode (session, specs, nodeType, nodeId, position);
}

CommandResult createNodeWithDebugOverride (GraphSession& session,
                                           const std::vector<NodeSpec>& specs,
                                           const std::vector<NodeCreationGate>& creationGates,
                                           const std::string& nodeType,
                                           const std::string& nodeId,
                                           CanvasPoint position,
                                           const std::string& overrideReason)
{
    const auto gate = checkDebugOverrideGate (creationGates, nodeType, overrideReason);
    if (! gate.ok)
        return gate;

    if (containsNode (session.graph, nodeId))
        return { false, "duplicate node: " + nodeId };

    if (findNodeSpec (specs, nodeType) == nullptr)
        return { false, "unknown node type: " + nodeType };

    const auto before = snapshotOf (session);
    GraphNode node { nodeId, nodeType, {}, { position.x, position.y } };
    appendDebugOverrideParams (node, overrideReason, gate.message);
    session.graph.editorGraph.nodes.push_back (std::move (node));
    session.selectedNodeIds = { nodeId };
    return commitCommand (session, "create_node_debug_override", before);
}

CommandResult createNodeAndConnect (GraphSession& session,
                                    const std::string& sourceEndpoint,
                                    const std::string& nodeType,
                                    const std::string& nodeId,
                                    CanvasPoint position)
{
    return createNodeAndConnect (session, makeSeedNodeSpecs(), sourceEndpoint, nodeType, nodeId, position);
}

CommandResult createNodeAndConnect (GraphSession& session,
                                    const std::vector<NodeSpec>& specs,
                                    const std::string& sourceEndpoint,
                                    const std::string& nodeType,
                                    const std::string& nodeId,
                                    CanvasPoint position)
{
    if (containsNode (session.graph, nodeId))
        return { false, "duplicate node: " + nodeId };

    const auto* spec = findNodeSpec (specs, nodeType);
    if (spec == nullptr)
        return { false, "unknown node type: " + nodeType };

    if (spec->inputs.empty())
        return { false, "new node has no input" };

    auto candidate = session.graph;
    candidate.editorGraph.nodes.push_back ({ nodeId, nodeType, {}, { position.x, position.y } });
    const auto targetEndpoint = nodeId + "." + spec->inputs.front().id;
    candidate.editorGraph.edges.push_back (makeEdge (candidate, specs, sourceEndpoint, targetEndpoint));
    syncRuntimeFromEditor (candidate);

    const auto report = validateGraphInvariants (candidate, specs);
    if (! report.ok)
        return { false, report.errors.empty() ? "invalid graph" : report.errors.front() };

    const auto before = snapshotOf (session);
    session.graph = candidate;
    session.selectedNodeIds = { nodeId };
    return commitCommand (session, "create_node+connect", before);
}

CommandResult createNodeAndConnect (GraphSession& session,
                                    const std::vector<NodeSpec>& specs,
                                    const std::vector<NodeCreationGate>& creationGates,
                                    const std::string& sourceEndpoint,
                                    const std::string& nodeType,
                                    const std::string& nodeId,
                                    CanvasPoint position)
{
    const auto gate = checkCreationGate (creationGates, nodeType);
    if (! gate.ok)
        return gate;

    return createNodeAndConnect (session, specs, sourceEndpoint, nodeType, nodeId, position);
}

CommandResult createNodeAndConnectWithDebugOverride (GraphSession& session,
                                                     const std::vector<NodeSpec>& specs,
                                                     const std::vector<NodeCreationGate>& creationGates,
                                                     const std::string& sourceEndpoint,
                                                     const std::string& nodeType,
                                                     const std::string& nodeId,
                                                     CanvasPoint position,
                                                     const std::string& overrideReason)
{
    const auto gate = checkDebugOverrideGate (creationGates, nodeType, overrideReason);
    if (! gate.ok)
        return gate;

    if (containsNode (session.graph, nodeId))
        return { false, "duplicate node: " + nodeId };

    const auto* spec = findNodeSpec (specs, nodeType);
    if (spec == nullptr)
        return { false, "unknown node type: " + nodeType };

    if (spec->inputs.empty())
        return { false, "new node has no input" };

    auto candidate = session.graph;
    GraphNode node { nodeId, nodeType, {}, { position.x, position.y } };
    appendDebugOverrideParams (node, overrideReason, gate.message);
    candidate.editorGraph.nodes.push_back (std::move (node));
    const auto targetEndpoint = nodeId + "." + spec->inputs.front().id;
    candidate.editorGraph.edges.push_back (makeEdge (candidate, specs, sourceEndpoint, targetEndpoint));
    syncRuntimeFromEditor (candidate);

    const auto report = validateGraphInvariants (candidate, specs);
    if (! report.ok)
        return { false, report.errors.empty() ? "invalid graph" : report.errors.front() };

    const auto before = snapshotOf (session);
    session.graph = candidate;
    session.selectedNodeIds = { nodeId };
    return commitCommand (session, "create_node+connect_debug_override", before);
}

CommandResult enterPatch (GraphSession& session, const std::string& nodeId)
{
    auto* node = findEditorNode (session.graph, nodeId);
    if (node == nullptr)
        return { false, "missing node: " + nodeId };

    if (node->type.rfind ("compound.", 0) != 0)
        return { false, "node is not compound: " + nodeId };

    const auto before = snapshotOf (session);
    session.currentPatchPath.push_back (nodeId);
    session.selectedNodeIds.clear();
    return commitCommand (session, "enter_patch", before);
}

CommandResult exitPatch (GraphSession& session)
{
    if (session.currentPatchPath.empty())
        return { false, "already at root patch" };

    const auto before = snapshotOf (session);
    const auto exited = session.currentPatchPath.back();
    session.currentPatchPath.pop_back();
    session.selectedNodeIds = { exited };
    return commitCommand (session, "exit_patch", before);
}

CommandResult setCollapsed (GraphSession& session, const std::string& nodeId, bool collapsed)
{
    auto* node = findEditorNode (session.graph, nodeId);
    if (node == nullptr)
        return { false, "missing node: " + nodeId };

    const auto before = snapshotOf (session);
    node->collapsed = collapsed;
    return commitCommand (session, collapsed ? "collapse_compound" : "expand_compound", before);
}

CommandResult storeExpandedPatchLayout (GraphSession& session,
                                        const std::string& parentNodeId,
                                        const GraphContract& expandedGraph)
{
    auto candidate = session.graph;

    if (! storeCompoundPatchInteractionLayout (candidate, parentNodeId, expandedGraph))
        return { false, "missing compound node: " + parentNodeId };

    const auto before = snapshotOf (session);
    session.graph = candidate;
    return commitCommand (session, "store_expanded_patch_layout", before);
}

CommandResult setParam (GraphSession& session,
                        const std::string& nodeId,
                        const std::string& paramId,
                        const std::string& value)
{
    auto* node = findEditorNode (session.graph, nodeId);
    if (node == nullptr)
        return { false, "missing node: " + nodeId };

    const auto before = snapshotOf (session);
    upsertParam (*node, paramId, value);
    return commitCommand (session, "set_param", before);
}

CommandResult setPortBinding (GraphSession& session,
                              const std::string& nodeId,
                              const std::string& portId,
                              const std::string& bindingMode,
                              const std::string& value)
{
    auto* node = findEditorNode (session.graph, nodeId);
    if (node == nullptr)
        return { false, "missing node: " + nodeId };

    const auto before = snapshotOf (session);
    upsertPortBinding (*node, portId, bindingMode, value);
    return commitCommand (session, "set_port_binding", before);
}

bool undo (GraphSession& session)
{
    if (session.undoStack.empty())
        return false;

    const auto record = session.undoStack.back();
    session.undoStack.pop_back();
    restoreSnapshot (session, record.before);
    session.redoStack.push_back (record);
    session.commandLog.push_back ("undo:" + record.commandName);
    return true;
}

bool redo (GraphSession& session)
{
    if (session.redoStack.empty())
        return false;

    const auto record = session.redoStack.back();
    session.redoStack.pop_back();
    restoreSnapshot (session, record.after);
    session.undoStack.push_back (record);
    session.commandLog.push_back ("redo:" + record.commandName);
    return true;
}

GraphInvariantReport validateGraphInvariants (const GraphContract& graph, const std::vector<NodeSpec>& specs)
{
    GraphInvariantReport report;
    report.ok = true;

    std::set<std::string> nodeIds;
    for (const auto& node : graph.editorGraph.nodes)
    {
        if (! nodeIds.insert (node.id).second)
            report.errors.push_back ("duplicate node id: " + node.id);

        if (findNodeSpec (specs, node.type) == nullptr)
            report.errors.push_back ("unknown node type: " + node.type);
    }

    std::set<std::string> edgeIds;
    std::map<std::string, int> targetCounts;
    for (const auto& edge : graph.editorGraph.edges)
    {
        if (edge.id.empty() || ! edgeIds.insert (edge.id).second)
            report.errors.push_back ("invalid edge id: " + edge.id);

        const auto sourceNodeId = nodeIdFromEndpoint (edge.from);
        const auto targetNodeId = nodeIdFromEndpoint (edge.to);
        const auto sourcePortId = portIdFromEndpoint (edge.from);
        const auto targetPortId = portIdFromEndpoint (edge.to);

        const auto* sourceSpec = specForNode (graph, specs, sourceNodeId);
        const auto* targetSpec = specForNode (graph, specs, targetNodeId);

        if (sourceSpec == nullptr)
        {
            report.errors.push_back ("missing source node: " + sourceNodeId);
            continue;
        }

        if (targetSpec == nullptr)
        {
            report.errors.push_back ("missing target node: " + targetNodeId);
            continue;
        }

        const auto* sourcePort = findPort (*sourceSpec, sourcePortId, "out");
        const auto* targetPort = findPort (*targetSpec, targetPortId, "in");

        if (sourcePort == nullptr)
            report.errors.push_back ("missing output port: " + edge.from);

        if (targetPort == nullptr)
            report.errors.push_back ("missing input port: " + edge.to);

        if (sourcePort != nullptr && targetPort != nullptr && sourcePort->dataType != targetPort->dataType)
            report.errors.push_back ("port type mismatch: " + edge.from + " -> " + edge.to);

        if (++targetCounts[edge.to] > 1)
            report.errors.push_back ("input cardinality exceeded: " + edge.to);
    }

    report.ok = report.errors.empty();
    return report;
}

HitTestResult hitTestGraph (const GraphContract& graph,
                            const std::vector<NodeSpec>& specs,
                            const CanvasViewState& view,
                            ScreenPoint screenPoint)
{
    const auto point = screenToCanvas (view, screenPoint);
    constexpr double portRadiusSquared = 8.0 * 8.0;
    constexpr double edgeHitDistanceSquared = 24.0 * 24.0;

    for (const auto& node : graph.editorGraph.nodes)
    {
        const auto* spec = findNodeSpec (specs, node.type);
        if (spec == nullptr)
            continue;

        for (size_t index = 0; index < spec->outputs.size(); ++index)
        {
            const auto center = portCenterForIndex (node, spec, "out", index);
            if (distanceSquared (point, center) <= portRadiusSquared)
                return { HitTestKind::outputPort, node.id, node.id + "." + spec->outputs[index].id, {} };
        }

        for (size_t index = 0; index < spec->inputs.size(); ++index)
        {
            const auto center = portCenterForIndex (node, spec, "in", index);
            if (distanceSquared (point, center) <= portRadiusSquared)
                return { HitTestKind::inputPort, node.id, node.id + "." + spec->inputs[index].id, {} };
        }
    }

    for (const auto& edge : graph.editorGraph.edges)
    {
        const auto from = portCenterInternal (graph, specs, edge.from);
        const auto to = portCenterInternal (graph, specs, edge.to);
        if (from.ok && to.ok && distanceToSegmentSquared (point, from.point, to.point) <= edgeHitDistanceSquared)
            return { HitTestKind::edge, {}, {}, edge.id };
    }

    for (const auto& node : graph.editorGraph.nodes)
        if (pointInNodeBody (point, node, specForNode (graph, specs, node.id)))
            return { HitTestKind::nodeBody, node.id, {}, {} };

    return {};
}

PortCenterResult portCenter (const GraphContract& graph,
                             const std::vector<NodeSpec>& specs,
                             const std::string& endpoint)
{
    return portCenterInternal (graph, specs, endpoint);
}

std::string markSavedAndCommitted (GraphSession& session)
{
    session.dirty = false;
    session.commandLog.push_back ("save_work:saved-and-committed");
    return "saved-and-committed";
}

std::string serializeInteractionState (const GraphSession& session)
{
    std::ostringstream out;
    out << "interaction-state-v1\n";
    out << "dirty\t" << (session.dirty ? "1" : "0") << "\n";

    for (const auto& pathItem : session.currentPatchPath)
        out << "path\t" << pathItem << "\n";

    for (const auto& node : session.graph.editorGraph.nodes)
    {
        out << "node\t" << node.id << "\t" << node.type << "\t"
            << node.position.x << "\t" << node.position.y << "\t"
            << (node.collapsed ? "1" : "0") << "\n";

        for (const auto& param : node.params)
            out << "param\t" << node.id << "\t" << param.id << "\t" << param.value << "\n";

        for (const auto& binding : node.portBindings)
            out << "binding\t" << node.id << "\t" << binding.portId << "\t"
                << binding.bindingMode << "\t" << binding.value << "\n";
    }

    for (const auto& edge : session.graph.editorGraph.edges)
        out << "edge\t" << edge.id << "\t" << edge.from << "\t" << edge.to << "\t"
            << edge.dataType << "\t" << edge.streamKind << "\n";

    return out.str();
}

GraphSession deserializeInteractionState (const std::string& encoded)
{
    GraphContract graph;
    graph.version = 1;
    GraphSession session;

    std::istringstream in (encoded);
    std::string line;
    std::getline (in, line);

    while (std::getline (in, line))
    {
        std::vector<std::string> fields;
        std::string field;
        std::istringstream row (line);
        while (std::getline (row, field, '\t'))
            fields.push_back (field);

        if (fields.empty())
            continue;

        if (fields[0] == "dirty" && fields.size() >= 2)
        {
            session.dirty = fields[1] == "1";
        }
        else if (fields[0] == "path" && fields.size() >= 2)
        {
            session.currentPatchPath.push_back (fields[1]);
        }
        else if (fields[0] == "node" && fields.size() >= 6)
        {
            graph.editorGraph.nodes.push_back ({ fields[1],
                                                 fields[2],
                                                 {},
                                                 { std::stod (fields[3]), std::stod (fields[4]) },
                                                 fields[5] == "1" });
        }
        else if (fields[0] == "param" && fields.size() >= 4)
        {
            if (auto* node = findEditorNode (graph, fields[1]))
                node->params.push_back ({ fields[2], fields[3] });
        }
        else if (fields[0] == "binding" && fields.size() >= 5)
        {
            if (auto* node = findEditorNode (graph, fields[1]))
                node->portBindings.push_back ({ fields[2], fields[3], fields[4] });
        }
        else if (fields[0] == "edge" && fields.size() >= 6)
        {
            graph.editorGraph.edges.push_back ({ fields[2], fields[3], fields[1], fields[4], fields[5] });
        }
    }

    syncRuntimeFromEditor (graph);
    session.graph = graph;
    return session;
}

BehaviorTraceReport runBehaviorTraceFixture (const std::string& path)
{
    BehaviorTraceReport report;
    const auto fixtureText = readTextFile (path);

    if (fixtureText.empty())
    {
        report.errors.push_back ("missing behavior fixture: " + path);
        return report;
    }

    const auto runTrace = [&] (const std::string& name,
                               const std::vector<std::string>& expectedCommands,
                               auto run) {
        if (! fixtureContainsTrace (fixtureText, name))
        {
            report.errors.push_back ("missing trace in fixture: " + name);
            return;
        }

        auto session = makeGraphSession (makeDefaultShaderOutputGraph());
        run (name, session);
        expectCommands (report, name, session, expectedCommands);
        appendObservedCommands (report, session);
        ++report.tracesRun;
    };

    runTrace ("move shader node", { "move_node" }, [&] (const auto& name, auto& session) {
        expectCommandOk (report, name, moveNode (session, "shader1", 16.0, 8.0));
    });

    runTrace ("connect shader to output", { "disconnect", "connect" }, [&] (const auto& name, auto& session) {
        expectCommandOk (report, name, disconnectEdge (session, "edge.shader1.output.out1.input"));
        expectCommandOk (report, name, connectPorts (session, "shader1.output", "out1.input"));
    });

    runTrace ("delete selected edge", { "disconnect", "undo:disconnect" }, [&] (const auto& name, auto& session) {
        expectCommandOk (report, name, disconnectEdge (session, "edge.shader1.output.out1.input"));
        expectBoolOk (report, name, undo (session), "undo failed");
    });

    runTrace ("delete selected node", { "delete_node", "undo:delete_node" }, [&] (const auto& name, auto& session) {
        session.selectedNodeIds = { "shader1" };
        expectCommandOk (report, name, deleteNode (session, "shader1"));
        expectBoolOk (report, name, undo (session), "undo failed");
    });

    runTrace ("create output node from dragged port", { "create_node+connect" }, [&] (const auto& name, auto& session) {
        expectCommandOk (report,
                         name,
                         createNodeAndConnect (session,
                                               "shader1.output",
                                               "output.preview",
                                               "out2",
                                               { 520.0, 160.0 }));
    });

    runTrace ("compound enter exit collapse",
              { "create_node", "enter_patch", "exit_patch", "collapse_compound" },
              [&] (const auto& name, auto& session) {
                  expectCommandOk (report, name, createNode (session, "compound.loudness", "loud1", { 160.0, 260.0 }));
                  expectCommandOk (report, name, enterPatch (session, "loud1"));
                  expectCommandOk (report, name, exitPatch (session));
                  expectCommandOk (report, name, setCollapsed (session, "loud1", true));
              });

    runTrace ("compound collapsed drag expanded roundtrip",
              { "create_node", "collapse_compound", "move_node", "enter_patch", "save_work:saved-and-committed" },
              [&] (const auto& name, auto& session) {
                  expectCommandOk (report, name, createNode (session, "compound.loudness", "loud1", { 180.0, 260.0 }));
                  expectCommandOk (report, name, setCollapsed (session, "loud1", true));
                  expectCommandOk (report, name, moveNode (session, "loud1", 96.0, 32.0));
                  expectCommandOk (report, name, enterPatch (session, "loud1"));
                  markSavedAndCommitted (session);

                  const auto restored = deserializeInteractionState (serializeInteractionState (session));
                  expectBoolOk (report,
                                name,
                                restored.currentPatchPath.size() == 1 && restored.currentPatchPath.front() == "loud1",
                                "patch path did not roundtrip");

                  const auto* restoredNode = findEditorNode (restored.graph, "loud1");
                  expectBoolOk (report,
                                name,
                                restoredNode != nullptr && restoredNode->collapsed,
                                "collapsed compound did not roundtrip");
              });

    runTrace ("compound public ports persist undo",
              { "create_node",
                "create_node",
                "create_node",
                "connect",
                "connect",
                "save_work:saved-and-committed",
                "disconnect",
                "undo:disconnect",
                "redo:disconnect" },
              [&] (const auto& name, auto& session) {
                  const auto inputEdgeFrom = std::string { "live_audio.channels" };
                  const auto inputEdgeTo = std::string { "loud1.audio.in" };
                  const auto outputEdgeFrom = std::string { "loud1.out" };
                  const auto outputEdgeTo = std::string { "midi1.value" };
                  const auto outputEdgeId = makeEdgeId (outputEdgeFrom, outputEdgeTo);

                  expectCommandOk (report, name, createNode (session, "audio.input", "live_audio", { 80.0, 260.0 }));
                  expectCommandOk (report, name, createNode (session, "compound.loudness", "loud1", { 280.0, 260.0 }));
                  expectCommandOk (report, name, createNode (session, "io.midi.cc_out", "midi1", { 520.0, 280.0 }));
                  expectCommandOk (report, name, connectPorts (session, inputEdgeFrom, inputEdgeTo));
                  expectCommandOk (report, name, connectPorts (session, outputEdgeFrom, outputEdgeTo));
                  markSavedAndCommitted (session);

                  const auto restored = deserializeInteractionState (serializeInteractionState (session));
                  expectBoolOk (report,
                                name,
                                hasEdge (restored.graph.editorGraph.edges, inputEdgeFrom, inputEdgeTo)
                                    && hasEdge (restored.graph.runtimeGraph.edges, inputEdgeFrom, inputEdgeTo),
                                "public input edge did not roundtrip through editor/runtime graphs");
                  expectBoolOk (report,
                                name,
                                hasEdge (restored.graph.editorGraph.edges, outputEdgeFrom, outputEdgeTo)
                                    && hasEdge (restored.graph.runtimeGraph.edges, outputEdgeFrom, outputEdgeTo),
                                "public output edge did not roundtrip through editor/runtime graphs");

                  expectCommandOk (report, name, disconnectEdge (session, outputEdgeId));
                  expectBoolOk (report,
                                name,
                                ! hasEdge (session.graph.editorGraph.edges, outputEdgeFrom, outputEdgeTo),
                                "disconnect did not remove public output edge");
                  expectBoolOk (report, name, undo (session), "undo failed");
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, outputEdgeFrom, outputEdgeTo),
                                "undo did not restore public output edge");
                  expectBoolOk (report, name, redo (session), "redo failed");
                  expectBoolOk (report,
                                name,
                                ! hasEdge (session.graph.editorGraph.edges, outputEdgeFrom, outputEdgeTo),
                                "redo did not remove public output edge");
              });

    runTrace ("inspector param and binding", { "set_param", "set_port_binding" }, [&] (const auto& name, auto& session) {
        expectCommandOk (report, name, setParam (session, "shader1", "fragmentSource", "void main(){}"));
        expectCommandOk (report,
                         name,
                         setPortBinding (session, "shader1", "output", "connected", "out1.input"));
    });

    runTrace ("undo redo save",
              { "move_node", "undo:move_node", "redo:move_node", "save_work:saved-and-committed" },
              [&] (const auto& name, auto& session) {
                  expectCommandOk (report, name, moveNode (session, "shader1", 4.0, 4.0));
                  expectBoolOk (report, name, undo (session), "undo failed");
                  expectBoolOk (report, name, redo (session), "redo failed");
                  markSavedAndCommitted (session);
              });

    report.ok = report.errors.empty();
    return report;
}
}
