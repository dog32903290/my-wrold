#pragma once

#include "InteractionContract.h"
#include "RuntimeRegistry.h"
#include "Tooll3SkinContract.h"

#include <imgui.h>

#include <string>
#include <vector>

namespace myworld
{
struct GraphNode;
struct ParamSpec;

namespace imgui_overlay
{
ImVec2 rectMin (Tooll3SkinRect rect);
ImVec2 rectMax (Tooll3SkinRect rect);
ImU32 rgba (int r, int g, int b, int a);
ImU32 rgba (Tooll3SkinColor colour);
ImVec4 vec4 (Tooll3SkinColor colour);

std::string patchPathText (const GraphSession& session);
const RuntimeOpModuleDiagnostic* diagnosticForNodeType (const std::vector<RuntimeOpModuleDiagnostic>& diagnostics,
                                                        const std::string& nodeType);
bool diagnosticIsReady (const RuntimeOpModuleDiagnostic& diagnostic);
ImVec4 diagnosticTextColour (const RuntimeOpModuleDiagnostic& diagnostic);
std::vector<NodeCreationGate> makeNodeCreationGates (const std::vector<RuntimeOpModuleDiagnostic>& diagnostics);
const char* diagnosticCreationLabel (const RuntimeOpModuleDiagnostic& diagnostic);
std::string debugOverrideReasonFor (const RuntimeOpModuleDiagnostic& diagnostic);
void drawRuntimeDiagnosticSummary (const RuntimeOpModuleDiagnostic& diagnostic);

ImVec2 toImVec (CanvasPoint point, const CanvasViewState& view, ImVec2 origin);
CanvasPoint displayedPosition (const GraphNode& node, const std::string& draggingNodeId, CanvasPoint dragCanvasDelta);
std::string selectedEdgeId (const GraphSession& session);
std::string selectedNodeId (const GraphSession& session);
bool nodeIsCompound (const GraphContract& graph, const std::string& nodeId);
bool selectedNodeIsCompound (const GraphSession& session);
CommandResult deleteSelectedGraphItem (GraphSession& session);

std::string paramValueForNode (const GraphNode& node, const std::string& paramId);
std::string bindingValueForPort (const GraphNode& node, const std::string& portId);
std::string bindingModeForPort (const GraphNode& node, const std::string& portId);
std::string demoValueForParam (const ParamSpec& param);
void drawPortStrip (ImDrawList& drawList, ImVec2 min, ImVec2 max, const Tooll3PortSkin& skin);
std::string compactInspectorValue (const ParamSpec& param, const std::string& value);
void drawInspectorRow (const std::string& label, const std::string& value, const std::string& state);

CanvasPoint canvasPointFromMouse (const CanvasViewState& view, ImVec2 origin);
CanvasViewState defaultInteractionView (ImVec2 canvasSize);
BehaviorTraceReport runBundledTraceFixture();
GraphSession makeDefaultOverlaySession();
}
}
