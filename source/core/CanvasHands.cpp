#include "CanvasHands.h"

#include <cmath>
#include <sstream>

namespace myworld
{
namespace
{
const GraphNode* findNode (const GraphContract& graph, const std::string& nodeId)
{
    for (const auto& node : graph.editorGraph.nodes)
        if (node.id == nodeId)
            return &node;

    return nullptr;
}

std::string buttonName (CanvasHandButton button)
{
    switch (button)
    {
        case CanvasHandButton::left:   return "left";
        case CanvasHandButton::right:  return "right";
        case CanvasHandButton::middle: return "middle";
    }

    return "left";
}

void setButton (CanvasHandPointerState& pointer, CanvasHandButton button, bool down)
{
    switch (button)
    {
        case CanvasHandButton::left:   pointer.leftDown = down; break;
        case CanvasHandButton::right:  pointer.rightDown = down; break;
        case CanvasHandButton::middle: pointer.middleDown = down; break;
    }
}

std::string hitName (HitTestKind kind)
{
    switch (kind)
    {
        case HitTestKind::none:       return "none";
        case HitTestKind::nodeBody:   return "nodeBody";
        case HitTestKind::inputPort:  return "inputPort";
        case HitTestKind::outputPort: return "outputPort";
        case HitTestKind::edge:       return "edge";
    }

    return "none";
}

std::string numberText (double value)
{
    std::ostringstream out;
    out << value;
    return out.str();
}

bool insideViewport (ScreenPoint point, const CanvasHandViewport& viewport)
{
    return point.x >= 0.0 && point.y >= 0.0 && point.x <= viewport.width && point.y <= viewport.height;
}

void appendCommandDelta (CanvasHandReport& report, const GraphSession& session, size_t firstCommandIndex)
{
    report.commandLogDelta.clear();

    for (auto index = firstCommandIndex; index < session.commandLog.size(); ++index)
        report.commandLogDelta.push_back (session.commandLog[index]);
}

void appendInvariantErrors (CanvasHandReport& report, const GraphContract& graph, const std::vector<NodeSpec>& specs)
{
    const auto invariant = validateGraphInvariants (graph, specs);

    if (invariant.ok)
        return;

    for (const auto& error : invariant.errors)
        report.errors.push_back ("invariant: " + error);
}

void appendCommandResult (CanvasHandReport& report,
                          const std::string& actionName,
                          const CommandResult& result,
                          const GraphSession& session,
                          const std::vector<NodeSpec>& specs)
{
    if (! result.ok)
    {
        report.errors.push_back (actionName + ": " + result.message);
        return;
    }

    appendInvariantErrors (report, session.graph, specs);
}
}

CanvasHandTarget canvasHandNode (const std::string& nodeId)
{
    return { CanvasHandTargetKind::node, nodeId, {} };
}

CanvasHandTarget canvasHandPort (const std::string& endpoint)
{
    return { CanvasHandTargetKind::port, endpoint, {} };
}

CanvasHandTarget canvasHandPoint (CanvasPoint point)
{
    return { CanvasHandTargetKind::canvasPoint, {}, point };
}

CanvasHandTarget canvasHandViewportCenter()
{
    return { CanvasHandTargetKind::viewportCenter, {}, {} };
}

CanvasHandAction canvasHandMoveTo (CanvasHandTarget target, int steps)
{
    CanvasHandAction action;
    action.kind = CanvasHandActionKind::moveTo;
    action.target = target;
    action.steps = steps;
    return action;
}

CanvasHandAction canvasHandDown (CanvasHandButton button)
{
    CanvasHandAction action;
    action.kind = CanvasHandActionKind::down;
    action.button = button;
    return action;
}

CanvasHandAction canvasHandUp (CanvasHandButton button)
{
    CanvasHandAction action;
    action.kind = CanvasHandActionKind::up;
    action.button = button;
    return action;
}

CanvasHandAction canvasHandClick (CanvasHandTarget target, CanvasHandButton button)
{
    CanvasHandAction action;
    action.kind = CanvasHandActionKind::click;
    action.target = target;
    action.button = button;
    return action;
}

CanvasHandAction canvasHandDrag (CanvasHandTarget from, CanvasHandTarget to, CanvasHandButton button, int steps)
{
    CanvasHandAction action;
    action.kind = CanvasHandActionKind::drag;
    action.from = from;
    action.to = to;
    action.button = button;
    action.steps = steps;
    return action;
}

CanvasHandAction canvasHandWheel (CanvasHandTarget target, double deltaX, double deltaY)
{
    CanvasHandAction action;
    action.kind = CanvasHandActionKind::wheel;
    action.target = target;
    action.wheelDeltaX = deltaX;
    action.wheelDeltaY = deltaY;
    return action;
}

CanvasHandResolvedTarget resolveCanvasHandTarget (const GraphSession& session,
                                                  const std::vector<NodeSpec>& specs,
                                                  const CanvasHandViewport& viewport,
                                                  CanvasHandTarget target)
{
    CanvasHandResolvedTarget result;

    if (viewport.width <= 0.0 || viewport.height <= 0.0)
    {
        result.message = "invalid viewport";
        return result;
    }

    if (target.kind == CanvasHandTargetKind::viewportCenter)
    {
        result.screenPoint = { viewport.width * 0.5, viewport.height * 0.5 };
        result.canvasPoint = screenToCanvas (session.view, result.screenPoint);
    }
    else if (target.kind == CanvasHandTargetKind::canvasPoint)
    {
        result.canvasPoint = target.point;
        result.screenPoint = canvasToScreen (session.view, result.canvasPoint);
    }
    else if (target.kind == CanvasHandTargetKind::port)
    {
        const auto center = portCenter (session.graph, specs, target.id);
        if (! center.ok)
        {
            result.message = "missing port target: " + target.id;
            return result;
        }

        result.canvasPoint = center.point;
        result.screenPoint = canvasToScreen (session.view, result.canvasPoint);
    }
    else if (target.kind == CanvasHandTargetKind::node)
    {
        const auto* node = findNode (session.graph, target.id);
        if (node == nullptr)
        {
            result.message = "missing node target: " + target.id;
            return result;
        }

        result.canvasPoint = { node->position.x + 30.0, node->position.y + 20.0 };
        result.screenPoint = canvasToScreen (session.view, result.canvasPoint);
    }

    if (! insideViewport (result.screenPoint, viewport))
    {
        result.message = "target outside viewport";
        return result;
    }

    result.hit = hitTestGraph (session.graph, specs, session.view, result.screenPoint);

    if (target.kind == CanvasHandTargetKind::port && result.hit.endpoint != target.id)
    {
        result.message = "port target hit mismatch: " + target.id;
        return result;
    }

    if (target.kind == CanvasHandTargetKind::node
        && (result.hit.kind != HitTestKind::nodeBody || result.hit.nodeId != target.id))
    {
        result.message = "node target hit mismatch: " + target.id;
        return result;
    }

    result.ok = true;
    result.message = "ok";
    return result;
}

CanvasHandReport runCanvasHandTrace (GraphSession& session,
                                     const std::vector<NodeSpec>& specs,
                                     const CanvasHandViewport& viewport,
                                     const std::vector<CanvasHandAction>& actions)
{
    CanvasHandReport report;
    const auto firstCommandIndex = session.commandLog.size();

    for (const auto& action : actions)
    {
        if (action.kind == CanvasHandActionKind::moveTo)
        {
            const auto target = resolveCanvasHandTarget (session, specs, viewport, action.target);
            if (! target.ok)
            {
                report.errors.push_back ("move: " + target.message);
                continue;
            }

            report.pointer.position = target.screenPoint;
            report.pointer.hover = target.hit;
            report.stateLog.push_back ("move:" + hitName (target.hit.kind));
        }
        else if (action.kind == CanvasHandActionKind::down)
        {
            setButton (report.pointer, action.button, true);
            report.pointer.activeGesture = buttonName (action.button);
            report.stateLog.push_back ("down:" + buttonName (action.button));
        }
        else if (action.kind == CanvasHandActionKind::up)
        {
            setButton (report.pointer, action.button, false);
            report.pointer.activeGesture.clear();
            report.stateLog.push_back ("up:" + buttonName (action.button));
        }
        else if (action.kind == CanvasHandActionKind::click)
        {
            const auto target = resolveCanvasHandTarget (session, specs, viewport, action.target);
            if (! target.ok)
            {
                report.errors.push_back ("click: " + target.message);
                continue;
            }

            report.pointer.position = target.screenPoint;
            report.pointer.hover = target.hit;
            setButton (report.pointer, action.button, true);

            if (action.button == CanvasHandButton::left)
            {
                session.selectedNodeIds.clear();
                session.selectedEdgeIds.clear();

                if (target.hit.kind == HitTestKind::nodeBody)
                    session.selectedNodeIds = { target.hit.nodeId };
                else if (target.hit.kind == HitTestKind::edge)
                    session.selectedEdgeIds = { target.hit.edgeId };
            }

            setButton (report.pointer, action.button, false);
            report.stateLog.push_back ("click:" + buttonName (action.button) + ":" + hitName (target.hit.kind));
        }
        else if (action.kind == CanvasHandActionKind::wheel)
        {
            const auto target = resolveCanvasHandTarget (session, specs, viewport, action.target);
            if (! target.ok)
            {
                report.errors.push_back ("wheel: " + target.message);
                continue;
            }

            report.pointer.position = target.screenPoint;
            report.pointer.hover = target.hit;

            if (std::abs (action.wheelDeltaY) > 0.000001)
                session.view = zoomViewAround (session.view,
                                               action.wheelDeltaY < 0.0 ? 1.10 : 0.90,
                                               target.screenPoint);

            report.stateLog.push_back ("wheel:" + numberText (action.wheelDeltaX) + "," + numberText (action.wheelDeltaY));
        }
        else if (action.kind == CanvasHandActionKind::drag)
        {
            const auto from = resolveCanvasHandTarget (session, specs, viewport, action.from);
            const auto to = resolveCanvasHandTarget (session, specs, viewport, action.to);

            if (! from.ok)
            {
                report.errors.push_back ("drag from: " + from.message);
                continue;
            }

            if (! to.ok)
            {
                report.errors.push_back ("drag to: " + to.message);
                continue;
            }

            setButton (report.pointer, action.button, true);
            report.pointer.position = to.screenPoint;
            report.pointer.hover = to.hit;
            report.pointer.activeGesture = "drag:" + buttonName (action.button);

            if (action.button == CanvasHandButton::left
                && from.hit.kind == HitTestKind::nodeBody)
            {
                appendCommandResult (report,
                                     "drag node",
                                     moveNode (session,
                                               from.hit.nodeId,
                                               to.canvasPoint.x - from.canvasPoint.x,
                                               to.canvasPoint.y - from.canvasPoint.y),
                                     session,
                                     specs);
            }
            else if (action.button == CanvasHandButton::left
                     && from.hit.kind == HitTestKind::outputPort
                     && to.hit.kind == HitTestKind::inputPort)
            {
                appendCommandResult (report,
                                     "drag connection",
                                     connectPorts (session, from.hit.endpoint, to.hit.endpoint),
                                     session,
                                     specs);
            }
            else if (action.button == CanvasHandButton::left
                     && from.hit.kind == HitTestKind::none
                     && to.hit.kind == HitTestKind::none)
            {
                session.view = panView (session.view,
                                        to.screenPoint.x - from.screenPoint.x,
                                        to.screenPoint.y - from.screenPoint.y);
            }

            setButton (report.pointer, action.button, false);
            report.pointer.activeGesture.clear();
            report.stateLog.push_back ("drag:" + buttonName (action.button) + ":"
                                      + hitName (from.hit.kind) + "->" + hitName (to.hit.kind));
        }
        else
        {
            report.stateLog.push_back ("pause");
        }
    }

    appendCommandDelta (report, session, firstCommandIndex);
    report.ok = report.errors.empty();
    return report;
}
}
