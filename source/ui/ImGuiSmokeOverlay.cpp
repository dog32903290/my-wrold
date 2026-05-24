#include "ImGuiSmokeOverlay.h"

#include "CanvasGeometry.h"
#include "CompoundPatch.h"
#include "GraphEndpoint.h"
#include "ImGuiSmokeOverlayHelpers.h"
#include "NodeSpec.h"
#include "NodeSpecQueries.h"
#include "Tooll3SkinContract.h"

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>

#include <algorithm>
#include <cmath>
#include <cfloat>

namespace myworld
{
using namespace imgui_overlay;

ImGuiSmokeOverlay::ImGuiSmokeOverlay()
    : interactionSession (makeDefaultOverlaySession()),
      shaderSourceDraft (defaultFragmentShader()),
      shaderSourceDraftNodeId ("shader1")
{
}

void ImGuiSmokeOverlay::initialise()
{
    if (initialised)
        return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2 (0.0f, 0.0f);
    style.FramePadding = ImVec2 (7.0f, 4.0f);
    style.ItemSpacing = ImVec2 (1.0f, 1.0f);
    style.ItemInnerSpacing = ImVec2 (3.0f, 2.0f);
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.TabRounding = 2.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;

    auto* colours = style.Colors;
    colours[ImGuiCol_Text] = ImVec4 (1.0f, 1.0f, 1.0f, 0.85f);
    colours[ImGuiCol_TextDisabled] = ImVec4 (0.50f, 0.50f, 0.50f, 1.0f);
    colours[ImGuiCol_WindowBg] = ImVec4 (0.0f, 0.0f, 0.0f, 0.0f);
    colours[ImGuiCol_ChildBg] = ImVec4 (0.0f, 0.0f, 0.0f, 0.0f);
    colours[ImGuiCol_Button] = ImVec4 (0.16f, 0.16f, 0.16f, 0.80f);
    colours[ImGuiCol_ButtonHovered] = ImVec4 (0.17f, 0.25f, 0.31f, 1.0f);
    colours[ImGuiCol_ButtonActive] = ImVec4 (0.27f, 0.57f, 1.0f, 1.0f);
    colours[ImGuiCol_FrameBg] = ImVec4 (0.13f, 0.13f, 0.13f, 0.80f);
    colours[ImGuiCol_FrameBgHovered] = ImVec4 (0.38f, 0.38f, 0.38f, 0.40f);
    colours[ImGuiCol_FrameBgActive] = ImVec4 (0.0f, 0.55f, 0.80f, 1.0f);
    colours[ImGuiCol_Separator] = ImVec4 (0.0f, 0.0f, 0.0f, 1.0f);
    ImGui_ImplOpenGL3_Init ("#version 150");
    initialised = true;
}

void ImGuiSmokeOverlay::setShaderSource (std::string source)
{
    shaderSourceDraft = std::move (source);
    shaderSourceDraftNodeId = selectedNodeId (interactionSession);
}

void ImGuiSmokeOverlay::setRuntimeOpDiagnostics (std::vector<RuntimeOpModuleDiagnostic> diagnostics)
{
    runtimeOpDiagnostics = std::move (diagnostics);
}

void ImGuiSmokeOverlay::requestDeleteSelection()
{
    deleteSelectionRequested = true;
}

void ImGuiSmokeOverlay::requestSaveWork()
{
    if (onSaveWorkRequested == nullptr)
    {
        lastInteractionMessage = "save_work: no active work";
        return;
    }

    const auto result = onSaveWorkRequested (interactionSession);
    lastInteractionMessage = result.ok ? "save_work: " + result.message
                                       : "save_work failed: " + result.message;
}

void ImGuiSmokeOverlay::requestPublishSelectedModule()
{
    const auto nodeId = selectedNodeId (interactionSession);
    if (! selectedNodeIsCompound (interactionSession))
    {
        lastInteractionMessage = "publish_module failed: select compound node";
        return;
    }

    if (onPublishModuleRequested == nullptr)
    {
        lastInteractionMessage = "publish_module: no active publisher";
        return;
    }

    const auto result = onPublishModuleRequested (interactionSession, nodeId);
    lastInteractionMessage = result.ok ? "publish_module: " + result.message
                                       : "publish_module failed: " + result.message;
}

void ImGuiSmokeOverlay::shutdown()
{
    if (! initialised)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    initialised = false;
}

void ImGuiSmokeOverlay::beginFrame (int width, int height, float scale, float deltaSeconds)
{
    if (! initialised)
        return;

    const auto safeScale = scale > 0.0f ? scale : 1.0f;
    auto& io = ImGui::GetIO();
    io.DisplaySize = ImVec2 (static_cast<float> (width) / safeScale,
                             static_cast<float> (height) / safeScale);
    io.DisplayFramebufferScale = ImVec2 (safeScale, safeScale);
    io.DeltaTime = deltaSeconds > 0.0f ? deltaSeconds : 1.0f / 60.0f;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

void ImGuiSmokeOverlay::drawSmokePanel (const std::vector<NodeSpec>& nodeSpecs,
                                        const CompoundPatchSpec& loudnessCompound,
                                        const std::string& shaderStatus)
{
    if (! initialised)
        return;

    if (deleteSelectionRequested)
    {
        deleteSelectionRequested = false;

        if (! ImGui::GetIO().WantTextInput)
            runInteractionCommand ("delete", deleteSelectedGraphItem (interactionSession));
    }

    const auto displaySize = ImGui::GetIO().DisplaySize;
    const auto layout = makeTooll3SkinLayout (displaySize.x, displaySize.y);

    ImGui::SetNextWindowPos (ImVec2 (0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize (displaySize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha (0.0f);

    constexpr auto windowFlags = ImGuiWindowFlags_NoCollapse
                               | ImGuiWindowFlags_NoMove
                               | ImGuiWindowFlags_NoResize
                               | ImGuiWindowFlags_NoTitleBar
                               | ImGuiWindowFlags_NoScrollbar
                               | ImGuiWindowFlags_NoScrollWithMouse
                               | ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin ("Tooll3 Skin Workspace", nullptr, windowFlags))
    {
        auto& drawList = *ImGui::GetWindowDrawList();

        drawList.AddRectFilled (rectMin (layout.topStrip), rectMax (layout.topStrip), rgba (0, 0, 0, 230));
        drawList.AddRectFilled (rectMin (layout.leftRail), rectMax (layout.leftRail), rgba (18, 18, 18, 232));
        drawList.AddRectFilled (rectMin (layout.bottomStrip), rectMax (layout.bottomStrip), rgba (0, 0, 0, 218));
        drawList.AddLine ({ 0.0f, static_cast<float> (layout.topStrip.height) },
                          { displaySize.x, static_cast<float> (layout.topStrip.height) },
                          rgba (50, 50, 50, 180),
                          1.0f);
        drawList.AddLine ({ static_cast<float> (layout.leftRail.width), static_cast<float> (layout.topStrip.height) },
                          { static_cast<float> (layout.leftRail.width), displaySize.y },
                          rgba (0, 0, 0, 220),
                          1.0f);

        ImGui::SetCursorScreenPos ({ 8.0f, 6.0f });
        ImGui::TextUnformatted ("My World   Edit   View   Windows   Help");
        ImGui::SameLine (520.0f);
        ImGui::TextDisabled ("nodes %d  edges %d  dirty %s  patch %s",
                             static_cast<int> (interactionSession.graph.editorGraph.nodes.size()),
                             static_cast<int> (interactionSession.graph.editorGraph.edges.size()),
                             interactionSession.dirty ? "yes" : "no",
                             patchPathText (interactionSession).c_str());

        ImGui::SetCursorScreenPos ({ static_cast<float> (layout.leftRail.x + 8.0),
                                     static_cast<float> (layout.leftRail.y + 8.0) });
        ImGui::BeginChild ("left-rail",
                           { static_cast<float> (layout.leftRail.width - 16.0),
                             static_cast<float> (layout.leftRail.height - 16.0) },
                           false,
                           ImGuiWindowFlags_NoScrollbar);
        {
            const auto railPolicy = makeTooll3LeftRailPolicy();

            if (ImGui::BeginTabBar ("left-rail-tabs", ImGuiTabBarFlags_NoTooltip))
            {
                for (const auto& tab : railPolicy.tabs)
                {
                    if (! ImGui::BeginTabItem (tab.c_str()))
                        continue;

                    if (tab == "Presets")
                    {
                        ImGui::Dummy (ImVec2 (0.0f, 44.0f));
                        ImGui::TextDisabled ("No presets yet.");
                    }
                    else if (tab == "Snapshots")
                    {
                        ImGui::Dummy (ImVec2 (0.0f, 44.0f));
                        ImGui::TextDisabled ("No snapshots yet.");
                    }
                    else
                    {
                        ImGui::TextDisabled ("nodes: %d", static_cast<int> (nodeSpecs.size()));
                        ImGui::Separator();

                        for (size_t index = 0; index < std::min<size_t> (nodeSpecs.size(), 7); ++index)
                            ImGui::TextDisabled ("%s", nodeSpecs[index].type.c_str());

                        if (! runtimeOpDiagnostics.empty())
                        {
                            ImGui::SeparatorText ("Runtime");

                            for (const auto& diagnostic : runtimeOpDiagnostics)
                                drawRuntimeDiagnosticSummary (diagnostic);
                        }
                    }

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::Dummy (ImVec2 (0.0f, 14.0f));
            drawInspectorPanel (nodeSpecs, shaderStatus);

            if (! runtimeOpDiagnostics.empty())
            {
                ImGui::SeparatorText ("Runtime Coverage");

                for (const auto& diagnostic : runtimeOpDiagnostics)
                    drawRuntimeDiagnosticSummary (diagnostic);
            }

            ImGui::SeparatorText ("Shader");
            ImGui::TextWrapped ("%s", shaderStatus.c_str());
            ImGui::SliderFloat ("smoke value", &smokeValue, 0.0f, 1.0f);

            ImGui::SeparatorText ("Audio Compound");
            ImGui::Checkbox ("expanded", &loudnessExpanded);
            ImGui::Text ("children: %d  outputs: %d",
                         static_cast<int> (loudnessCompound.children.size()),
                         static_cast<int> (loudnessCompound.publicOutputs.size()));

            if (loudnessExpanded)
            {
                ImGui::SeparatorText ("child patchers");

                for (const auto& child : loudnessCompound.children)
                    ImGui::BulletText ("%s  [%s]", child.id.c_str(), child.nodeType.c_str());

                ImGui::SeparatorText ("public outputs");

                for (const auto& output : loudnessCompound.publicOutputs)
                    ImGui::BulletText ("%s -> %s", output.id.c_str(), output.mapsTo.c_str());
            }
        }
        ImGui::EndChild();

        ImGui::SetCursorScreenPos (rectMin (layout.outputSurface));
        drawInteractionCanvas (nodeSpecs,
                               loudnessCompound,
                               static_cast<float> (layout.outputSurface.width),
                               static_cast<float> (layout.outputSurface.height));

        ImGui::SetCursorScreenPos ({ static_cast<float> (layout.bottomStrip.x + 8.0),
                                     static_cast<float> (layout.bottomStrip.y + 6.0) });
        ImGui::BeginChild ("bottom-transport",
                           { static_cast<float> (layout.bottomStrip.width - 16.0),
                             static_cast<float> (layout.bottomStrip.height - 8.0) },
                           false,
                           ImGuiWindowFlags_NoScrollbar);
        {
            const auto transportPolicy = makeTooll3TransportPolicy();
            const auto appTime = static_cast<float> (ImGui::GetTime());
            ImGui::TextDisabled ("00:00:%05.2f", appTime);
            ImGui::SameLine();
            if (ImGui::Button ("Reset"))
            {
                interactionSession = makeDefaultOverlaySession();
                expandedPatchSession = {};
                interactionViewReady = false;
                expandedPatchViewReady = false;
                draggingNodeId.clear();
                draggingConnectionEndpoint.clear();
                pendingCreateSourceEndpoint.clear();
                dragCanvasDelta = {};
                draggingConnectionPoint = {};
                pendingCreatePosition = {};
                previousPanDrag = {};
                hasTraceReport = false;
                panningCanvas = false;
                expandedPatchParentId.clear();
                lastInteractionMessage = "reset";
            }
            ImGui::SameLine();
            if (ImGui::Button ("Undo"))
                runInteractionCommand ("undo", undo (interactionSession));
            ImGui::SameLine();
            if (ImGui::Button ("Redo"))
                runInteractionCommand ("redo", redo (interactionSession));
            ImGui::SameLine();
            if (ImGui::Button ("Delete"))
                runInteractionCommand ("delete", deleteSelectedGraphItem (interactionSession));
            ImGui::SameLine();
            if (ImGui::Button ("Run Trace"))
            {
                lastTraceReport = runBundledTraceFixture();
                hasTraceReport = true;
                lastInteractionMessage = lastTraceReport.ok ? "trace suite: ok" : "trace suite: failed";
            }
            ImGui::SameLine();
            if (hasTraceReport)
            {
                ImGui::TextDisabled ("%s traces %d commands %d",
                                     lastTraceReport.ok ? "ok" : "failed",
                                     lastTraceReport.tracesRun,
                                     static_cast<int> (lastTraceReport.commandsObserved.size()));
            }
            else
            {
                ImGui::TextDisabled ("trace not run");
            }

            if (transportPolicy.hasCommandStrip)
                ImGui::TextDisabled ("last: %s   commands %d",
                                     lastInteractionMessage.c_str(),
                                     static_cast<int> (interactionSession.commandLog.size()));

            if (transportPolicy.timelineEditingParked)
            {
                const auto trackPos = ImGui::GetCursorScreenPos();
                const auto trackWidth = ImGui::GetContentRegionAvail().x;
                const auto playhead = std::fmod (appTime, 10.0f) / 10.0f;
                auto& transportDrawList = *ImGui::GetWindowDrawList();
                transportDrawList.AddRectFilled (trackPos,
                                                 { trackPos.x + trackWidth, trackPos.y + 7.0f },
                                                 rgba (28, 28, 30, 210),
                                                 0.0f);
                transportDrawList.AddLine ({ trackPos.x + trackWidth * playhead, trackPos.y - 2.0f },
                                           { trackPos.x + trackWidth * playhead, trackPos.y + 10.0f },
                                           rgba (116, 166, 226, 230),
                                           1.0f);
                ImGui::Dummy ({ trackWidth, 10.0f });
            }
        }
        ImGui::EndChild();
    }

    ImGui::End();
}

void ImGuiSmokeOverlay::drawInteractionControls()
{
    if (ImGui::Button ("Reset"))
    {
        interactionSession = makeGraphSession (makeDefaultShaderOutputGraph());
        expandedPatchSession = {};
        interactionSession.selectedNodeIds = { "shader1" };
        interactionViewReady = false;
        expandedPatchViewReady = false;
        draggingNodeId.clear();
        draggingConnectionEndpoint.clear();
        pendingCreateSourceEndpoint.clear();
        dragCanvasDelta = {};
        draggingConnectionPoint = {};
        pendingCreatePosition = {};
        previousPanDrag = {};
        hasTraceReport = false;
        panningCanvas = false;
        expandedPatchParentId.clear();
        lastInteractionMessage = "reset";
    }

    ImGui::SameLine();
    if (ImGui::Button ("Move Shader"))
        runInteractionCommand ("move shader", moveNode (interactionSession, "shader1", 16.0, 8.0));

    ImGui::SameLine();
    if (ImGui::Button ("Undo"))
        runInteractionCommand ("undo", undo (interactionSession));

    ImGui::SameLine();
    if (ImGui::Button ("Redo"))
        runInteractionCommand ("redo", redo (interactionSession));

    if (ImGui::Button ("Delete"))
        runInteractionCommand ("delete", deleteSelectedGraphItem (interactionSession));

    ImGui::SameLine();
    if (ImGui::Button ("Connect"))
        runInteractionCommand ("connect", connectPorts (interactionSession, "shader1.output", "out1.input"));

    ImGui::SameLine();
    if (ImGui::Button ("Create Output"))
    {
        runInteractionCommand ("create output",
                               findEditorNode (interactionSession.graph, "out2") != nullptr
                                   ? CommandResult { false, "out2 already exists" }
                                   : createNodeAndConnect (interactionSession,
                                                           "shader1.output",
                                                           "output.preview",
                                                           "out2",
                                                           { 520.0, 160.0 }));
    }

    if (ImGui::Button ("Add Loudness"))
    {
        if (findEditorNode (interactionSession.graph, "loud1") == nullptr)
            runInteractionCommand ("create loudness",
                                   createNode (interactionSession, "compound.loudness", "loud1", { 180.0, 250.0 }));
        else
            lastInteractionMessage = "create loudness: loud1 already exists";
    }

    ImGui::SameLine();
    if (ImGui::Button ("Enter"))
    {
        const auto nodeId = selectedNodeId (interactionSession);
        runInteractionCommand ("enter patch",
                               selectedNodeIsCompound (interactionSession)
                                   ? enterPatch (interactionSession, nodeId)
                                   : CommandResult { false, "select compound node" });
    }

    ImGui::SameLine();
    if (ImGui::Button ("Exit"))
    {
        bool canExit = true;

        if (! interactionSession.currentPatchPath.empty() && ! expandedPatchParentId.empty())
        {
            const auto parentNodeId = interactionSession.currentPatchPath.back();
            const auto storeResult = storeExpandedPatchLayout (interactionSession,
                                                               parentNodeId,
                                                               expandedPatchSession.graph);

            if (storeResult.ok)
                runInteractionCommand ("store expanded layout", storeResult);
            else
            {
                lastInteractionMessage = "store expanded layout: " + storeResult.message;
                canExit = false;
            }
        }

        if (canExit)
            runInteractionCommand ("exit patch", exitPatch (interactionSession));
    }

    if (ImGui::Button ("Collapse/Expand"))
    {
        const auto nodeId = selectedNodeId (interactionSession);
        const auto* node = findEditorNode (interactionSession.graph, nodeId);
        runInteractionCommand ("collapse toggle",
                               nodeIsCompound (interactionSession.graph, nodeId)
                                   ? setCollapsed (interactionSession, nodeId, ! node->collapsed)
                                   : CommandResult { false, "select compound node" });
    }

    ImGui::SameLine();
    if (ImGui::Button ("Param"))
        runInteractionCommand ("set param", setParam (interactionSession, "shader1", "fragmentSource", "void main(){}"));

    ImGui::SameLine();
    ImGui::SameLine();
    if (ImGui::Button ("Save Work"))
        requestSaveWork();

    ImGui::SameLine();
    if (ImGui::Button ("Publish Module"))
        requestPublishSelectedModule();
}

void ImGuiSmokeOverlay::drawInteractionCanvas (const std::vector<NodeSpec>& nodeSpecs,
                                               const CompoundPatchSpec& loudnessCompound,
                                               float canvasWidth,
                                               float canvasHeight)
{
    const ImVec2 canvasSize {
        std::max (320.0f, canvasWidth),
        std::max (240.0f, canvasHeight)
    };

    const auto insidePatch = ! interactionSession.currentPatchPath.empty();
    if (insidePatch)
    {
        const auto parentNodeId = interactionSession.currentPatchPath.back();
        if (expandedPatchParentId != parentNodeId)
        {
            expandedPatchSession = makeGraphSession (makeCompoundPatchInteractionGraph (loudnessCompound,
                                                                                        parentNodeId,
                                                                                        interactionSession.graph));
            expandedPatchParentId = parentNodeId;
            expandedPatchViewReady = false;
        }

        if (! expandedPatchViewReady)
        {
            expandedPatchSession.view = defaultInteractionView (canvasSize);
            expandedPatchViewReady = true;
        }
    }
    else
    {
        expandedPatchParentId.clear();

        if (! interactionViewReady)
        {
            interactionSession.view = defaultInteractionView (canvasSize);
            interactionViewReady = true;
        }
    }

    auto& canvasSession = insidePatch ? expandedPatchSession : interactionSession;
    const auto storeExpandedLayoutAndExit = [this]
    {
        bool canExit = true;

        if (! interactionSession.currentPatchPath.empty() && ! expandedPatchParentId.empty())
        {
            const auto parentNodeId = interactionSession.currentPatchPath.back();
            const auto storeResult = storeExpandedPatchLayout (interactionSession,
                                                               parentNodeId,
                                                               expandedPatchSession.graph);

            if (storeResult.ok)
                runInteractionCommand ("store expanded layout", storeResult);
            else
            {
                lastInteractionMessage = "store expanded layout: " + storeResult.message;
                canExit = false;
            }
        }

        if (canExit)
            runInteractionCommand ("exit patch", exitPatch (interactionSession));
    };

    const auto origin = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton ("interaction-canvas",
                            canvasSize,
                            ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const auto hovered = ImGui::IsItemHovered();
    const auto active = ImGui::IsItemActive();
    const auto wheel = ImGui::GetIO().MouseWheel;

    if (hovered && wheel != 0.0f && draggingNodeId.empty() && draggingConnectionEndpoint.empty())
    {
        const auto mouse = ImGui::GetMousePos();
        canvasSession.view = zoomViewAround (canvasSession.view,
                                             wheel > 0.0f ? 1.10 : 0.90,
                                             { mouse.x - origin.x, mouse.y - origin.y });
        lastInteractionMessage = "zoom canvas";
    }

    auto& drawList = *ImGui::GetWindowDrawList();
    drawList.AddRectFilled (origin,
                            { origin.x + canvasSize.x, origin.y + canvasSize.y },
                            IM_COL32 (0, 0, 0, 36),
                            0.0f);

    const auto gridStep = static_cast<float> (48.0 * canvasSession.view.scale);
    if (gridStep > 8.0f)
    {
        const auto gridColour = IM_COL32 (255, 255, 255, 18);
        const auto startX = std::fmod (static_cast<float> (canvasSession.view.scrollX), gridStep);
        const auto startY = std::fmod (static_cast<float> (canvasSession.view.scrollY), gridStep);

        for (auto x = startX; x < canvasSize.x; x += gridStep)
            drawList.AddLine ({ origin.x + x, origin.y },
                              { origin.x + x, origin.y + canvasSize.y },
                              gridColour,
                              1.0f);

        for (auto y = startY; y < canvasSize.y; y += gridStep)
            drawList.AddLine ({ origin.x, origin.y + y },
                              { origin.x + canvasSize.x, origin.y + y },
                              gridColour,
                              1.0f);
    }

    if (! interactionSession.currentPatchPath.empty())
    {
        drawList.AddText ({ origin.x + 16.0f, origin.y + 14.0f },
                          IM_COL32 (250, 214, 112, 255),
                          ("inside " + patchPathText (interactionSession)).c_str());
        ImGui::SetCursorScreenPos ({ origin.x + 16.0f, origin.y + 34.0f });
        ImGui::PushID ("expanded-patch-exit");
        if (ImGui::SmallButton ("Exit"))
            storeExpandedLayoutAndExit();
        ImGui::PopID();
    }

    drawList.PushClipRect (origin,
                           { origin.x + canvasSize.x, origin.y + canvasSize.y },
                           true);

    if (hovered && ImGui::IsMouseClicked (ImGuiMouseButton_Left))
    {
        const auto mouse = ImGui::GetMousePos();
        const auto hit = hitTestGraph (canvasSession.graph,
                                       nodeSpecs,
                                       canvasSession.view,
                                       { mouse.x - origin.x, mouse.y - origin.y });

        canvasSession.selectedEdgeIds.clear();

        if (hit.kind == HitTestKind::outputPort)
        {
            draggingConnectionEndpoint = hit.endpoint;
            draggingConnectionPoint = canvasPointFromMouse (canvasSession.view, origin);
            lastInteractionMessage = "drag connection: " + hit.endpoint;
        }
        else if (hit.kind == HitTestKind::nodeBody)
        {
            draggingNodeId = hit.nodeId;
            canvasSession.selectedNodeIds = { hit.nodeId };

            if (! insidePatch
                && ImGui::IsMouseDoubleClicked (ImGuiMouseButton_Left)
                && nodeIsCompound (interactionSession.graph, hit.nodeId))
                runInteractionCommand ("enter patch", enterPatch (interactionSession, hit.nodeId));
        }
        else if (hit.kind == HitTestKind::edge)
        {
            canvasSession.selectedNodeIds.clear();
            canvasSession.selectedEdgeIds = { hit.edgeId };
            lastInteractionMessage = "selected edge: " + hit.edgeId;
        }
        else
        {
            canvasSession.selectedNodeIds.clear();
            panningCanvas = true;
            previousPanDrag = {};
            lastInteractionMessage = "pan canvas";
        }
    }

    if (hovered && ImGui::IsMouseClicked (ImGuiMouseButton_Right))
    {
        const auto mouse = ImGui::GetMousePos();
        const auto hit = hitTestGraph (canvasSession.graph,
                                       nodeSpecs,
                                       canvasSession.view,
                                       { mouse.x - origin.x, mouse.y - origin.y });

        if (hit.kind == HitTestKind::none && ! insidePatch)
        {
            pendingCreateSourceEndpoint.clear();
            pendingCreatePosition = canvasPointFromMouse (canvasSession.view, origin);
            nodeBrowserScreenPosition = { mouse.x, mouse.y };
            nodeBrowserFilter.clear();
            ImGui::OpenPopup ("workspace-node-browser");
            lastInteractionMessage = "node browser";
        }
        else if (hit.kind == HitTestKind::none)
        {
            pendingCreateSourceEndpoint.clear();
            pendingCreatePosition = {};
            lastInteractionMessage = "expanded patch: creation parked";
        }
    }

    if (! draggingConnectionEndpoint.empty() && active)
    {
        draggingConnectionPoint = canvasPointFromMouse (canvasSession.view, origin);
    }
    else if (panningCanvas && active && ImGui::IsMouseDragging (ImGuiMouseButton_Left))
    {
        const auto drag = ImGui::GetMouseDragDelta (ImGuiMouseButton_Left);
        const auto deltaX = static_cast<double> (drag.x) - previousPanDrag.x;
        const auto deltaY = static_cast<double> (drag.y) - previousPanDrag.y;
        canvasSession.view = panView (canvasSession.view, deltaX, deltaY);
        previousPanDrag = { drag.x, drag.y };
    }
    else if (! draggingNodeId.empty() && active && ImGui::IsMouseDragging (ImGuiMouseButton_Left))
    {
        const auto drag = ImGui::GetMouseDragDelta (ImGuiMouseButton_Left);
        dragCanvasDelta = { drag.x / canvasSession.view.scale,
                            drag.y / canvasSession.view.scale };
    }

    if (! draggingConnectionEndpoint.empty() && ImGui::IsMouseReleased (ImGuiMouseButton_Left))
    {
        const auto mouse = ImGui::GetMousePos();
        const auto hit = hitTestGraph (canvasSession.graph,
                                       nodeSpecs,
                                       canvasSession.view,
                                       { mouse.x - origin.x, mouse.y - origin.y });

        if (hit.kind == HitTestKind::inputPort)
            runInteractionCommand ("connect gesture",
                                   connectPorts (canvasSession, nodeSpecs, draggingConnectionEndpoint, hit.endpoint));
        else
        {
            if (! insidePatch)
            {
                pendingCreateSourceEndpoint = draggingConnectionEndpoint;
                pendingCreatePosition = canvasPointFromMouse (canvasSession.view, origin);
                ImGui::OpenPopup ("create-compatible-node");
                lastInteractionMessage = "create node search";
            }
            else
            {
                pendingCreateSourceEndpoint.clear();
                pendingCreatePosition = {};
                lastInteractionMessage = "expanded patch: create search parked";
            }
        }

        draggingConnectionEndpoint.clear();
        draggingConnectionPoint = {};
    }
    else if (panningCanvas && ImGui::IsMouseReleased (ImGuiMouseButton_Left))
    {
        panningCanvas = false;
        previousPanDrag = {};
    }
    else if (! draggingNodeId.empty() && ImGui::IsMouseReleased (ImGuiMouseButton_Left))
    {
        if (std::abs (dragCanvasDelta.x) > 0.5 || std::abs (dragCanvasDelta.y) > 0.5)
            runInteractionCommand ("drag " + draggingNodeId,
                                   moveNode (canvasSession, draggingNodeId, dragCanvasDelta.x, dragCanvasDelta.y));

        draggingNodeId.clear();
        dragCanvasDelta = {};
    }

    for (const auto& edge : canvasSession.graph.editorGraph.edges)
    {
        const auto from = portCenter (canvasSession.graph, nodeSpecs, edge.from);
        const auto to = portCenter (canvasSession.graph, nodeSpecs, edge.to);

        if (! from.ok || ! to.ok)
            continue;

        const auto p1 = toImVec (from.point, canvasSession.view, origin);
        const auto p2 = toImVec (to.point, canvasSession.view, origin);
        const auto c1 = ImVec2 (p1.x + 58.0f, p1.y);
        const auto c2 = ImVec2 (p2.x - 58.0f, p2.y);
        const auto selected = std::find (canvasSession.selectedEdgeIds.begin(),
                                         canvasSession.selectedEdgeIds.end(),
                                         edge.id) != canvasSession.selectedEdgeIds.end();
        const auto skin = makeTooll3ConnectionSkin (edge.dataType, selected, true);
        drawList.AddBezierCubic (p1,
                                 c1,
                                 c2,
                                 p2,
                                 rgba (skin.color),
                                 static_cast<float> (skin.thickness),
                                 24);
    }

    if (! draggingConnectionEndpoint.empty())
    {
        const auto from = portCenter (canvasSession.graph, nodeSpecs, draggingConnectionEndpoint);

        if (from.ok)
        {
            const auto p1 = toImVec (from.point, canvasSession.view, origin);
            const auto p2 = toImVec (draggingConnectionPoint, canvasSession.view, origin);
            const auto sourceDataType = outputDataTypeForEndpoint (canvasSession.graph,
                                                                   nodeSpecs,
                                                                   draggingConnectionEndpoint);
            const auto skin = makeTooll3ConnectionSkin (sourceDataType, false, true);
            drawList.AddBezierCubic (p1,
                                     { p1.x + 58.0f, p1.y },
                                     { p2.x - 58.0f, p2.y },
                                     p2,
                                     rgba (skin.color),
                                     static_cast<float> (skin.thickness),
                                     24);
        }
    }

    const auto mousePosition = ImGui::GetMousePos();
    const auto hoverHit = hovered ? hitTestGraph (canvasSession.graph,
                                                  nodeSpecs,
                                                  canvasSession.view,
                                                  { mousePosition.x - origin.x, mousePosition.y - origin.y })
                                  : HitTestResult {};
    const auto sourceDataType = draggingConnectionEndpoint.empty()
                                    ? std::string {}
                                    : outputDataTypeForEndpoint (canvasSession.graph,
                                                                 nodeSpecs,
                                                                 draggingConnectionEndpoint);
    const auto toScreenMin = [&] (CanvasNodeBounds bounds)
    {
        return toImVec ({ bounds.x, bounds.y }, canvasSession.view, origin);
    };
    const auto toScreenMax = [&] (CanvasNodeBounds bounds)
    {
        return toImVec ({ bounds.x + bounds.width, bounds.y + bounds.height }, canvasSession.view, origin);
    };
    const auto drawClippedText = [&] (CanvasNodeBounds bounds, ImU32 colour, const std::string& text, bool alignRight)
    {
        if (text.empty() || canvasSession.view.scale < 0.80)
            return;

        const auto min = toScreenMin (bounds);
        const auto max = toScreenMax (bounds);

        if (max.x <= min.x || max.y <= min.y)
            return;

        const auto labelSize = ImGui::CalcTextSize (text.c_str());
        const auto textX = alignRight ? std::max (min.x, max.x - labelSize.x) : min.x;
        const ImVec4 clipRect { min.x, min.y, max.x, max.y };
        drawList.AddText (ImGui::GetFont(),
                          ImGui::GetFontSize(),
                          { textX, min.y },
                          colour,
                          text.c_str(),
                          nullptr,
                          0.0f,
                          &clipRect);
    };

    for (const auto& node : canvasSession.graph.editorGraph.nodes)
    {
        auto displayNode = node;
        const auto position = displayedPosition (node, draggingNodeId, dragCanvasDelta);
        displayNode.position = { position.x, position.y };
        const auto* spec = findNodeSpec (nodeSpecs, node.type);
        const auto surface = canvasNodeSurfaceGeometry (displayNode, spec);
        const auto topLeft = toScreenMin (surface.bounds);
        const auto bottomRight = toScreenMax (surface.bounds);
        const auto primaryDataType = primaryDataTypeForSpec (spec);
        const auto selected = std::find (canvasSession.selectedNodeIds.begin(),
                                         canvasSession.selectedNodeIds.end(),
                                         node.id) != canvasSession.selectedNodeIds.end();
        const auto activeNode = selected || node.id == draggingNodeId;
        const auto hoveredNode = hoverHit.kind == HitTestKind::nodeBody && hoverHit.nodeId == node.id;
        const auto skin = makeTooll3NodeSkin (node.type, primaryDataType, activeNode, hoveredNode);

        drawList.AddRectFilled (topLeft, bottomRight, rgba (skin.fill), static_cast<float> (skin.cornerRadius));
        drawList.AddRect (topLeft,
                          bottomRight,
                          rgba (skin.border),
                          static_cast<float> (skin.cornerRadius),
                          0,
                          static_cast<float> (skin.borderWidth));
        drawClippedText (surface.titleBounds, rgba (skin.label), node.id, false);

        if (spec == nullptr)
            continue;

        for (size_t index = 0; index < surface.inputRows.size(); ++index)
        {
            const auto compatible = sourceDataType.empty() || spec->inputs[index].dataType == sourceDataType;
            const auto portSkin = makeTooll3PortSkin (spec->inputs[index].dataType,
                                                      compatible,
                                                      ! draggingConnectionEndpoint.empty());
            const auto stripMin = toScreenMin (surface.inputRows[index].stripBounds);
            const auto stripMax = toScreenMax (surface.inputRows[index].stripBounds);
            drawPortStrip (drawList,
                           stripMin,
                           stripMax,
                           portSkin);

            drawClippedText (surface.inputRows[index].labelBounds,
                             rgba (portSkin.label),
                             surface.inputRows[index].label,
                             false);
        }

        for (size_t index = 0; index < surface.outputRows.size(); ++index)
        {
            const auto endpoint = node.id + "." + spec->outputs[index].id;
            const auto activePort = endpoint == draggingConnectionEndpoint;
            const auto portSkin = makeTooll3PortSkin (spec->outputs[index].dataType, true, activePort);
            const auto stripMin = toScreenMin (surface.outputRows[index].stripBounds);
            const auto stripMax = toScreenMax (surface.outputRows[index].stripBounds);
            drawPortStrip (drawList,
                           stripMin,
                           stripMax,
                           portSkin);

            drawClippedText (surface.outputRows[index].labelBounds,
                             rgba (portSkin.label),
                             surface.outputRows[index].label,
                             true);
        }
    }

    drawList.PopClipRect();
    ImGui::Dummy (ImVec2 (0.0f, 6.0f));
    drawCreateNodePopup (nodeSpecs);
    drawWorkspaceNodeBrowser (nodeSpecs);
}

void ImGuiSmokeOverlay::runInteractionCommand (const std::string& label, CommandResult result)
{
    if (result.ok)
        interactionSession.selectedEdgeIds.clear();

    lastInteractionMessage = result.ok ? label + ": ok" : label + ": " + result.message;
}

void ImGuiSmokeOverlay::runInteractionCommand (const std::string& label, bool result)
{
    lastInteractionMessage = result ? label + ": ok" : label + ": rejected";
}

void ImGuiSmokeOverlay::render()
{
    if (! initialised)
        return;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData (ImGui::GetDrawData());
}

bool ImGuiSmokeOverlay::wantsMouse() const
{
    return initialised && ImGui::GetIO().WantCaptureMouse;
}

void ImGuiSmokeOverlay::setMousePosition (float x, float y)
{
    if (initialised)
        ImGui::GetIO().AddMousePosEvent (x, y);
}

void ImGuiSmokeOverlay::setMouseButton (int buttonIndex, bool isDown)
{
    if (initialised)
        ImGui::GetIO().AddMouseButtonEvent (buttonIndex, isDown);
}

void ImGuiSmokeOverlay::addMouseWheel (float deltaY)
{
    if (initialised)
        ImGui::GetIO().AddMouseWheelEvent (0.0f, deltaY);
}
}
