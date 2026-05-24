#include "ImGuiSmokeOverlay.h"

#include "GraphEndpoint.h"
#include "ImGuiSmokeOverlayHelpers.h"
#include "NodeSpec.h"
#include "NodeSpecQueries.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace myworld
{
using namespace imgui_overlay;

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
}
