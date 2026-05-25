#include "ImGuiSmokeOverlay.h"

#include "GraphEndpoint.h"
#include "ImGuiSmokeOverlayHelpers.h"
#include "NodeSpec.h"
#include "ParameterControl.h"
#include "ParameterRowState.h"
#include "Tooll3SkinContract.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <cfloat>
#include <cstdlib>

namespace myworld
{
using namespace imgui_overlay;

namespace
{
bool parseFloatForControl (const std::string& text, float& value)
{
    char* end = nullptr;
    const auto parsed = std::strtof (text.c_str(), &end);
    if (end == text.c_str())
        return false;

    value = parsed;
    return true;
}

bool parseIntForControl (const std::string& text, int& value)
{
    char* end = nullptr;
    const auto parsed = std::strtol (text.c_str(), &end, 10);
    if (end == text.c_str())
        return false;

    value = static_cast<int> (parsed);
    return true;
}
}

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
        std::string currentParamGroup;
        for (const auto& param : spec->params)
        {
            const auto* row = rowFor ("param." + param.id);
            if (row == nullptr)
                continue;

            const auto group = row->group.empty() ? std::string { "Parameters" } : row->group;
            if (group != currentParamGroup)
            {
                ImGui::SeparatorText (group.c_str());
                currentParamGroup = group;
            }

            const auto value = row->value;
            drawInspectorRow (param.id, compactInspectorValue (param, value), row->stateLabel);
            if (! row->description.empty() && ImGui::IsItemHovered())
                ImGui::SetTooltip ("%s", row->description.c_str());

            if (param.dataType == "text.glsl")
                continue;

            const auto control = parameterControlForParam (param);
            ImGui::PushID (("typed-param-" + node->id + "-" + param.id).c_str());

            switch (control.kind)
            {
                case ParameterControlKind::floatSlider:
                {
                    float next = 0.0f;
                    if (! parseFloatForControl (value.empty() ? param.defaultValue : value, next))
                        next = 0.0f;

                    const auto changed = control.hasRange
                                             ? ImGui::SliderFloat ("##value",
                                                                  &next,
                                                                  static_cast<float> (control.minimum),
                                                                  static_cast<float> (control.maximum))
                                             : ImGui::InputFloat ("##value", &next);

                    if (changed)
                        runInteractionCommand ("set " + param.id,
                                               setTypedParam (interactionSession, node->id, param, std::to_string (next)));
                    break;
                }
                case ParameterControlKind::integerStepper:
                {
                    int next = 0;
                    if (! parseIntForControl (value.empty() ? param.defaultValue : value, next))
                        next = 0;

                    if (ImGui::InputInt ("##value", &next))
                        runInteractionCommand ("set " + param.id,
                                               setTypedParam (interactionSession, node->id, param, std::to_string (next)));
                    break;
                }
                case ParameterControlKind::toggle:
                {
                    auto next = value == "true" || value == "1" || value == "on";
                    if (ImGui::Checkbox ("##value", &next))
                        runInteractionCommand ("set " + param.id,
                                               setTypedParam (interactionSession, node->id, param, next ? "true" : "false"));
                    break;
                }
                case ParameterControlKind::enumMenu:
                {
                    const auto preview = value.empty() ? param.defaultValue : value;
                    if (ImGui::BeginCombo ("##value", preview.c_str()))
                    {
                        for (const auto& option : control.options)
                        {
                            if (ImGui::Selectable (option.c_str(), option == preview))
                                runInteractionCommand ("set " + param.id,
                                                       setTypedParam (interactionSession, node->id, param, option));
                        }

                        ImGui::EndCombo();
                    }
                    break;
                }
                case ParameterControlKind::vectorEditor:
                case ParameterControlKind::textField:
                case ParameterControlKind::pathField:
                {
                    auto next = value;
                    if (ImGui::InputText ("##value", &next, ImGuiInputTextFlags_EnterReturnsTrue))
                        runInteractionCommand ("set " + param.id,
                                               setTypedParam (interactionSession, node->id, param, next));
                    break;
                }
                case ParameterControlKind::multilineText:
                {
                    auto next = value;
                    if (ImGui::InputTextMultiline ("##value",
                                                   &next,
                                                   { -FLT_MIN, 96.0f },
                                                   ImGuiInputTextFlags_AllowTabInput))
                    {
                        runInteractionCommand ("set " + param.id,
                                               setTypedParam (interactionSession, node->id, param, next));
                    }
                    break;
                }
                case ParameterControlKind::unsupported:
                {
                    const auto label = "Set##value";
                    if (ImGui::Button (label))
                        runInteractionCommand ("set " + param.id,
                                               setParam (interactionSession, node->id, param.id, demoValueForParam (param)));
                    break;
                }
            }

            ImGui::PopID();

            if (control.kind != ParameterControlKind::multilineText)
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
