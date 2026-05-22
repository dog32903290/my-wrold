#pragma once

#include "GraphContract.h"
#include "NodeSpec.h"

#include <string>
#include <vector>

namespace myworld
{
struct CanvasPoint
{
    double x = 0.0;
    double y = 0.0;
};

struct ScreenPoint
{
    double x = 0.0;
    double y = 0.0;
};

struct CanvasViewState
{
    double scale = 1.0;
    double scrollX = 0.0;
    double scrollY = 0.0;
};

ScreenPoint canvasToScreen (const CanvasViewState& view, CanvasPoint point);
CanvasPoint screenToCanvas (const CanvasViewState& view, ScreenPoint point);
CanvasViewState panView (const CanvasViewState& view, double deltaX, double deltaY);
CanvasViewState zoomViewAround (const CanvasViewState& view, double zoomFactor, ScreenPoint focus);

struct CommandResult
{
    bool ok = false;
    std::string message;
};

struct GraphInvariantReport
{
    bool ok = false;
    std::vector<std::string> errors;
};

struct BehaviorTraceReport
{
    bool ok = false;
    int tracesRun = 0;
    std::vector<std::string> commandsObserved;
    std::vector<std::string> errors;
};

struct GraphSession
{
    GraphContract graph;
    CanvasViewState view;
    std::vector<std::string> selectedNodeIds;
    std::vector<std::string> selectedEdgeIds;
    std::vector<std::string> currentPatchPath;
    std::vector<std::string> commandLog;
    bool dirty = false;

    struct Snapshot
    {
        GraphContract graph;
        CanvasViewState view;
        std::vector<std::string> selectedNodeIds;
        std::vector<std::string> selectedEdgeIds;
        std::vector<std::string> currentPatchPath;
        bool dirty = false;
    };

    struct HistoryRecord
    {
        std::string commandName;
        Snapshot before;
        Snapshot after;
    };

    std::vector<HistoryRecord> undoStack;
    std::vector<HistoryRecord> redoStack;
};

GraphSession makeGraphSession (GraphContract graph);
CommandResult moveNode (GraphSession& session, const std::string& nodeId, double deltaX, double deltaY);
CommandResult deleteNode (GraphSession& session, const std::string& nodeId);
CommandResult connectPorts (GraphSession& session, const std::string& from, const std::string& to);
CommandResult disconnectEdge (GraphSession& session, const std::string& edgeId);
CommandResult createNode (GraphSession& session, const std::string& nodeType, const std::string& nodeId, CanvasPoint position);
CommandResult createNodeAndConnect (GraphSession& session,
                                    const std::string& sourceEndpoint,
                                    const std::string& nodeType,
                                    const std::string& nodeId,
                                    CanvasPoint position);
CommandResult enterPatch (GraphSession& session, const std::string& nodeId);
CommandResult exitPatch (GraphSession& session);
CommandResult setCollapsed (GraphSession& session, const std::string& nodeId, bool collapsed);
CommandResult setParam (GraphSession& session, const std::string& nodeId, const std::string& paramId, const std::string& value);
CommandResult setPortBinding (GraphSession& session,
                              const std::string& nodeId,
                              const std::string& portId,
                              const std::string& bindingMode,
                              const std::string& value);
bool undo (GraphSession& session);
bool redo (GraphSession& session);
GraphInvariantReport validateGraphInvariants (const GraphContract& graph, const std::vector<NodeSpec>& specs);

enum class HitTestKind
{
    none,
    nodeBody,
    inputPort,
    outputPort,
    edge
};

struct HitTestResult
{
    HitTestKind kind = HitTestKind::none;
    std::string nodeId;
    std::string endpoint;
    std::string edgeId;
};

struct PortCenterResult
{
    bool ok = false;
    CanvasPoint point;
};

HitTestResult hitTestGraph (const GraphContract& graph,
                            const std::vector<NodeSpec>& specs,
                            const CanvasViewState& view,
                            ScreenPoint screenPoint);
PortCenterResult portCenter (const GraphContract& graph,
                             const std::vector<NodeSpec>& specs,
                             const std::string& endpoint);
std::string markSavedAndCommitted (GraphSession& session);
std::string serializeInteractionState (const GraphSession& session);
GraphSession deserializeInteractionState (const std::string& encoded);
BehaviorTraceReport runBehaviorTraceFixture (const std::string& path);
}
