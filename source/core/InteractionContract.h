#pragma once

#include "GraphContract.h"
#include "NodeSpec.h"
#include "OutputViewState.h"
#include "TimelineState.h"
#include "VariationState.h"

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

struct NodeCreationGate
{
    std::string nodeType;
    bool canCreate = true;
    std::string reason;
};

enum class InputInsertMode
{
    before,
    after,
    replace
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

struct CollaborationLogEntry
{
    std::string actor;
    std::string commandId;
    std::string operation;
    std::string intent;
    std::string status;
    std::string result;
    std::string proofEvidence;
    std::string error;
};

struct GraphSession
{
    GraphContract graph;
    CanvasViewState view;
    OutputViewState outputView;
    TimelineState timeline;
    VariationLibrary variations;
    std::vector<std::string> selectedNodeIds;
    std::vector<std::string> selectedEdgeIds;
    std::vector<std::string> currentPatchPath;
    std::vector<std::string> commandLog;
    std::vector<CollaborationLogEntry> collaborationLog;
    bool dirty = false;

    struct Snapshot
    {
        GraphContract graph;
        CanvasViewState view;
        OutputViewState outputView;
        TimelineState timeline;
        VariationLibrary variations;
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
CommandResult connectPorts (GraphSession& session,
                            const std::vector<NodeSpec>& specs,
                            const std::string& from,
                            const std::string& to);
CommandResult disconnectEdge (GraphSession& session, const std::string& edgeId);
CommandResult reconnectInputEnd (GraphSession& session,
                                 const std::string& edgeId,
                                 const std::string& newSourceEndpoint);
CommandResult reconnectInputEnd (GraphSession& session,
                                 const std::vector<NodeSpec>& specs,
                                 const std::string& edgeId,
                                 const std::string& newSourceEndpoint);
CommandResult reconnectOutputBeginning (GraphSession& session,
                                        const std::string& edgeId,
                                        const std::string& newTargetEndpoint);
CommandResult reconnectOutputBeginning (GraphSession& session,
                                        const std::vector<NodeSpec>& specs,
                                        const std::string& edgeId,
                                        const std::string& newTargetEndpoint);
CommandResult connectHiddenInput (GraphSession& session,
                                  const std::string& sourceEndpoint,
                                  const std::string& targetNodeId,
                                  const std::string& targetInputPortId);
CommandResult connectHiddenInput (GraphSession& session,
                                  const std::vector<NodeSpec>& specs,
                                  const std::string& sourceEndpoint,
                                  const std::string& targetNodeId,
                                  const std::string& targetInputPortId);
CommandResult insertInputEdge (GraphSession& session,
                               const std::string& sourceEndpoint,
                               const std::string& targetEndpoint,
                               InputInsertMode mode);
CommandResult insertInputEdge (GraphSession& session,
                               const std::vector<NodeSpec>& specs,
                               const std::string& sourceEndpoint,
                               const std::string& targetEndpoint,
                               InputInsertMode mode);
CommandResult createNode (GraphSession& session, const std::string& nodeType, const std::string& nodeId, CanvasPoint position);
CommandResult createNode (GraphSession& session,
                          const std::vector<NodeSpec>& specs,
                          const std::string& nodeType,
                          const std::string& nodeId,
                          CanvasPoint position);
CommandResult createNode (GraphSession& session,
                          const std::vector<NodeSpec>& specs,
                          const std::vector<NodeCreationGate>& creationGates,
                          const std::string& nodeType,
                          const std::string& nodeId,
                          CanvasPoint position);
CommandResult createNodeWithDebugOverride (GraphSession& session,
                                           const std::vector<NodeSpec>& specs,
                                           const std::vector<NodeCreationGate>& creationGates,
                                           const std::string& nodeType,
                                           const std::string& nodeId,
                                           CanvasPoint position,
                                           const std::string& overrideReason);
CommandResult createNodeAndConnect (GraphSession& session,
                                    const std::string& sourceEndpoint,
                                    const std::string& nodeType,
                                    const std::string& nodeId,
                                    CanvasPoint position);
CommandResult createNodeAndConnect (GraphSession& session,
                                    const std::vector<NodeSpec>& specs,
                                    const std::string& sourceEndpoint,
                                    const std::string& nodeType,
                                    const std::string& nodeId,
                                    CanvasPoint position);
CommandResult createNodeAndConnectWithDebugOverride (GraphSession& session,
                                                     const std::vector<NodeSpec>& specs,
                                                     const std::vector<NodeCreationGate>& creationGates,
                                                     const std::string& sourceEndpoint,
                                                     const std::string& nodeType,
                                                     const std::string& nodeId,
                                                     CanvasPoint position,
                                                     const std::string& overrideReason);
CommandResult createNodeAndConnect (GraphSession& session,
                                    const std::vector<NodeSpec>& specs,
                                    const std::vector<NodeCreationGate>& creationGates,
                                    const std::string& sourceEndpoint,
                                    const std::string& nodeType,
                                    const std::string& nodeId,
                                    CanvasPoint position);
CommandResult splitEdgeWithNode (GraphSession& session,
                                 const std::string& edgeId,
                                 const std::string& nodeType,
                                 const std::string& nodeId,
                                 CanvasPoint position);
CommandResult splitEdgeWithNode (GraphSession& session,
                                 const std::vector<NodeSpec>& specs,
                                 const std::string& edgeId,
                                 const std::string& nodeType,
                                 const std::string& nodeId,
                                 CanvasPoint position);
CommandResult insertExistingNodeOnEdge (GraphSession& session,
                                        const std::string& edgeId,
                                        const std::string& nodeId);
CommandResult insertExistingNodeOnEdge (GraphSession& session,
                                        const std::vector<NodeSpec>& specs,
                                        const std::string& edgeId,
                                        const std::string& nodeId);
CommandResult snapConnect (GraphSession& session, const std::string& from, const std::string& to);
CommandResult snapConnect (GraphSession& session,
                           const std::vector<NodeSpec>& specs,
                           const std::string& from,
                           const std::string& to);
CommandResult unsnapDisconnect (GraphSession& session, const std::string& edgeId);
CommandResult shakeDisconnectNode (GraphSession& session, const std::string& nodeId);
CommandResult shakeDisconnectNode (GraphSession& session,
                                   const std::vector<NodeSpec>& specs,
                                   const std::string& nodeId);
CommandResult enterPatch (GraphSession& session, const std::string& nodeId);
CommandResult exitPatch (GraphSession& session);
CommandResult setCollapsed (GraphSession& session, const std::string& nodeId, bool collapsed);
CommandResult storeExpandedPatchLayout (GraphSession& session,
                                        const std::string& parentNodeId,
                                        const GraphContract& expandedGraph);
CommandResult setParam (GraphSession& session, const std::string& nodeId, const std::string& paramId, const std::string& value);
CommandResult resetParam (GraphSession& session, const std::string& nodeId, const std::string& paramId);
CommandResult setPortBinding (GraphSession& session,
                              const std::string& nodeId,
                              const std::string& portId,
                              const std::string& bindingMode,
                              const std::string& value);
CommandResult resetPortBinding (GraphSession& session, const std::string& nodeId, const std::string& portId);
CommandResult playTimeline (GraphSession& session, int direction, double playbackRate);
CommandResult pauseTimeline (GraphSession& session);
CommandResult stopTimeline (GraphSession& session);
CommandResult stepTimelineFrames (GraphSession& session, double frameDelta);
CommandResult setTimelineTempo (GraphSession& session, double bpm);
CommandResult setTimelineFramesPerSecond (GraphSession& session, double framesPerSecond);
CommandResult setTimelinePositionBars (GraphSession& session, double positionBars);
CommandResult setTimelineLoop (GraphSession& session, double startBars, double endBars, bool looping);
CommandResult createPreset (GraphSession& session,
                            const std::string& nodeId,
                            const NodeSpec& spec,
                            const std::string& presetId,
                            const std::string& title,
                            const VariationCaptureOptions& options = {});
CommandResult applyPreset (GraphSession& session, const std::string& presetId);
CommandResult createSnapshot (GraphSession& session,
                              const std::string& snapshotId,
                              const std::string& title,
                              const std::vector<std::string>& enabledNodeIds);
CommandResult applySnapshot (GraphSession& session, const std::string& snapshotId);
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
