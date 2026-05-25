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

const PortSpec* portForEndpoint (const GraphContract& graph,
                                 const std::vector<NodeSpec>& specs,
                                 const std::string& endpoint,
                                 const std::string& direction)
{
    const auto* spec = specForNode (graph, specs, nodeIdFromEndpoint (endpoint));
    return spec == nullptr ? nullptr : findPort (*spec, portIdFromEndpoint (endpoint), direction);
}

std::string outputDataTypeForEndpointInternal (const GraphContract& graph,
                                               const std::vector<NodeSpec>& specs,
                                               const std::string& endpoint)
{
    const auto* port = portForEndpoint (graph, specs, endpoint, "out");
    return port == nullptr ? std::string {} : port->dataType;
}

std::string inputDataTypeForEndpointInternal (const GraphContract& graph,
                                              const std::vector<NodeSpec>& specs,
                                              const std::string& endpoint)
{
    const auto* port = portForEndpoint (graph, specs, endpoint, "in");
    return port == nullptr ? std::string {} : port->dataType;
}

const PortSpec* firstInputMatching (const NodeSpec& spec, const std::string& dataType)
{
    const auto found = std::find_if (spec.inputs.begin(), spec.inputs.end(), [&] (const auto& port) {
        return port.dataType == dataType;
    });

    return found == spec.inputs.end() ? nullptr : &*found;
}

const PortSpec* firstOutputMatching (const NodeSpec& spec, const std::string& dataType)
{
    const auto found = std::find_if (spec.outputs.begin(), spec.outputs.end(), [&] (const auto& port) {
        return port.dataType == dataType;
    });

    return found == spec.outputs.end() ? nullptr : &*found;
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

const GraphEdge* findEdgeById (const std::vector<GraphEdge>& edges, const std::string& edgeId)
{
    const auto found = std::find_if (edges.begin(), edges.end(), [&] (const auto& edge) {
        return edge.id == edgeId;
    });

    return found == edges.end() ? nullptr : &*found;
}

GraphEdge* findEdgeToEndpoint (std::vector<GraphEdge>& edges, const std::string& endpoint)
{
    const auto found = std::find_if (edges.begin(), edges.end(), [&] (const auto& edge) {
        return edge.to == endpoint;
    });

    return found == edges.end() ? nullptr : &*found;
}

bool eraseEdgeToEndpoint (std::vector<GraphEdge>& edges, const std::string& endpoint)
{
    const auto originalSize = edges.size();
    edges.erase (std::remove_if (edges.begin(), edges.end(), [&] (const auto& edge) {
        return edge.to == endpoint;
    }), edges.end());
    return edges.size() != originalSize;
}

size_t inputIndexForPort (const NodeSpec& spec, const std::string& portId)
{
    for (size_t index = 0; index < spec.inputs.size(); ++index)
        if (spec.inputs[index].id == portId)
            return index;

    return spec.inputs.size();
}

std::string inputEndpointForIndex (const std::string& nodeId, const NodeSpec& spec, size_t index)
{
    return nodeId + "." + spec.inputs[index].id;
}

size_t lastInputIndexForDataType (const NodeSpec& spec, const std::string& dataType)
{
    for (auto index = spec.inputs.size(); index > 0; --index)
        if (spec.inputs[index - 1].dataType == dataType)
            return index - 1;

    return spec.inputs.size();
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

CommandResult reconnectInputEnd (GraphSession& session,
                                 const std::string& edgeId,
                                 const std::string& newSourceEndpoint)
{
    return reconnectInputEnd (session, makeSeedNodeSpecs(), edgeId, newSourceEndpoint);
}

CommandResult reconnectInputEnd (GraphSession& session,
                                 const std::vector<NodeSpec>& specs,
                                 const std::string& edgeId,
                                 const std::string& newSourceEndpoint)
{
    const auto* existingEdge = findEdgeById (session.graph.editorGraph.edges, edgeId);
    if (existingEdge == nullptr)
        return { false, "missing edge: " + edgeId };

    const auto oldEdge = *existingEdge;
    auto candidate = session.graph;
    eraseEdgeById (candidate.editorGraph.edges, edgeId);

    if (hasEdge (candidate.editorGraph.edges, newSourceEndpoint, oldEdge.to))
        return { false, "duplicate edge" };

    const auto newEdge = makeEdge (candidate, specs, newSourceEndpoint, oldEdge.to);
    if (newEdge.dataType.empty())
        return { false, "missing source port" };

    candidate.editorGraph.edges.push_back (newEdge);
    syncRuntimeFromEditor (candidate);

    const auto report = validateGraphInvariants (candidate, specs);
    if (! report.ok)
        return { false, report.errors.empty() ? "invalid graph" : report.errors.front() };

    const auto before = snapshotOf (session);
    session.graph = candidate;
    session.selectedEdgeIds = { newEdge.id };
    return commitCommand (session, "reconnect", before);
}

CommandResult reconnectOutputBeginning (GraphSession& session,
                                        const std::string& edgeId,
                                        const std::string& newTargetEndpoint)
{
    return reconnectOutputBeginning (session, makeSeedNodeSpecs(), edgeId, newTargetEndpoint);
}

CommandResult reconnectOutputBeginning (GraphSession& session,
                                        const std::vector<NodeSpec>& specs,
                                        const std::string& edgeId,
                                        const std::string& newTargetEndpoint)
{
    const auto* existingEdge = findEdgeById (session.graph.editorGraph.edges, edgeId);
    if (existingEdge == nullptr)
        return { false, "missing edge: " + edgeId };

    const auto oldEdge = *existingEdge;
    auto candidate = session.graph;
    eraseEdgeById (candidate.editorGraph.edges, edgeId);

    if (hasEdge (candidate.editorGraph.edges, oldEdge.from, newTargetEndpoint))
        return { false, "duplicate edge" };

    const auto newEdge = makeEdge (candidate, specs, oldEdge.from, newTargetEndpoint);
    if (newEdge.dataType.empty())
        return { false, "missing source port" };

    candidate.editorGraph.edges.push_back (newEdge);
    syncRuntimeFromEditor (candidate);

    const auto report = validateGraphInvariants (candidate, specs);
    if (! report.ok)
        return { false, report.errors.empty() ? "invalid graph" : report.errors.front() };

    const auto before = snapshotOf (session);
    session.graph = candidate;
    session.selectedEdgeIds = { newEdge.id };
    return commitCommand (session, "reconnect", before);
}

CommandResult connectHiddenInput (GraphSession& session,
                                  const std::string& sourceEndpoint,
                                  const std::string& targetNodeId,
                                  const std::string& targetInputPortId)
{
    return connectHiddenInput (session, makeSeedNodeSpecs(), sourceEndpoint, targetNodeId, targetInputPortId);
}

CommandResult connectHiddenInput (GraphSession& session,
                                  const std::vector<NodeSpec>& specs,
                                  const std::string& sourceEndpoint,
                                  const std::string& targetNodeId,
                                  const std::string& targetInputPortId)
{
    const auto targetEndpoint = targetNodeId + "." + targetInputPortId;
    if (hasEdge (session.graph.editorGraph.edges, sourceEndpoint, targetEndpoint))
        return { false, "duplicate edge" };

    const auto edge = makeEdge (session.graph, specs, sourceEndpoint, targetEndpoint);
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
    session.selectedEdgeIds = { edge.id };
    return commitCommand (session, "connect_hidden_input", before);
}

CommandResult insertInputEdge (GraphSession& session,
                               const std::string& sourceEndpoint,
                               const std::string& targetEndpoint,
                               InputInsertMode mode)
{
    return insertInputEdge (session, makeSeedNodeSpecs(), sourceEndpoint, targetEndpoint, mode);
}

CommandResult insertInputEdge (GraphSession& session,
                               const std::vector<NodeSpec>& specs,
                               const std::string& sourceEndpoint,
                               const std::string& targetEndpoint,
                               InputInsertMode mode)
{
    const auto targetNodeId = nodeIdFromEndpoint (targetEndpoint);
    const auto targetPortId = portIdFromEndpoint (targetEndpoint);
    const auto* targetSpec = specForNode (session.graph, specs, targetNodeId);
    if (targetSpec == nullptr)
        return { false, "missing target node: " + targetNodeId };

    const auto targetIndex = inputIndexForPort (*targetSpec, targetPortId);
    if (targetIndex >= targetSpec->inputs.size())
        return { false, "missing input port: " + targetEndpoint };

    const auto sourceDataType = outputDataTypeForEndpointInternal (session.graph, specs, sourceEndpoint);
    if (sourceDataType.empty())
        return { false, "missing source port" };

    if (targetSpec->inputs[targetIndex].dataType != sourceDataType)
        return { false, "port type mismatch: " + sourceEndpoint + " -> " + targetEndpoint };

    auto insertIndex = targetIndex;
    if (mode == InputInsertMode::after)
        ++insertIndex;

    const auto lastIndex = lastInputIndexForDataType (*targetSpec, sourceDataType);
    if (lastIndex >= targetSpec->inputs.size() || insertIndex > lastIndex)
        return { false, "no compatible input slot" };

    auto candidate = session.graph;
    const auto insertEndpoint = inputEndpointForIndex (targetNodeId, *targetSpec, insertIndex);

    if (mode == InputInsertMode::replace)
    {
        eraseEdgeToEndpoint (candidate.editorGraph.edges, insertEndpoint);
    }
    else
    {
        const auto lastEndpoint = inputEndpointForIndex (targetNodeId, *targetSpec, lastIndex);
        if (findEdgeToEndpoint (candidate.editorGraph.edges, lastEndpoint) != nullptr)
            return { false, "no free input slot" };

        for (auto index = lastIndex; index > insertIndex; --index)
        {
            const auto previousEndpoint = inputEndpointForIndex (targetNodeId, *targetSpec, index - 1);
            const auto nextEndpoint = inputEndpointForIndex (targetNodeId, *targetSpec, index);

            if (auto* shiftedEdge = findEdgeToEndpoint (candidate.editorGraph.edges, previousEndpoint))
            {
                shiftedEdge->to = nextEndpoint;
                shiftedEdge->id = makeEdgeId (shiftedEdge->from, shiftedEdge->to);
            }
        }
    }

    if (hasEdge (candidate.editorGraph.edges, sourceEndpoint, insertEndpoint))
        return { false, "duplicate edge" };

    candidate.editorGraph.edges.push_back (makeEdge (candidate, specs, sourceEndpoint, insertEndpoint));
    syncRuntimeFromEditor (candidate);

    const auto report = validateGraphInvariants (candidate, specs);
    if (! report.ok)
        return { false, report.errors.empty() ? "invalid graph" : report.errors.front() };

    const auto before = snapshotOf (session);
    session.graph = candidate;
    session.selectedEdgeIds = { makeEdgeId (sourceEndpoint, insertEndpoint) };
    return commitCommand (session, "multi_input_insert", before);
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

CommandResult splitEdgeWithNode (GraphSession& session,
                                 const std::string& edgeId,
                                 const std::string& nodeType,
                                 const std::string& nodeId,
                                 CanvasPoint position)
{
    return splitEdgeWithNode (session, makeSeedNodeSpecs(), edgeId, nodeType, nodeId, position);
}

CommandResult splitEdgeWithNode (GraphSession& session,
                                 const std::vector<NodeSpec>& specs,
                                 const std::string& edgeId,
                                 const std::string& nodeType,
                                 const std::string& nodeId,
                                 CanvasPoint position)
{
    if (containsNode (session.graph, nodeId))
        return { false, "duplicate node: " + nodeId };

    const auto* existingEdge = findEdgeById (session.graph.editorGraph.edges, edgeId);
    if (existingEdge == nullptr)
        return { false, "missing edge: " + edgeId };

    const auto* newSpec = findNodeSpec (specs, nodeType);
    if (newSpec == nullptr)
        return { false, "unknown node type: " + nodeType };

    const auto oldEdge = *existingEdge;
    const auto sourceDataType = outputDataTypeForEndpointInternal (session.graph, specs, oldEdge.from);
    const auto targetDataType = inputDataTypeForEndpointInternal (session.graph, specs, oldEdge.to);

    const auto* inputPort = firstInputMatching (*newSpec, sourceDataType);
    if (inputPort == nullptr)
        return { false, "new node has no compatible input" };

    const auto* outputPort = firstOutputMatching (*newSpec, targetDataType);
    if (outputPort == nullptr)
        return { false, "new node has no compatible output" };

    auto candidate = session.graph;
    eraseEdgeById (candidate.editorGraph.edges, edgeId);
    candidate.editorGraph.nodes.push_back ({ nodeId, nodeType, {}, { position.x, position.y } });

    const auto upstream = makeEdge (candidate, specs, oldEdge.from, nodeId + "." + inputPort->id);
    const auto downstream = makeEdge (candidate, specs, nodeId + "." + outputPort->id, oldEdge.to);
    if (upstream.dataType.empty() || downstream.dataType.empty())
        return { false, "missing source port" };

    candidate.editorGraph.edges.push_back (upstream);
    candidate.editorGraph.edges.push_back (downstream);
    syncRuntimeFromEditor (candidate);

    const auto report = validateGraphInvariants (candidate, specs);
    if (! report.ok)
        return { false, report.errors.empty() ? "invalid graph" : report.errors.front() };

    const auto before = snapshotOf (session);
    session.graph = candidate;
    session.selectedNodeIds = { nodeId };
    session.selectedEdgeIds.clear();
    return commitCommand (session, "split_edge_create_node", before);
}

CommandResult insertExistingNodeOnEdge (GraphSession& session,
                                        const std::string& edgeId,
                                        const std::string& nodeId)
{
    return insertExistingNodeOnEdge (session, makeSeedNodeSpecs(), edgeId, nodeId);
}

CommandResult insertExistingNodeOnEdge (GraphSession& session,
                                        const std::vector<NodeSpec>& specs,
                                        const std::string& edgeId,
                                        const std::string& nodeId)
{
    const auto* existingEdge = findEdgeById (session.graph.editorGraph.edges, edgeId);
    if (existingEdge == nullptr)
        return { false, "missing edge: " + edgeId };

    const auto* existingNode = findEditorNode (session.graph, nodeId);
    if (existingNode == nullptr)
        return { false, "missing node: " + nodeId };

    const auto* nodeSpec = findNodeSpec (specs, existingNode->type);
    if (nodeSpec == nullptr)
        return { false, "unknown node type: " + existingNode->type };

    const auto oldEdge = *existingEdge;
    const auto sourceDataType = outputDataTypeForEndpointInternal (session.graph, specs, oldEdge.from);
    const auto targetDataType = inputDataTypeForEndpointInternal (session.graph, specs, oldEdge.to);

    const auto* inputPort = firstInputMatching (*nodeSpec, sourceDataType);
    if (inputPort == nullptr)
        return { false, "existing node has no compatible input" };

    const auto* outputPort = firstOutputMatching (*nodeSpec, targetDataType);
    if (outputPort == nullptr)
        return { false, "existing node has no compatible output" };

    auto candidate = session.graph;
    eraseEdgeById (candidate.editorGraph.edges, edgeId);

    const auto upstream = makeEdge (candidate, specs, oldEdge.from, nodeId + "." + inputPort->id);
    const auto downstream = makeEdge (candidate, specs, nodeId + "." + outputPort->id, oldEdge.to);
    if (upstream.dataType.empty() || downstream.dataType.empty())
        return { false, "missing source port" };

    candidate.editorGraph.edges.push_back (upstream);
    candidate.editorGraph.edges.push_back (downstream);
    syncRuntimeFromEditor (candidate);

    const auto report = validateGraphInvariants (candidate, specs);
    if (! report.ok)
        return { false, report.errors.empty() ? "invalid graph" : report.errors.front() };

    const auto before = snapshotOf (session);
    session.graph = candidate;
    session.selectedNodeIds = { nodeId };
    session.selectedEdgeIds.clear();
    return commitCommand (session, "insert_node_on_edge", before);
}

CommandResult snapConnect (GraphSession& session, const std::string& from, const std::string& to)
{
    return snapConnect (session, makeSeedNodeSpecs(), from, to);
}

CommandResult snapConnect (GraphSession& session,
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
    session.selectedEdgeIds = { edge.id };
    return commitCommand (session, "snap_connect", before);
}

CommandResult unsnapDisconnect (GraphSession& session, const std::string& edgeId)
{
    const auto before = snapshotOf (session);

    if (! eraseEdgeById (session.graph.editorGraph.edges, edgeId))
        return { false, "missing edge: " + edgeId };

    eraseSelectionValue (session.selectedEdgeIds, edgeId);
    return commitCommand (session, "unsnap_disconnect", before);
}

CommandResult shakeDisconnectNode (GraphSession& session, const std::string& nodeId)
{
    return shakeDisconnectNode (session, makeSeedNodeSpecs(), nodeId);
}

CommandResult shakeDisconnectNode (GraphSession& session,
                                   const std::vector<NodeSpec>& specs,
                                   const std::string& nodeId)
{
    if (findEditorNode (session.graph, nodeId) == nullptr)
        return { false, "missing node: " + nodeId };

    auto candidate = session.graph;
    const auto originalEdgeCount = candidate.editorGraph.edges.size();
    eraseIncidentEdges (candidate.editorGraph.edges, nodeId);
    if (candidate.editorGraph.edges.size() == originalEdgeCount)
        return { false, "node has no incident edges: " + nodeId };

    syncRuntimeFromEditor (candidate);

    const auto report = validateGraphInvariants (candidate, specs);
    if (! report.ok)
        return { false, report.errors.empty() ? "invalid graph" : report.errors.front() };

    const auto before = snapshotOf (session);
    session.graph = candidate;
    session.selectedNodeIds = { nodeId };
    session.selectedEdgeIds.clear();
    return commitCommand (session, "shake_disconnect", before);
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

    runTrace ("reconnect input end",
              { "reconnect", "undo:reconnect", "redo:reconnect" },
              [&] (const auto& name, auto& session) {
                  session.graph.editorGraph.nodes.push_back ({ "shader2", "shader.fragment", {}, { 120.0, 260.0 } });
                  syncRuntimeFromEditor (session.graph);

                  expectCommandOk (report,
                                   name,
                                   reconnectInputEnd (session,
                                                      "edge.shader1.output.out1.input",
                                                      "shader2.output"));
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "shader2.output", "out1.input"),
                                "reconnect input did not create new source edge");
                  expectBoolOk (report, name, undo (session), "undo failed");
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "shader1.output", "out1.input"),
                                "undo did not restore old input edge");
                  expectBoolOk (report, name, redo (session), "redo failed");
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "shader2.output", "out1.input"),
                                "redo did not restore reconnected input edge");
              });

    runTrace ("reconnect output beginning",
              { "reconnect", "undo:reconnect", "redo:reconnect" },
              [&] (const auto& name, auto& session) {
                  session.graph.editorGraph.nodes.push_back ({ "out2", "output.preview", {}, { 560.0, 260.0 } });
                  syncRuntimeFromEditor (session.graph);

                  expectCommandOk (report,
                                   name,
                                   reconnectOutputBeginning (session,
                                                             "edge.shader1.output.out1.input",
                                                             "out2.input"));
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "shader1.output", "out2.input"),
                                "reconnect output did not create new target edge");
                  expectBoolOk (report, name, undo (session), "undo failed");
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "shader1.output", "out1.input"),
                                "undo did not restore old output edge");
                  expectBoolOk (report, name, redo (session), "redo failed");
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "shader1.output", "out2.input"),
                                "redo did not restore reconnected output edge");
              });

    runTrace ("split edge create operator",
              { "split_edge_create_node", "undo:split_edge_create_node", "redo:split_edge_create_node" },
              [&] (const auto& name, auto& session) {
                  const auto specs = makeSeedNodeSpecs();
                  session.graph.editorGraph.nodes.push_back ({ "loud1", "analyzer.loudness", {}, { 180.0, 360.0 } });
                  session.graph.editorGraph.nodes.push_back ({ "midi1", "io.midi.cc_out", {}, { 540.0, 360.0 } });
                  session.graph.editorGraph.edges.push_back (makeEdge (session.graph, specs, "loud1.out", "midi1.value"));
                  syncRuntimeFromEditor (session.graph);

                  expectCommandOk (report,
                                   name,
                                   splitEdgeWithNode (session,
                                                      "edge.loud1.out.midi1.value",
                                                      "signal.smoother",
                                                      "smooth1",
                                                      { 360.0, 360.0 }));
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "loud1.out", "smooth1.input")
                                    && hasEdge (session.graph.editorGraph.edges, "smooth1.out", "midi1.value"),
                                "split edge did not create both replacement edges");
                  expectBoolOk (report, name, undo (session), "undo failed");
                  expectBoolOk (report,
                                name,
                                findEditorNode (session.graph, "smooth1") == nullptr
                                    && hasEdge (session.graph.editorGraph.edges, "loud1.out", "midi1.value"),
                                "undo did not restore original split edge");
                  expectBoolOk (report, name, redo (session), "redo failed");
                  expectBoolOk (report,
                                name,
                                findEditorNode (session.graph, "smooth1") != nullptr
                                    && hasEdge (session.graph.editorGraph.edges, "loud1.out", "smooth1.input"),
                                "redo did not restore split node");
              });

    runTrace ("drop connection onto operator hidden input",
              { "connect_hidden_input", "undo:connect_hidden_input", "redo:connect_hidden_input" },
              [&] (const auto& name, auto& session) {
                  session.graph.editorGraph.nodes.push_back ({ "loud1", "analyzer.loudness", {}, { 180.0, 320.0 } });
                  session.graph.editorGraph.nodes.push_back ({ "loud_out", "analyzer.loudness_out", {}, { 420.0, 320.0 } });
                  syncRuntimeFromEditor (session.graph);

                  expectCommandOk (report, name, connectHiddenInput (session, "loud1.out", "loud_out", "rms"));
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "loud1.out", "loud_out.rms"),
                                "hidden input picker did not connect selected input");
                  expectBoolOk (report, name, undo (session), "undo failed");
                  expectBoolOk (report,
                                name,
                                ! hasEdge (session.graph.editorGraph.edges, "loud1.out", "loud_out.rms"),
                                "undo did not remove hidden input edge");
                  expectBoolOk (report, name, redo (session), "redo failed");
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "loud1.out", "loud_out.rms"),
                                "redo did not restore hidden input edge");
              });

    runTrace ("multi input insert before",
              { "multi_input_insert", "undo:multi_input_insert", "redo:multi_input_insert" },
              [&] (const auto& name, auto& session) {
                  auto specs = makeSeedNodeSpecs();
                  specs.push_back ({ "signal.mix3",
                                     "Mix 3",
                                     "signal",
                                     "combine",
                                     "audioAnalysis",
                                     "meter_scope",
                                     "docs/nodes/signal.mix3.md",
                                     1,
                                     { { "a", "A", "signal.float", "in" },
                                       { "b", "B", "signal.float", "in" },
                                       { "c", "C", "signal.float", "in" } },
                                     { { "out", "Out", "signal.float", "out" } },
                                     {} });
                  session.graph.editorGraph.nodes.push_back ({ "loud1", "analyzer.loudness", {}, { 100.0, 420.0 } });
                  session.graph.editorGraph.nodes.push_back ({ "smooth1", "signal.smoother", {}, { 100.0, 520.0 } });
                  session.graph.editorGraph.nodes.push_back ({ "gain1", "analyzer.analysis_gain", {}, { 100.0, 620.0 } });
                  session.graph.editorGraph.nodes.push_back ({ "mix1", "signal.mix3", {}, { 420.0, 520.0 } });
                  session.graph.editorGraph.edges.push_back (makeEdge (session.graph, specs, "loud1.out", "mix1.a"));
                  session.graph.editorGraph.edges.push_back (makeEdge (session.graph, specs, "smooth1.out", "mix1.b"));
                  syncRuntimeFromEditor (session.graph);

                  expectCommandOk (report,
                                   name,
                                   insertInputEdge (session,
                                                    specs,
                                                    "gain1.out",
                                                    "mix1.b",
                                                    InputInsertMode::before));
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "gain1.out", "mix1.b")
                                    && hasEdge (session.graph.editorGraph.edges, "smooth1.out", "mix1.c"),
                                "multi input insert did not shift edge order");
                  expectBoolOk (report, name, undo (session), "undo failed");
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "smooth1.out", "mix1.b"),
                                "undo did not restore shifted input");
                  expectBoolOk (report, name, redo (session), "redo failed");
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "gain1.out", "mix1.b")
                                    && hasEdge (session.graph.editorGraph.edges, "smooth1.out", "mix1.c"),
                                "redo did not restore multi input insert");
              });

    runTrace ("drag existing node onto edge insert",
              { "insert_node_on_edge", "undo:insert_node_on_edge", "redo:insert_node_on_edge" },
              [&] (const auto& name, auto& session) {
                  const auto specs = makeSeedNodeSpecs();
                  session.graph.editorGraph.nodes.push_back ({ "loud1", "analyzer.loudness", {}, { 180.0, 360.0 } });
                  session.graph.editorGraph.nodes.push_back ({ "midi1", "io.midi.cc_out", {}, { 540.0, 360.0 } });
                  session.graph.editorGraph.nodes.push_back ({ "smooth1", "signal.smoother", {}, { 360.0, 360.0 } });
                  session.graph.editorGraph.edges.push_back (makeEdge (session.graph, specs, "loud1.out", "midi1.value"));
                  syncRuntimeFromEditor (session.graph);

                  expectCommandOk (report,
                                   name,
                                   insertExistingNodeOnEdge (session, "edge.loud1.out.midi1.value", "smooth1"));
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "loud1.out", "smooth1.input")
                                    && hasEdge (session.graph.editorGraph.edges, "smooth1.out", "midi1.value"),
                                "existing node insert did not create replacement edges");
                  expectBoolOk (report, name, undo (session), "undo failed");
                  expectBoolOk (report,
                                name,
                                findEditorNode (session.graph, "smooth1") != nullptr
                                    && hasEdge (session.graph.editorGraph.edges, "loud1.out", "midi1.value"),
                                "undo did not restore original edge while keeping existing node");
                  expectBoolOk (report, name, redo (session), "redo failed");
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "loud1.out", "smooth1.input"),
                                "redo did not restore existing node insert");
              });

    runTrace ("snap connect unsnap disconnect",
              { "snap_connect", "unsnap_disconnect", "undo:unsnap_disconnect" },
              [&] (const auto& name, auto& session) {
                  session.graph.editorGraph.nodes.push_back ({ "audio1", "audio.input", {}, { 80.0, 520.0 } });
                  session.graph.editorGraph.nodes.push_back ({ "mono1", "audio.mono_mix", {}, { 300.0, 520.0 } });
                  syncRuntimeFromEditor (session.graph);

                  expectCommandOk (report, name, snapConnect (session, "audio1.channels", "mono1.input"));
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "audio1.channels", "mono1.input"),
                                "snap did not create edge");
                  expectCommandOk (report, name, unsnapDisconnect (session, "edge.audio1.channels.mono1.input"));
                  expectBoolOk (report,
                                name,
                                ! hasEdge (session.graph.editorGraph.edges, "audio1.channels", "mono1.input"),
                                "unsnap did not remove edge");
                  expectBoolOk (report, name, undo (session), "undo failed");
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "audio1.channels", "mono1.input"),
                                "undo unsnap did not restore edge");
              });

    runTrace ("shake disconnect dragged node edges",
              { "shake_disconnect", "undo:shake_disconnect", "redo:shake_disconnect" },
              [&] (const auto& name, auto& session) {
                  const auto specs = makeSeedNodeSpecs();
                  session.graph.editorGraph.nodes.push_back ({ "audio1", "audio.input", {}, { 80.0, 640.0 } });
                  session.graph.editorGraph.nodes.push_back ({ "mono1", "audio.mono_mix", {}, { 300.0, 640.0 } });
                  session.graph.editorGraph.nodes.push_back ({ "loud1", "analyzer.loudness", {}, { 520.0, 640.0 } });
                  session.graph.editorGraph.edges.push_back (makeEdge (session.graph, specs, "audio1.channels", "mono1.input"));
                  session.graph.editorGraph.edges.push_back (makeEdge (session.graph, specs, "mono1.mono", "loud1.input"));
                  syncRuntimeFromEditor (session.graph);

                  expectCommandOk (report, name, shakeDisconnectNode (session, "mono1"));
                  expectBoolOk (report,
                                name,
                                ! hasEdge (session.graph.editorGraph.edges, "audio1.channels", "mono1.input")
                                    && ! hasEdge (session.graph.editorGraph.edges, "mono1.mono", "loud1.input"),
                                "shake did not remove dragged node incident edges");
                  expectBoolOk (report, name, undo (session), "undo failed");
                  expectBoolOk (report,
                                name,
                                hasEdge (session.graph.editorGraph.edges, "audio1.channels", "mono1.input")
                                    && hasEdge (session.graph.editorGraph.edges, "mono1.mono", "loud1.input"),
                                "undo shake did not restore incident edges");
                  expectBoolOk (report, name, redo (session), "redo failed");
                  expectBoolOk (report,
                                name,
                                ! hasEdge (session.graph.editorGraph.edges, "audio1.channels", "mono1.input")
                                    && ! hasEdge (session.graph.editorGraph.edges, "mono1.mono", "loud1.input"),
                                "redo shake did not remove incident edges");
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
