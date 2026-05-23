#include "ImGuiSmokeOverlay.h"

#include "CompoundPatch.h"
#include "NodeSpec.h"
#include "Tooll3SkinContract.h"

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cfloat>
#include <sstream>

namespace myworld
{
namespace
{
constexpr float nodeWidth = 140.0f;
constexpr float nodeHeight = 60.0f;

ImVec2 rectMin (Tooll3SkinRect rect)
{
    return { static_cast<float> (rect.x), static_cast<float> (rect.y) };
}

ImVec2 rectMax (Tooll3SkinRect rect)
{
    return {
        static_cast<float> (rect.x + rect.width),
        static_cast<float> (rect.y + rect.height)
    };
}

ImVec2 rectSize (Tooll3SkinRect rect)
{
    return {
        static_cast<float> (rect.width),
        static_cast<float> (rect.height)
    };
}

ImU32 rgba (int r, int g, int b, int a)
{
    return IM_COL32 (r, g, b, a);
}

ImU32 rgba (Tooll3SkinColor colour)
{
    return IM_COL32 (colour.r, colour.g, colour.b, colour.a);
}

ImVec4 vec4 (Tooll3SkinColor colour)
{
    return {
        static_cast<float> (colour.r) / 255.0f,
        static_cast<float> (colour.g) / 255.0f,
        static_cast<float> (colour.b) / 255.0f,
        static_cast<float> (colour.a) / 255.0f
    };
}

std::string patchPathText (const GraphSession& session)
{
    if (session.currentPatchPath.empty())
        return "root";

    std::ostringstream text;
    for (size_t index = 0; index < session.currentPatchPath.size(); ++index)
    {
        if (index > 0)
            text << "/";

        text << session.currentPatchPath[index];
    }

    return text.str();
}

const GraphNode* findNode (const GraphContract& graph, const std::string& id)
{
    for (const auto& node : graph.editorGraph.nodes)
        if (node.id == id)
            return &node;

    return nullptr;
}

bool hasNode (const GraphContract& graph, const std::string& id)
{
    return findNode (graph, id) != nullptr;
}

const NodeSpec* specForNode (const GraphContract& graph,
                             const std::vector<NodeSpec>& specs,
                             const std::string& nodeId)
{
    const auto* node = findNode (graph, nodeId);
    return node == nullptr ? nullptr : findNodeSpec (specs, node->type);
}

const RuntimeOpModuleDiagnostic* diagnosticForNodeType (const std::vector<RuntimeOpModuleDiagnostic>& diagnostics,
                                                        const std::string& nodeType)
{
    const auto found = std::find_if (diagnostics.begin(), diagnostics.end(), [&nodeType] (const auto& diagnostic) {
        return diagnostic.nodeType == nodeType;
    });

    return found == diagnostics.end() ? nullptr : &*found;
}

bool diagnosticIsReady (const RuntimeOpModuleDiagnostic& diagnostic)
{
    return diagnostic.status == "runtime-op-ready";
}

ImVec4 diagnosticTextColour (const RuntimeOpModuleDiagnostic& diagnostic)
{
    return diagnosticIsReady (diagnostic)
               ? ImVec4 (0.55f, 0.82f, 0.66f, 1.0f)
               : ImVec4 (1.0f, 0.70f, 0.42f, 1.0f);
}

std::vector<NodeCreationGate> makeNodeCreationGates (const std::vector<RuntimeOpModuleDiagnostic>& diagnostics)
{
    std::vector<NodeCreationGate> gates;
    gates.reserve (diagnostics.size());

    for (const auto& diagnostic : diagnostics)
    {
        const auto canCreate = runtimeOpDiagnosticAllowsCreation (diagnostic);
        gates.push_back ({ diagnostic.nodeType,
                           canCreate,
                           canCreate ? std::string {} : diagnostic.creationBlockReason });
    }

    return gates;
}

const char* diagnosticCreationLabel (const RuntimeOpModuleDiagnostic& diagnostic)
{
    return diagnostic.creationLabel.empty() ? diagnostic.browserLabel.c_str() : diagnostic.creationLabel.c_str();
}

std::string debugOverrideReasonFor (const RuntimeOpModuleDiagnostic& diagnostic)
{
    return diagnostic.creationBlockReason.empty()
               ? "debug override: " + diagnostic.nodeType
               : "debug override: " + diagnostic.creationBlockReason;
}

void drawRuntimeDiagnosticSummary (const RuntimeOpModuleDiagnostic& diagnostic)
{
    ImGui::TextColored (diagnosticTextColour (diagnostic), "%s", diagnostic.browserLabel.c_str());
    ImGui::Indent (12.0f);
    ImGui::TextWrapped ("%s", diagnostic.nodeType.c_str());

    if (! diagnosticIsReady (diagnostic))
        ImGui::TextWrapped ("%s", diagnostic.inspectorDetail.c_str());

    ImGui::Unindent (12.0f);
}

std::string nodeIdFromEndpoint (const std::string& endpoint)
{
    const auto dot = endpoint.find ('.');
    return dot == std::string::npos ? endpoint : endpoint.substr (0, dot);
}

std::string portIdFromEndpoint (const std::string& endpoint)
{
    const auto dot = endpoint.find ('.');
    return dot == std::string::npos ? std::string {} : endpoint.substr (dot + 1);
}

std::string outputDataTypeForEndpoint (const GraphContract& graph,
                                       const std::vector<NodeSpec>& specs,
                                       const std::string& endpoint)
{
    const auto* spec = specForNode (graph, specs, nodeIdFromEndpoint (endpoint));

    if (spec == nullptr)
        return {};

    const auto portId = portIdFromEndpoint (endpoint);
    for (const auto& port : spec->outputs)
        if (port.id == portId)
            return port.dataType;

    return {};
}

std::string makeNodeIdStem (const std::string& nodeType);

bool canCreateFromEndpoint (const NodeSpec& spec, const std::string& sourceDataType)
{
    return ! spec.inputs.empty() && spec.inputs.front().dataType == sourceDataType;
}

bool nodeSpecMatchesFilter (const NodeSpec& spec, const std::string& filter)
{
    if (filter.empty())
        return true;

    const auto needle = makeNodeIdStem (filter);
    const auto haystack = makeNodeIdStem (spec.type + " " + spec.displayName + " " + spec.category + " " + spec.subcategory);
    return haystack.find (needle) != std::string::npos;
}

std::string makeNodeIdStem (const std::string& nodeType)
{
    std::string stem;

    for (const auto c : nodeType)
    {
        if (std::isalnum (static_cast<unsigned char> (c)))
            stem.push_back (static_cast<char> (std::tolower (static_cast<unsigned char> (c))));
        else if (! stem.empty() && stem.back() != '_')
            stem.push_back ('_');
    }

    while (! stem.empty() && stem.back() == '_')
        stem.pop_back();

    return stem.empty() ? "node" : stem;
}

std::string makeUniqueNodeId (const GraphContract& graph, const std::string& nodeType)
{
    const auto stem = makeNodeIdStem (nodeType);

    for (int index = 1; index < 1000; ++index)
    {
        const auto candidate = stem + std::to_string (index);
        if (! hasNode (graph, candidate))
            return candidate;
    }

    return stem + "_overflow";
}

ImVec2 toImVec (CanvasPoint point, const CanvasViewState& view, ImVec2 origin)
{
    const auto screen = canvasToScreen (view, point);
    return { origin.x + static_cast<float> (screen.x),
             origin.y + static_cast<float> (screen.y) };
}

CanvasPoint displayedPosition (const GraphNode& node, const std::string& draggingNodeId, CanvasPoint dragCanvasDelta)
{
    if (node.id != draggingNodeId)
        return { node.position.x, node.position.y };

    return { node.position.x + dragCanvasDelta.x, node.position.y + dragCanvasDelta.y };
}

std::string selectedEdgeId (const GraphSession& session)
{
    return session.selectedEdgeIds.empty() ? std::string {} : session.selectedEdgeIds.front();
}

std::string selectedNodeId (const GraphSession& session)
{
    return session.selectedNodeIds.empty() ? std::string {} : session.selectedNodeIds.front();
}

bool nodeIsCompound (const GraphContract& graph, const std::string& nodeId)
{
    const auto* node = findNode (graph, nodeId);
    return node != nullptr && node->type.rfind ("compound.", 0) == 0;
}

bool selectedNodeIsCompound (const GraphSession& session)
{
    return nodeIsCompound (session.graph, selectedNodeId (session));
}

CommandResult deleteSelectedGraphItem (GraphSession& session)
{
    const auto edgeId = selectedEdgeId (session);
    if (! edgeId.empty())
        return disconnectEdge (session, edgeId);

    const auto nodeId = selectedNodeId (session);
    if (! nodeId.empty())
        return deleteNode (session, nodeId);

    return { false, "nothing selected" };
}

std::string paramValueForNode (const GraphNode& node, const std::string& paramId)
{
    for (const auto& param : node.params)
        if (param.id == paramId)
            return param.value;

    return {};
}

std::string bindingValueForPort (const GraphNode& node, const std::string& portId)
{
    for (const auto& binding : node.portBindings)
    {
        if (binding.portId == portId)
            return binding.bindingMode + ":" + binding.value;
    }

    return {};
}

std::string bindingModeForPort (const GraphNode& node, const std::string& portId)
{
    for (const auto& binding : node.portBindings)
        if (binding.portId == portId)
            return binding.bindingMode.empty() ? "default" : binding.bindingMode;

    return "default";
}

std::string demoValueForParam (const ParamSpec& param)
{
    if (! param.defaultValue.empty())
        return param.defaultValue;

    if (param.dataType == "text.glsl")
        return "void main(){}";

    return "demo";
}

std::string primaryDataTypeForSpec (const NodeSpec* spec)
{
    if (spec == nullptr)
        return {};

    if (! spec->outputs.empty())
        return spec->outputs.front().dataType;

    if (! spec->inputs.empty())
        return spec->inputs.front().dataType;

    return {};
}

void drawPortStrip (ImDrawList& drawList, ImVec2 min, ImVec2 max, const Tooll3PortSkin& skin)
{
    drawList.AddRectFilled (min, max, rgba (skin.strip), 0.0f);

    if (skin.compatibleHighlight)
        drawList.AddRect (min, max, IM_COL32 (245, 250, 255, 235), 0.0f, 0, 1.0f);
}

std::string compactInspectorValue (const ParamSpec& param, const std::string& value)
{
    if (param.dataType == "text.glsl")
        return value.empty() ? "GLSL" : std::to_string (value.size()) + " bytes";

    if (value.empty())
        return param.defaultValue.empty() ? "default" : param.defaultValue;

    return value;
}

void drawInspectorRow (const std::string& label, const std::string& value, const std::string& state)
{
    const auto skin = makeTooll3InspectorRowSkin (state);
    const auto pos = ImGui::GetCursorScreenPos();
    const auto width = ImGui::GetContentRegionAvail().x;

    ImGui::GetWindowDrawList()->AddRectFilled ({ pos.x, pos.y + 2.0f },
                                               { pos.x + 4.0f, pos.y + 18.0f },
                                               rgba (skin.stateAccent),
                                               0.0f);
    ImGui::Dummy ({ width, 22.0f });
    ImGui::SetCursorScreenPos ({ pos.x + 9.0f, pos.y + 1.0f });
    ImGui::TextColored (vec4 (skin.label), "%s", label.c_str());
    ImGui::SameLine (100.0f);
    ImGui::TextColored (vec4 (skin.value), "%s", value.c_str());
    const auto stateSize = ImGui::CalcTextSize (skin.stateLabel.c_str());
    ImGui::SameLine (std::max (170.0f, width - stateSize.x - 4.0f));
    ImGui::TextDisabled ("%s", skin.stateLabel.c_str());
}

CanvasPoint canvasPointFromMouse (const CanvasViewState& view, ImVec2 origin)
{
    const auto mouse = ImGui::GetMousePos();
    return screenToCanvas (view, { mouse.x - origin.x, mouse.y - origin.y });
}

CanvasViewState defaultInteractionView (ImVec2 canvasSize)
{
    return {
        1.0,
        std::max (48.0, static_cast<double> (canvasSize.x) * 0.28 - 80.0),
        std::max (56.0, static_cast<double> (canvasSize.y) * 0.38 - 80.0)
    };
}

BehaviorTraceReport runBundledTraceFixture()
{
    const std::vector<std::string> candidatePaths {
        "fixtures/interaction/tooll3-t0-t7.behavior.json",
        "../../../../../fixtures/interaction/tooll3-t0-t7.behavior.json",
        "/Users/chenbaiwei/Desktop/我的世界/fixtures/interaction/tooll3-t0-t7.behavior.json"
    };

    BehaviorTraceReport lastReport;

    for (const auto& path : candidatePaths)
    {
        auto report = runBehaviorTraceFixture (path);
        if (report.ok)
            return report;

        lastReport = std::move (report);
    }

    return lastReport;
}

GraphSession makeDefaultOverlaySession()
{
    auto session = makeGraphSession (makeDefaultShaderOutputGraph());
    session.selectedNodeIds = { "shader1" };
    return session;
}
}

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
                interactionViewReady = false;
                draggingNodeId.clear();
                draggingConnectionEndpoint.clear();
                pendingCreateSourceEndpoint.clear();
                dragCanvasDelta = {};
                draggingConnectionPoint = {};
                pendingCreatePosition = {};
                previousPanDrag = {};
                savedInteractionState.clear();
                hasTraceReport = false;
                panningCanvas = false;
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
                ImGui::TextDisabled ("last: %s   saved bytes %d",
                                     lastInteractionMessage.c_str(),
                                     static_cast<int> (savedInteractionState.size()));

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
        interactionSession.selectedNodeIds = { "shader1" };
        interactionViewReady = false;
        draggingNodeId.clear();
        draggingConnectionEndpoint.clear();
        pendingCreateSourceEndpoint.clear();
        dragCanvasDelta = {};
        draggingConnectionPoint = {};
        pendingCreatePosition = {};
        previousPanDrag = {};
        savedInteractionState.clear();
        hasTraceReport = false;
        panningCanvas = false;
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
                               hasNode (interactionSession.graph, "out2")
                                   ? CommandResult { false, "out2 already exists" }
                                   : createNodeAndConnect (interactionSession,
                                                           "shader1.output",
                                                           "output.preview",
                                                           "out2",
                                                           { 520.0, 160.0 }));
    }

    if (ImGui::Button ("Add Loudness"))
    {
        if (! hasNode (interactionSession.graph, "loud1"))
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
        runInteractionCommand ("exit patch", exitPatch (interactionSession));

    if (ImGui::Button ("Collapse/Expand"))
    {
        const auto nodeId = selectedNodeId (interactionSession);
        const auto* node = findNode (interactionSession.graph, nodeId);
        runInteractionCommand ("collapse toggle",
                               nodeIsCompound (interactionSession.graph, nodeId)
                                   ? setCollapsed (interactionSession, nodeId, ! node->collapsed)
                                   : CommandResult { false, "select compound node" });
    }

    ImGui::SameLine();
    if (ImGui::Button ("Param"))
        runInteractionCommand ("set param", setParam (interactionSession, "shader1", "fragmentSource", "void main(){}"));

    ImGui::SameLine();
    if (ImGui::Button ("Save State"))
    {
        lastInteractionMessage = markSavedAndCommitted (interactionSession);
        savedInteractionState = serializeInteractionState (interactionSession);
    }

    ImGui::SameLine();
    if (ImGui::Button ("Reload State"))
    {
        if (savedInteractionState.empty())
        {
            lastInteractionMessage = "reload state: no saved state";
        }
        else
        {
            const auto view = interactionSession.view;
            interactionSession = deserializeInteractionState (savedInteractionState);
            interactionSession.view = view;
            lastInteractionMessage = "reload state: ok";
        }
    }
}

void ImGuiSmokeOverlay::drawInteractionCanvas (const std::vector<NodeSpec>& nodeSpecs,
                                               float canvasWidth,
                                               float canvasHeight)
{
    const ImVec2 canvasSize {
        std::max (320.0f, canvasWidth),
        std::max (240.0f, canvasHeight)
    };

    if (! interactionViewReady)
    {
        interactionSession.view = defaultInteractionView (canvasSize);
        interactionViewReady = true;
    }

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
        interactionSession.view = zoomViewAround (interactionSession.view,
                                                  wheel > 0.0f ? 1.10 : 0.90,
                                                  { mouse.x - origin.x, mouse.y - origin.y });
        lastInteractionMessage = "zoom canvas";
    }

    auto& drawList = *ImGui::GetWindowDrawList();
    drawList.AddRectFilled (origin,
                            { origin.x + canvasSize.x, origin.y + canvasSize.y },
                            IM_COL32 (0, 0, 0, 36),
                            0.0f);

    const auto gridStep = static_cast<float> (48.0 * interactionSession.view.scale);
    if (gridStep > 8.0f)
    {
        const auto gridColour = IM_COL32 (255, 255, 255, 18);
        const auto startX = std::fmod (static_cast<float> (interactionSession.view.scrollX), gridStep);
        const auto startY = std::fmod (static_cast<float> (interactionSession.view.scrollY), gridStep);

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
    }

    drawList.PushClipRect (origin,
                           { origin.x + canvasSize.x, origin.y + canvasSize.y },
                           true);

    if (hovered && ImGui::IsMouseClicked (ImGuiMouseButton_Left))
    {
        const auto mouse = ImGui::GetMousePos();
        const auto hit = hitTestGraph (interactionSession.graph,
                                       nodeSpecs,
                                       interactionSession.view,
                                       { mouse.x - origin.x, mouse.y - origin.y });

        interactionSession.selectedEdgeIds.clear();

        if (hit.kind == HitTestKind::outputPort)
        {
            draggingConnectionEndpoint = hit.endpoint;
            draggingConnectionPoint = canvasPointFromMouse (interactionSession.view, origin);
            lastInteractionMessage = "drag connection: " + hit.endpoint;
        }
        else if (hit.kind == HitTestKind::nodeBody)
        {
            draggingNodeId = hit.nodeId;
            interactionSession.selectedNodeIds = { hit.nodeId };

            if (ImGui::IsMouseDoubleClicked (ImGuiMouseButton_Left) && nodeIsCompound (interactionSession.graph, hit.nodeId))
                runInteractionCommand ("enter patch", enterPatch (interactionSession, hit.nodeId));
        }
        else if (hit.kind == HitTestKind::edge)
        {
            interactionSession.selectedNodeIds.clear();
            interactionSession.selectedEdgeIds = { hit.edgeId };
            lastInteractionMessage = "selected edge: " + hit.edgeId;
        }
        else
        {
            interactionSession.selectedNodeIds.clear();
            panningCanvas = true;
            previousPanDrag = {};
            lastInteractionMessage = "pan canvas";
        }
    }

    if (hovered && ImGui::IsMouseClicked (ImGuiMouseButton_Right))
    {
        const auto mouse = ImGui::GetMousePos();
        const auto hit = hitTestGraph (interactionSession.graph,
                                       nodeSpecs,
                                       interactionSession.view,
                                       { mouse.x - origin.x, mouse.y - origin.y });

        if (hit.kind == HitTestKind::none)
        {
            pendingCreateSourceEndpoint.clear();
            pendingCreatePosition = canvasPointFromMouse (interactionSession.view, origin);
            nodeBrowserScreenPosition = { mouse.x, mouse.y };
            nodeBrowserFilter.clear();
            ImGui::OpenPopup ("workspace-node-browser");
            lastInteractionMessage = "node browser";
        }
    }

    if (! draggingConnectionEndpoint.empty() && active)
    {
        draggingConnectionPoint = canvasPointFromMouse (interactionSession.view, origin);
    }
    else if (panningCanvas && active && ImGui::IsMouseDragging (ImGuiMouseButton_Left))
    {
        const auto drag = ImGui::GetMouseDragDelta (ImGuiMouseButton_Left);
        const auto deltaX = static_cast<double> (drag.x) - previousPanDrag.x;
        const auto deltaY = static_cast<double> (drag.y) - previousPanDrag.y;
        interactionSession.view = panView (interactionSession.view, deltaX, deltaY);
        previousPanDrag = { drag.x, drag.y };
    }
    else if (! draggingNodeId.empty() && active && ImGui::IsMouseDragging (ImGuiMouseButton_Left))
    {
        const auto drag = ImGui::GetMouseDragDelta (ImGuiMouseButton_Left);
        dragCanvasDelta = { drag.x / interactionSession.view.scale,
                            drag.y / interactionSession.view.scale };
    }

    if (! draggingConnectionEndpoint.empty() && ImGui::IsMouseReleased (ImGuiMouseButton_Left))
    {
        const auto mouse = ImGui::GetMousePos();
        const auto hit = hitTestGraph (interactionSession.graph,
                                       nodeSpecs,
                                       interactionSession.view,
                                       { mouse.x - origin.x, mouse.y - origin.y });

        if (hit.kind == HitTestKind::inputPort)
            runInteractionCommand ("connect gesture", connectPorts (interactionSession, draggingConnectionEndpoint, hit.endpoint));
        else
        {
            pendingCreateSourceEndpoint = draggingConnectionEndpoint;
            pendingCreatePosition = canvasPointFromMouse (interactionSession.view, origin);
            ImGui::OpenPopup ("create-compatible-node");
            lastInteractionMessage = "create node search";
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
                                   moveNode (interactionSession, draggingNodeId, dragCanvasDelta.x, dragCanvasDelta.y));

        draggingNodeId.clear();
        dragCanvasDelta = {};
    }

    for (const auto& edge : interactionSession.graph.editorGraph.edges)
    {
        const auto from = portCenter (interactionSession.graph, nodeSpecs, edge.from);
        const auto to = portCenter (interactionSession.graph, nodeSpecs, edge.to);

        if (! from.ok || ! to.ok)
            continue;

        const auto p1 = toImVec (from.point, interactionSession.view, origin);
        const auto p2 = toImVec (to.point, interactionSession.view, origin);
        const auto c1 = ImVec2 (p1.x + 58.0f, p1.y);
        const auto c2 = ImVec2 (p2.x - 58.0f, p2.y);
        const auto selected = std::find (interactionSession.selectedEdgeIds.begin(),
                                         interactionSession.selectedEdgeIds.end(),
                                         edge.id) != interactionSession.selectedEdgeIds.end();
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
        const auto from = portCenter (interactionSession.graph, nodeSpecs, draggingConnectionEndpoint);

        if (from.ok)
        {
            const auto p1 = toImVec (from.point, interactionSession.view, origin);
            const auto p2 = toImVec (draggingConnectionPoint, interactionSession.view, origin);
            const auto sourceDataType = outputDataTypeForEndpoint (interactionSession.graph,
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
    const auto hoverHit = hovered ? hitTestGraph (interactionSession.graph,
                                                  nodeSpecs,
                                                  interactionSession.view,
                                                  { mousePosition.x - origin.x, mousePosition.y - origin.y })
                                  : HitTestResult {};
    const auto sourceDataType = draggingConnectionEndpoint.empty()
                                    ? std::string {}
                                    : outputDataTypeForEndpoint (interactionSession.graph,
                                                                 nodeSpecs,
                                                                 draggingConnectionEndpoint);

    for (const auto& node : interactionSession.graph.editorGraph.nodes)
    {
        const auto position = displayedPosition (node, draggingNodeId, dragCanvasDelta);
        const auto topLeft = toImVec (position, interactionSession.view, origin);
        const auto bottomRight = ImVec2 (topLeft.x + nodeWidth, topLeft.y + nodeHeight);
        const auto* spec = findNodeSpec (nodeSpecs, node.type);
        const auto primaryDataType = primaryDataTypeForSpec (spec);
        const auto selected = std::find (interactionSession.selectedNodeIds.begin(),
                                         interactionSession.selectedNodeIds.end(),
                                         node.id) != interactionSession.selectedNodeIds.end();
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
        drawList.AddText ({ topLeft.x + 12.0f, topLeft.y + 10.0f },
                          rgba (skin.label),
                          node.id.c_str());
        drawList.AddText ({ topLeft.x + 12.0f, topLeft.y + 32.0f },
                          rgba (skin.secondaryLabel),
                          node.type.c_str());

        if (spec == nullptr)
            continue;

        for (size_t index = 0; index < spec->inputs.size(); ++index)
        {
            const auto centerY = topLeft.y + 30.0f + static_cast<float> (index) * 18.0f;
            const auto compatible = sourceDataType.empty() || spec->inputs[index].dataType == sourceDataType;
            const auto portSkin = makeTooll3PortSkin (spec->inputs[index].dataType,
                                                      compatible,
                                                      ! draggingConnectionEndpoint.empty());
            drawPortStrip (drawList,
                           { topLeft.x, centerY - 7.0f },
                           { topLeft.x + static_cast<float> (portSkin.stripWidth), centerY + 7.0f },
                           portSkin);

            if (interactionSession.view.scale >= 0.80)
                drawList.AddText ({ topLeft.x + 10.0f, centerY - 7.0f },
                                  rgba (portSkin.label),
                                  spec->inputs[index].id.c_str());
        }

        for (size_t index = 0; index < spec->outputs.size(); ++index)
        {
            const auto centerY = topLeft.y + 30.0f + static_cast<float> (index) * 18.0f;
            const auto endpoint = node.id + "." + spec->outputs[index].id;
            const auto activePort = endpoint == draggingConnectionEndpoint;
            const auto portSkin = makeTooll3PortSkin (spec->outputs[index].dataType, true, activePort);
            drawPortStrip (drawList,
                           { bottomRight.x - static_cast<float> (portSkin.stripWidth), centerY - 7.0f },
                           { bottomRight.x, centerY + 7.0f },
                           portSkin);

            if (interactionSession.view.scale >= 0.80)
            {
                const auto labelSize = ImGui::CalcTextSize (spec->outputs[index].id.c_str());
                drawList.AddText ({ bottomRight.x - labelSize.x - 10.0f, centerY - 7.0f },
                                  rgba (portSkin.label),
                                  spec->outputs[index].id.c_str());
            }
        }
    }

    drawList.PopClipRect();
    ImGui::Dummy (ImVec2 (0.0f, 6.0f));
    drawCreateNodePopup (nodeSpecs);
    drawWorkspaceNodeBrowser (nodeSpecs);
}

void ImGuiSmokeOverlay::drawCreateNodePopup (const std::vector<NodeSpec>& nodeSpecs)
{
    if (pendingCreateSourceEndpoint.empty())
        return;

    ImGui::SetNextWindowPos (ImGui::GetMousePos(), ImGuiCond_Appearing);
    ImGui::SetNextWindowSize (ImVec2 (270.0f, 0.0f), ImGuiCond_Appearing);

    if (! ImGui::BeginPopup ("create-compatible-node"))
        return;

    const auto sourceDataType = outputDataTypeForEndpoint (interactionSession.graph,
                                                           nodeSpecs,
                                                           pendingCreateSourceEndpoint);
    const auto creationGates = makeNodeCreationGates (runtimeOpDiagnostics);
    ImGui::Text ("from %s", pendingCreateSourceEndpoint.c_str());
    ImGui::Text ("type %s", sourceDataType.empty() ? "unknown" : sourceDataType.c_str());
    ImGui::Separator();

    bool showedCandidate = false;

    for (const auto& spec : nodeSpecs)
    {
        if (! canCreateFromEndpoint (spec, sourceDataType))
            continue;

        showedCandidate = true;
        const auto label = spec.displayName + "##" + spec.type;
        const auto* diagnostic = diagnosticForNodeType (runtimeOpDiagnostics, spec.type);
        const auto canCreate = diagnostic == nullptr || runtimeOpDiagnosticAllowsCreation (*diagnostic);

        if (! canCreate)
            ImGui::BeginDisabled();

        if (ImGui::Selectable (label.c_str()))
        {
            const auto nodeId = makeUniqueNodeId (interactionSession.graph, spec.type);
            runInteractionCommand ("create " + spec.type,
                                   createNodeAndConnect (interactionSession,
                                                         nodeSpecs,
                                                         creationGates,
                                                         pendingCreateSourceEndpoint,
                                                         spec.type,
                                                         nodeId,
                                                         pendingCreatePosition));
            pendingCreateSourceEndpoint.clear();
            pendingCreatePosition = {};
            ImGui::CloseCurrentPopup();
        }

        if (! canCreate)
            ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::TextDisabled ("%s", spec.type.c_str());

        if (diagnostic != nullptr)
        {
            ImGui::SameLine();
            ImGui::TextColored (diagnosticTextColour (*diagnostic), "%s", diagnosticCreationLabel (*diagnostic));

            if (! canCreate)
            {
                ImGui::SameLine();
                const auto overrideLabel = "Override##connect-" + spec.type;

                if (ImGui::SmallButton (overrideLabel.c_str()))
                {
                    const auto nodeId = makeUniqueNodeId (interactionSession.graph, spec.type);
                    runInteractionCommand ("debug override " + spec.type,
                                           createNodeAndConnectWithDebugOverride (
                                               interactionSession,
                                               nodeSpecs,
                                               creationGates,
                                               pendingCreateSourceEndpoint,
                                               spec.type,
                                               nodeId,
                                               pendingCreatePosition,
                                               debugOverrideReasonFor (*diagnostic)));
                    pendingCreateSourceEndpoint.clear();
                    pendingCreatePosition = {};
                    ImGui::CloseCurrentPopup();
                }
            }
        }
    }

    if (! showedCandidate)
        ImGui::TextUnformatted ("no compatible node");

    if (ImGui::Button ("Cancel"))
    {
        pendingCreateSourceEndpoint.clear();
        pendingCreatePosition = {};
        lastInteractionMessage = "create node search: cancelled";
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void ImGuiSmokeOverlay::drawWorkspaceNodeBrowser (const std::vector<NodeSpec>& nodeSpecs)
{
    ImGui::SetNextWindowPos ({ static_cast<float> (nodeBrowserScreenPosition.x),
                               static_cast<float> (nodeBrowserScreenPosition.y) },
                             ImGuiCond_Appearing);
    ImGui::SetNextWindowSize (ImVec2 (340.0f, 390.0f), ImGuiCond_Appearing);

    if (! ImGui::BeginPopup ("workspace-node-browser"))
        return;

    ImGui::TextUnformatted ("Create Node");
    ImGui::InputText ("##node-browser-filter", &nodeBrowserFilter);
    ImGui::Separator();

    bool showedCandidate = false;
    const auto creationGates = makeNodeCreationGates (runtimeOpDiagnostics);

    for (const auto& spec : nodeSpecs)
    {
        if (! nodeSpecMatchesFilter (spec, nodeBrowserFilter))
            continue;

        showedCandidate = true;
        const auto label = spec.displayName + "##workspace-" + spec.type;
        const auto* diagnostic = diagnosticForNodeType (runtimeOpDiagnostics, spec.type);
        const auto canCreate = diagnostic == nullptr || runtimeOpDiagnosticAllowsCreation (*diagnostic);

        if (! canCreate)
            ImGui::BeginDisabled();

        if (ImGui::Selectable (label.c_str()))
        {
            const auto nodeId = makeUniqueNodeId (interactionSession.graph, spec.type);
            runInteractionCommand ("create " + spec.type,
                                   createNode (interactionSession,
                                               nodeSpecs,
                                               creationGates,
                                               spec.type,
                                               nodeId,
                                               pendingCreatePosition));
            nodeBrowserFilter.clear();
            ImGui::CloseCurrentPopup();
        }

        if (! canCreate)
            ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::TextDisabled ("%s", spec.type.c_str());

        if (diagnostic != nullptr)
        {
            ImGui::SameLine();
            ImGui::TextColored (diagnosticTextColour (*diagnostic), "%s", diagnosticCreationLabel (*diagnostic));

            if (! canCreate)
            {
                ImGui::SameLine();
                const auto overrideLabel = "Override##workspace-" + spec.type;

                if (ImGui::SmallButton (overrideLabel.c_str()))
                {
                    const auto nodeId = makeUniqueNodeId (interactionSession.graph, spec.type);
                    runInteractionCommand ("debug override " + spec.type,
                                           createNodeWithDebugOverride (
                                               interactionSession,
                                               nodeSpecs,
                                               creationGates,
                                               spec.type,
                                               nodeId,
                                               pendingCreatePosition,
                                               debugOverrideReasonFor (*diagnostic)));
                    nodeBrowserFilter.clear();
                    ImGui::CloseCurrentPopup();
                }
            }
        }
    }

    if (! showedCandidate)
        ImGui::TextDisabled ("empty");

    if (ImGui::Button ("Cancel"))
    {
        nodeBrowserFilter.clear();
        lastInteractionMessage = "node browser: cancelled";
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void ImGuiSmokeOverlay::drawInspectorPanel (const std::vector<NodeSpec>& nodeSpecs, const std::string& shaderStatus)
{
    ImGui::SeparatorText ("Inspector");

    const auto nodeId = selectedNodeId (interactionSession);
    const auto* node = findNode (interactionSession.graph, nodeId);
    const auto* spec = node == nullptr ? nullptr : findNodeSpec (nodeSpecs, node->type);

    if (node == nullptr || spec == nullptr)
    {
        ImGui::TextUnformatted ("select a node");
        return;
    }

    const auto policy = makeTooll3InspectorPolicy (node->type, true);
    ImGui::Text ("%s", node->id.c_str());
    ImGui::TextDisabled ("%s", node->type.c_str());

    if (const auto* diagnostic = diagnosticForNodeType (runtimeOpDiagnostics, node->type))
    {
        ImGui::TextColored (diagnosticTextColour (*diagnostic), "runtime: %s", diagnostic->browserLabel.c_str());
        ImGui::TextDisabled ("%s", diagnostic->inspectorDetail.c_str());
    }
    else
    {
        ImGui::TextDisabled ("runtime: not cataloged");
    }

    if (spec->params.empty())
    {
        ImGui::TextDisabled ("params: none");
    }
    else
    {
        for (const auto& param : spec->params)
        {
            const auto stored = paramValueForNode (*node, param.id);
            const auto state = stored.empty() ? "default" : "manual";
            drawInspectorRow (param.id, compactInspectorValue (param, stored), state);

            if (param.dataType == "text.glsl")
                continue;

            const auto label = "Set##param-" + node->id + "-" + param.id;
            if (ImGui::Button (label.c_str()))
                runInteractionCommand ("set " + param.id,
                                       setParam (interactionSession, node->id, param.id, demoValueForParam (param)));
        }
    }

    if (spec->inputs.empty())
    {
        ImGui::TextDisabled ("inputs: none");
    }
    else
    {
        for (const auto& input : spec->inputs)
        {
            const auto stored = bindingValueForPort (*node, input.id);
            const auto state = bindingModeForPort (*node, input.id);
            drawInspectorRow ("input " + input.id, stored.empty() ? input.dataType : stored, state);

            const auto label = "Bind##input-" + node->id + "-" + input.id;
            if (ImGui::Button (label.c_str()))
                runInteractionCommand ("bind " + input.id,
                                       setPortBinding (interactionSession,
                                                       node->id,
                                                       input.id,
                                                       "connected",
                                                       "shader1.output"));
        }
    }

    if (! policy.shaderSourceEditorVisible)
        return;

    if (shaderSourceDraftNodeId != node->id)
    {
        const auto storedSource = paramValueForNode (*node, "source");
        shaderSourceDraft = storedSource.empty() ? defaultFragmentShader() : storedSource;
        shaderSourceDraftNodeId = node->id;
    }

    ImGui::SeparatorText ("Shader Source");

    if (policy.compileStatusVisible)
        ImGui::TextWrapped ("%s", shaderStatus.c_str());

    ImGui::InputTextMultiline ("##shader-source",
                               &shaderSourceDraft,
                               { -FLT_MIN, 210.0f },
                               ImGuiInputTextFlags_AllowTabInput);

    if (ImGui::Button ("Apply Source"))
    {
        const auto result = setParam (interactionSession, node->id, "source", shaderSourceDraft);

        if (result.ok && onShaderSourceSubmitted != nullptr)
            onShaderSourceSubmitted (shaderSourceDraft);

        runInteractionCommand ("set source", result);
    }
}

void ImGuiSmokeOverlay::drawTracePanel()
{
    ImGui::SeparatorText ("Behavior Trace");

    if (ImGui::Button ("Run Trace"))
    {
        lastTraceReport = runBundledTraceFixture();
        hasTraceReport = true;
        lastInteractionMessage = lastTraceReport.ok ? "trace suite: ok" : "trace suite: failed";
    }

    ImGui::SameLine();

    if (! hasTraceReport)
    {
        ImGui::TextDisabled ("not run");
        return;
    }

    ImGui::Text ("%s  traces %d  commands %d",
                 lastTraceReport.ok ? "ok" : "failed",
                 lastTraceReport.tracesRun,
                 static_cast<int> (lastTraceReport.commandsObserved.size()));

    if (! lastTraceReport.errors.empty())
        ImGui::TextWrapped ("error: %s", lastTraceReport.errors.front().c_str());
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
