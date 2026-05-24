#include "CanvasGeometry.h"
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

    myworld::NodeSpec attackSpec;
    attackSpec.type = "compound.attack";
    attackSpec.displayName = "Attack";
    attackSpec.inputs.push_back ({ "audio.onset_event", "audio.onset_event", "event.pulse", "in" });
    attackSpec.outputs.push_back ({ "onset_event", "onset_event", "event.pulse", "out" });
    attackSpec.outputs.push_back ({ "attack_value", "attack_value", "signal.float", "out" });
    attackSpec.outputs.push_back ({ "attack_envelope", "attack_envelope", "signal.float", "out" });
    attackSpec.outputs.push_back ({ "confidence", "confidence", "signal.float", "out" });

    myworld::GraphContract analyzerGraph;
    analyzerGraph.editorGraph.nodes.push_back ({ "compound_attack4", "compound.attack", {}, { 40.0, 50.0 } });
    analyzerGraph.runtimeGraph.nodes = analyzerGraph.editorGraph.nodes;

    const auto analyzerSpecs = std::vector<myworld::NodeSpec> { attackSpec };
    const auto finalOutput = myworld::canvasPortCenterForIndex (analyzerGraph.editorGraph.nodes.front(),
                                                                &attackSpec,
                                                                "out",
                                                                3);
    hit = myworld::hitTestGraph (analyzerGraph, analyzerSpecs, {}, { finalOutput.x, finalOutput.y });
    expect (hit.kind == myworld::HitTestKind::outputPort, "dynamic analyzer output port hit");
    expect (hit.endpoint == "compound_attack4.confidence", "dynamic analyzer output endpoint");

    hit = myworld::hitTestGraph (analyzerGraph, analyzerSpecs, {}, { finalOutput.x - 16.0, finalOutput.y });
    expect (hit.kind == myworld::HitTestKind::nodeBody, "dynamic analyzer body includes final output row");
    expect (hit.nodeId == "compound_attack4", "dynamic analyzer body node id");

    std::cout << "node hit tests ok\n";
    return 0;
}
