#pragma once

#include "InteractionContract.h"

#include <string>
#include <vector>

namespace myworld
{
enum class CanvasHandButton
{
    left,
    right,
    middle
};

enum class CanvasHandTargetKind
{
    node,
    port,
    canvasPoint,
    viewportCenter
};

struct CanvasHandViewport
{
    double width = 0.0;
    double height = 0.0;
};

struct CanvasHandTarget
{
    CanvasHandTargetKind kind = CanvasHandTargetKind::canvasPoint;
    std::string id;
    CanvasPoint point;
};

struct CanvasHandResolvedTarget
{
    bool ok = false;
    std::string message;
    CanvasPoint canvasPoint;
    ScreenPoint screenPoint;
    HitTestResult hit;
};

struct CanvasHandPointerState
{
    ScreenPoint position;
    HitTestResult hover;
    bool leftDown = false;
    bool rightDown = false;
    bool middleDown = false;
    std::string activeGesture;
};

enum class CanvasHandActionKind
{
    moveTo,
    down,
    up,
    click,
    drag,
    wheel,
    pause
};

struct CanvasHandAction
{
    CanvasHandActionKind kind = CanvasHandActionKind::pause;
    CanvasHandTarget target;
    CanvasHandTarget from;
    CanvasHandTarget to;
    CanvasHandButton button = CanvasHandButton::left;
    int steps = 1;
    double wheelDeltaX = 0.0;
    double wheelDeltaY = 0.0;
};

struct CanvasHandReport
{
    bool ok = false;
    std::vector<std::string> errors;
    std::vector<std::string> stateLog;
    std::vector<std::string> commandLogDelta;
    CanvasHandPointerState pointer;
};

CanvasHandTarget canvasHandNode (const std::string& nodeId);
CanvasHandTarget canvasHandPort (const std::string& endpoint);
CanvasHandTarget canvasHandPoint (CanvasPoint point);
CanvasHandTarget canvasHandViewportCenter();

CanvasHandAction canvasHandMoveTo (CanvasHandTarget target, int steps = 1);
CanvasHandAction canvasHandDown (CanvasHandButton button);
CanvasHandAction canvasHandUp (CanvasHandButton button);
CanvasHandAction canvasHandClick (CanvasHandTarget target, CanvasHandButton button = CanvasHandButton::left);
CanvasHandAction canvasHandDrag (CanvasHandTarget from,
                                 CanvasHandTarget to,
                                 CanvasHandButton button = CanvasHandButton::left,
                                 int steps = 4);
CanvasHandAction canvasHandWheel (CanvasHandTarget target, double deltaX, double deltaY);

CanvasHandResolvedTarget resolveCanvasHandTarget (const GraphSession& session,
                                                  const std::vector<NodeSpec>& specs,
                                                  const CanvasHandViewport& viewport,
                                                  CanvasHandTarget target);
CanvasHandReport runCanvasHandTrace (GraphSession& session,
                                     const std::vector<NodeSpec>& specs,
                                     const CanvasHandViewport& viewport,
                                     const std::vector<CanvasHandAction>& actions);
}
