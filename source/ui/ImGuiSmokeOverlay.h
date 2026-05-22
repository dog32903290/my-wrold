#pragma once

#include "InteractionContract.h"

#include <string>
#include <vector>

namespace myworld
{
struct CompoundPatchSpec;
struct NodeSpec;

class ImGuiSmokeOverlay
{
public:
    void initialise();
    void shutdown();
    void beginFrame (int width, int height, float scale, float deltaSeconds);
    void drawSmokePanel (const std::vector<NodeSpec>& nodeSpecs,
                         const CompoundPatchSpec& loudnessCompound,
                         const std::string& shaderStatus);
    void render();
    bool wantsMouse() const;
    void setMousePosition (float x, float y);
    void setMouseButton (int buttonIndex, bool isDown);
    void addMouseWheel (float deltaY);

private:
    void drawInteractionCanvas (const std::vector<NodeSpec>& nodeSpecs);
    void drawInteractionControls();
    void drawCreateNodePopup (const std::vector<NodeSpec>& nodeSpecs);
    void drawInspectorPanel (const std::vector<NodeSpec>& nodeSpecs);
    void drawTracePanel();
    void runInteractionCommand (const std::string& label, CommandResult result);
    void runInteractionCommand (const std::string& label, bool result);

    bool initialised = false;
    float smokeValue = 0.35f;
    bool loudnessExpanded = false;
    GraphSession interactionSession = makeGraphSession (makeDefaultShaderOutputGraph());
    std::string draggingNodeId;
    std::string draggingConnectionEndpoint;
    std::string pendingCreateSourceEndpoint;
    CanvasPoint dragCanvasDelta;
    CanvasPoint draggingConnectionPoint;
    CanvasPoint pendingCreatePosition;
    ScreenPoint previousPanDrag;
    std::string savedInteractionState;
    BehaviorTraceReport lastTraceReport;
    std::string lastInteractionMessage = "ready";
    bool interactionViewReady = false;
    bool panningCanvas = false;
    bool hasTraceReport = false;
};
}
