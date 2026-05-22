#include "GraphContract.h"
#include "InteractionContract.h"
#include "NodeSpec.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}
}

int main()
{
    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    const auto specs = myworld::makeSeedNodeSpecs();

    myworld::CanvasViewState view;
    view.scale = 1.0;

    auto hit = myworld::hitTestGraph (session.graph, specs, view, { 110.0, 100.0 });
    expect (hit.kind == myworld::HitTestKind::nodeBody, "shader body hit");
    expect (hit.nodeId == "shader1", "shader body node id");

    hit = myworld::hitTestGraph (session.graph, specs, view, { 220.0, 110.0 });
    expect (hit.kind == myworld::HitTestKind::outputPort, "shader output port hit");
    expect (hit.endpoint == "shader1.output", "shader output endpoint");

    view = myworld::panView (view, 20.0, 10.0);
    view = myworld::zoomViewAround (view, 2.0, { 0.0, 0.0 });
    const auto shaderBodyScreen = myworld::canvasToScreen (view, { 110.0, 100.0 });
    hit = myworld::hitTestGraph (session.graph, specs, view, shaderBodyScreen);
    expect (hit.kind == myworld::HitTestKind::nodeBody, "shader body hit after pan zoom");

    expect (myworld::moveNode (session, "shader1", 40.0, 20.0).ok, "move node");
    const auto movedOutput = myworld::portCenter (session.graph, specs, "shader1.output");
    expect (movedOutput.ok, "moved output exists");
    expect (movedOutput.point.x > 250.0, "moved output follows node x");

    hit = myworld::hitTestGraph (session.graph, specs, {}, { 265.0, 130.0 });
    expect (hit.kind == myworld::HitTestKind::outputPort, "moved output hit");

    hit = myworld::hitTestGraph (session.graph, specs, {}, { 260.0, 110.0 });
    expect (hit.kind == myworld::HitTestKind::edge, "edge hit");
    expect (hit.edgeId == "edge.shader1.output.out1.input", "edge id hit");

    std::cout << "node hit tests ok\n";
    return 0;
}
