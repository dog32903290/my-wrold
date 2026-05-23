#pragma once

#include "InteractionContract.h"
#include "RuntimeRegistry.h"

#include <functional>
#include <string>
#include <vector>

namespace myworld
{
struct CompoundPatchSpec;
struct NodeSpec;

class ImGuiSmokeOverlay
{
public:
    using ShaderSourceCallback = std::function<void (const std::string&)>;

    ImGuiSmokeOverlay();

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
    void setShaderSource (std::string source);
    void setRuntimeOpDiagnostics (std::vector<RuntimeOpModuleDiagnostic> diagnostics);
    void requestDeleteSelection();

    ShaderSourceCallback onShaderSourceSubmitted;

private:
    void drawInteractionCanvas (const std::vector<NodeSpec>& nodeSpecs, float canvasWidth, float canvasHeight);
    void drawInteractionControls();
    void drawCreateNodePopup (const std::vector<NodeSpec>& nodeSpecs);
    void drawWorkspaceNodeBrowser (const std::vector<NodeSpec>& nodeSpecs);
    void drawInspectorPanel (const std::vector<NodeSpec>& nodeSpecs, const std::string& shaderStatus);
    void drawTracePanel();
    void runInteractionCommand (const std::string& label, CommandResult result);
    void runInteractionCommand (const std::string& label, bool result);

    bool initialised = false;
    float smokeValue = 0.35f;
    bool loudnessExpanded = false;
    GraphSession interactionSession;
    std::string draggingNodeId;
    std::string draggingConnectionEndpoint;
    std::string pendingCreateSourceEndpoint;
    std::string shaderSourceDraft;
    std::string shaderSourceDraftNodeId;
    std::string nodeBrowserFilter;
    std::vector<RuntimeOpModuleDiagnostic> runtimeOpDiagnostics;
    CanvasPoint dragCanvasDelta;
    CanvasPoint draggingConnectionPoint;
    CanvasPoint pendingCreatePosition;
    ScreenPoint nodeBrowserScreenPosition;
    ScreenPoint previousPanDrag;
    std::string savedInteractionState;
    BehaviorTraceReport lastTraceReport;
    std::string lastInteractionMessage = "ready";
    bool interactionViewReady = false;
    bool panningCanvas = false;
    bool hasTraceReport = false;
    bool deleteSelectionRequested = false;
};
}
