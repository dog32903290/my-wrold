#include "ImGuiSmokeOverlayHelpers.h"

#include "CanvasGeometry.h"
#include "GraphEndpoint.h"
#include "NodeSpec.h"

#include <algorithm>
#include <sstream>

namespace myworld::imgui_overlay
{
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
    const auto* node = findEditorNode (graph, nodeId);
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
    followSelectedOutputNode (session.outputView, session.graph, session.selectedNodeIds);
    return session;
}
}
