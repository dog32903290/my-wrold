#include "Tooll3SkinContract.h"

#include <cstdlib>
#include <iostream>
#include <string>

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
}

int main()
{
    outputSurfaceOwnsTheWorkspace();
    shaderSourceIsSelectedNodeDetail();
    canvasIsNotAFramedSubPanel();
    edgePanelsAndTransportExist();
    return 0;
}
