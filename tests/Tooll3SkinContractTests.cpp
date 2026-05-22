#include "Tooll3SkinContract.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace
{
void require (bool condition, const std::string& message)
{
    if (condition)
        return;

    std::cerr << message << "\n";
    std::exit (1);
}

void outputSurfaceOwnsTheWorkspace()
{
    const auto layout = myworld::makeTooll3SkinLayout (1440.0, 860.0);
    require (layout.outputAreaRatio >= 0.70, "output surface should own at least 70 percent of workspace");
    require (layout.outputSurface.width > layout.leftRail.width * 2.0, "output surface should dominate the left rail");
}

void shaderSourceIsSelectedNodeDetail()
{
    const auto policy = myworld::makeTooll3SkinPolicy();
    require (! policy.globalShaderSourceVisible, "shader source should not be globally visible");
    require (policy.shaderSourceVisibility == "selected_shader_node", "shader source should be selected-node detail");
}

void canvasIsNotAFramedSubPanel()
{
    const auto policy = myworld::makeTooll3SkinPolicy();
    require (policy.outputAsWorkspaceBackground, "output should be the workspace background");
    require (! policy.nodeCanvasIsFramedPanel, "node canvas should not be a framed sub-panel");
    require (policy.gridWhenOutputActive == "muted", "grid should be muted while output background is active");
}

void edgePanelsAndTransportExist()
{
    const auto layout = myworld::makeTooll3SkinLayout (1280.0, 760.0);
    require (layout.leftRail.width >= 220.0, "left rail should exist");
    require (layout.bottomStrip.height >= 28.0, "bottom transport strip should exist");
    require (layout.outputSurface.height > layout.bottomStrip.height * 10.0, "bottom strip should not dominate output");
}

bool sameColour (myworld::Tooll3SkinColor a, myworld::Tooll3SkinColor b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

void nodeColourFollowsRoleAndType()
{
    const auto shader = myworld::makeTooll3NodeSkin ("shader.fragment", "texture.rgba", false, false);
    const auto output = myworld::makeTooll3NodeSkin ("output.preview", "texture.rgba", false, false);
    const auto audio = myworld::makeTooll3NodeSkin ("audio.input", "audio.channels", false, false);
    const auto signal = myworld::makeTooll3NodeSkin ("analyzer.rms", "signal.float", false, false);

    require (shader.role == "shader", "shader node role");
    require (output.role == "output", "output node role");
    require (audio.role == "audio", "audio node role");
    require (signal.role == "signal", "signal node role");
    require (! sameColour (shader.fill, output.fill), "shader and output should scan as different roles");
    require (! sameColour (audio.fill, signal.fill), "audio stream and signal value should scan differently");
    require (shader.inputStrip.a > 0 && shader.outputStrip.a > 0, "node skins include port strip colours");
}

void nodeStateDoesNotChangeGeometry()
{
    const auto base = myworld::makeTooll3NodeSkin ("shader.fragment", "texture.rgba", false, false);
    const auto selected = myworld::makeTooll3NodeSkin ("shader.fragment", "texture.rgba", true, false);
    const auto hovered = myworld::makeTooll3NodeSkin ("shader.fragment", "texture.rgba", false, true);

    require (base.cornerRadius == 0.0, "Tooll3 node skin is square");
    require (selected.cornerRadius == base.cornerRadius, "selected node keeps same radius");
    require (hovered.cornerRadius == base.cornerRadius, "hovered node keeps same radius");
    require (selected.layoutStableOnSelection, "selected state must not resize nodes");
    require (hovered.layoutStableOnHover, "hover state must not resize nodes");
    require (selected.borderWidth > base.borderWidth, "selected state is an outline change");
}

void portStripsUseDataTypeColour()
{
    const auto texture = myworld::makeTooll3PortSkin ("texture.rgba", true, false);
    const auto command = myworld::makeTooll3PortSkin ("command.graph", true, true);
    const auto muted = myworld::makeTooll3PortSkin ("audio.mono", false, false);

    require (texture.stripWidth >= 4.0, "ports are visible side strips");
    require (! sameColour (texture.strip, command.strip), "port strip colour follows data type");
    require (command.compatibleHighlight, "active compatible port is highlighted");
    require (muted.muted, "incompatible port is muted");
    require (muted.strip.a < texture.strip.a, "muted port has lower opacity");
}

void connectionColourFollowsTypeAndCompatibility()
{
    const auto texture = myworld::makeTooll3ConnectionSkin ("texture.rgba", false, true);
    const auto selected = myworld::makeTooll3ConnectionSkin ("texture.rgba", true, true);
    const auto audio = myworld::makeTooll3ConnectionSkin ("audio.mono", false, true);
    const auto incompatible = myworld::makeTooll3ConnectionSkin ("audio.mono", false, false);

    require (! sameColour (texture.color, audio.color), "connection colour follows data type");
    require (selected.thickness > texture.thickness, "selected connection thickens");
    require (! incompatible.compatible, "incompatible connection is marked");
    require (incompatible.color.a < audio.color.a, "incompatible connection is visually muted");
}

void shaderSourceBelongsToSelectedNodeInspector()
{
    const auto none = myworld::makeTooll3InspectorPolicy ({}, false);
    const auto shader = myworld::makeTooll3InspectorPolicy ("shader.fragment", true);
    const auto output = myworld::makeTooll3InspectorPolicy ("output.preview", true);

    require (! none.panelVisible, "inspector is quiet without selection");
    require (shader.panelVisible, "selected shader shows inspector");
    require (shader.shaderSourceEditorVisible, "selected shader owns source editor");
    require (shader.compileStatusVisible, "selected shader shows compile status");
    require (shader.shaderSourceOwner == "selected_shader_node", "source owner is selected shader node");
    require (! shader.globalShaderSourceVisible, "shader source is not global");
    require (! output.shaderSourceEditorVisible, "non-shader node does not show source editor");
}

void parameterRowsExposeValueState()
{
    const auto defaultRow = myworld::makeTooll3InspectorRowSkin ("default");
    const auto manualRow = myworld::makeTooll3InspectorRowSkin ("manual");
    const auto connectedRow = myworld::makeTooll3InspectorRowSkin ("connected");
    const auto animatedRow = myworld::makeTooll3InspectorRowSkin ("animated");

    require (defaultRow.stateLabel == "default", "default row label");
    require (manualRow.stateLabel == "manual", "manual row label");
    require (connectedRow.stateLabel == "connected", "connected row label");
    require (animatedRow.stateLabel == "animated", "animated row label");
    require (! sameColour (defaultRow.stateAccent, manualRow.stateAccent), "manual differs from default");
    require (! sameColour (connectedRow.stateAccent, manualRow.stateAccent), "connected differs from manual");
    require (connectedRow.connectionVisible, "connected row exposes connection state");
    require (defaultRow.layoutStableOnStateChange, "row state does not resize layout");
    require (animatedRow.layoutStableOnStateChange, "animated state does not resize layout");
}

void workspaceContextMenuIsCommandBacked()
{
    const auto menu = myworld::makeTooll3WorkspaceMenuPolicy();
    require (menu.rightClickOpensNodeBrowser, "right click opens node browser");
    require (menu.onlyOnEmptyCanvas, "context menu opens only on empty canvas");
    require (menu.popupAnchoredToGesture, "node browser is anchored to gesture position");
    require (menu.createsNodesThroughCommandGraph, "node browser create goes through commandGraph");
    require (menu.supportsSearchFilter, "node browser has search/filter field");
    require (menu.rejectsDanglingEdges, "node browser does not create dangling edges");
}

void leftRailHasTooll3TabsAndSelectionContext()
{
    const auto rail = myworld::makeTooll3LeftRailPolicy();
    const std::vector<std::string> expectedTabs { "Presets", "Snapshots", "Library" };

    require (rail.tabs == expectedTabs, "left rail primary tabs");
    require (rail.emptyStateIsQuiet, "left rail empty state is quiet");
    require (rail.selectionContextAttached, "selection inspector remains attached to left rail");
    require (rail.collapsibleLater, "left rail can collapse later without changing center workspace");
}

void bottomTransportIsWorkspaceBoundary()
{
    const auto transport = myworld::makeTooll3TransportPolicy();
    require (transport.hasTimeReadout, "transport has time readout");
    require (transport.hasCommandStrip, "transport has command strip");
    require (transport.hasTraceStatus, "transport has trace status");
    require (transport.timelineEditingParked, "timeline editing is parked");
    require (transport.usesRealAppTime, "transport time comes from app time");
    require (transport.layoutStable, "transport layout is stable");
}
}

int main()
{
    outputSurfaceOwnsTheWorkspace();
    shaderSourceIsSelectedNodeDetail();
    canvasIsNotAFramedSubPanel();
    edgePanelsAndTransportExist();
    nodeColourFollowsRoleAndType();
    nodeStateDoesNotChangeGeometry();
    portStripsUseDataTypeColour();
    connectionColourFollowsTypeAndCompatibility();
    shaderSourceBelongsToSelectedNodeInspector();
    parameterRowsExposeValueState();
    workspaceContextMenuIsCommandBacked();
    leftRailHasTooll3TabsAndSelectionContext();
    bottomTransportIsWorkspaceBoundary();
    return 0;
}
