#include "ImGuiSmokeOverlay.h"

#include "GraphEndpoint.h"
#include "ImGuiSmokeOverlayHelpers.h"
#include "NodeSpec.h"
#include "ParameterRowState.h"
#include "Tooll3SkinContract.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <cfloat>

namespace myworld
{
using namespace imgui_overlay;

void ImGuiSmokeOverlay::drawInspectorPanel (const std::vector<NodeSpec>& nodeSpecs, const std::string& shaderStatus)
{
    ImGui::SeparatorText ("Inspector");

    const auto nodeId = selectedNodeId (interactionSession);
    const auto* node = findEditorNode (interactionSession.graph, nodeId);
    const auto* spec = node == nullptr ? nullptr : findNodeSpec (nodeSpecs, node->type);

    if (node == nullptr || spec == nullptr)
    {
        ImGui::TextUnformatted ("select a node");
        return;
    }

    const auto policy = makeTooll3InspectorPolicy (node->type, true);
    const auto rowStates = parameterRowsForNode (interactionSession.graph, *node, *spec);
    const auto rowFor = [&rowStates] (const std::string& rowId) -> const ParameterRowState*
    {
        for (const auto& row : rowStates)
            if (row.id == rowId)
                return &row;

        return nullptr;
    };

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
            const auto* row = rowFor ("param." + param.id);
            const auto value = row == nullptr ? param.defaultValue : row->value;
            const auto state = row == nullptr ? "missing" : row->stateLabel;
            drawInspectorRow (param.id, compactInspectorValue (param, value), state);

            if (param.dataType == "text.glsl")
                continue;

            const auto label = "Set##param-" + node->id + "-" + param.id;
            if (ImGui::Button (label.c_str()))
                runInteractionCommand ("set " + param.id,
                                       setParam (interactionSession, node->id, param.id, demoValueForParam (param)));

            ImGui::SameLine();
            const auto resetLabel = "Reset##param-" + node->id + "-" + param.id;
            if (ImGui::Button (resetLabel.c_str()))
                runInteractionCommand ("reset " + param.id,
                                       resetParam (interactionSession, node->id, param.id));
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
            const auto* row = rowFor ("input." + input.id);
            drawInspectorRow ("input " + input.id,
                              row == nullptr ? input.dataType : row->value,
                              row == nullptr ? "missing" : row->stateLabel);

            const auto label = "Bind##input-" + node->id + "-" + input.id;
            if (ImGui::Button (label.c_str()))
                runInteractionCommand ("bind " + input.id,
                                       setPortBinding (interactionSession,
                                                       node->id,
                                                       input.id,
                                                       "connected",
                                                       "shader1.output"));

            ImGui::SameLine();
            const auto resetLabel = "Reset##input-" + node->id + "-" + input.id;
            if (ImGui::Button (resetLabel.c_str()))
                runInteractionCommand ("reset " + input.id,
                                       resetPortBinding (interactionSession, node->id, input.id));
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
}
