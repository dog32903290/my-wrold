#include "CanvasGeometry.h"
#include "GraphContract.h"
#include "NodeSpec.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (condition)
        return;

    std::cerr << "FAIL: " << message << '\n';
    std::exit (1);
}
}

int main()
{
    const auto& geometry = myworld::defaultCanvasGeometry();
    expect (geometry.nodeWidth == 140.0, "default node width");
    expect (geometry.nodeHeight == 60.0, "default node height");
    expect (geometry.firstPortOffsetY == 30.0, "default first port offset");
    expect (geometry.portSpacing == 18.0, "default port spacing");

    const auto graph = myworld::makeDefaultShaderOutputGraph();
    const auto& shader = graph.editorGraph.nodes.front();

    const auto bounds = myworld::canvasNodeBounds (shader);
    expect (bounds.x == 80.0, "node bounds x");
    expect (bounds.y == 80.0, "node bounds y");
    expect (bounds.width == geometry.nodeWidth, "node bounds width follows contract");
    expect (bounds.height == geometry.nodeHeight, "node bounds height follows contract");

    expect (myworld::canvasPointInNodeBody ({ 80.0, 80.0 }, shader), "top-left point inside node");
    expect (myworld::canvasPointInNodeBody ({ 220.0, 140.0 }, shader), "bottom-right point inside node");
    expect (! myworld::canvasPointInNodeBody ({ 220.1, 140.0 }, shader), "point outside node width");

    const auto outputPort = myworld::canvasPortCenterForIndex (shader, "out", 0);
    expect (outputPort.x == 220.0, "output port x follows node width");
    expect (outputPort.y == 110.0, "first output port y");

    const auto secondInputPort = myworld::canvasPortCenterForIndex (shader, "in", 1);
    expect (secondInputPort.x == 80.0, "input port x follows node left");
    expect (secondInputPort.y == 128.0, "second input port y follows spacing");

    myworld::NodeSpec attackSpec;
    attackSpec.type = "compound.attack";
    attackSpec.displayName = "Attack";
    attackSpec.inputs.push_back ({ "audio.onset_event", "audio.onset_event", "event.pulse", "in" });
    attackSpec.outputs.push_back ({ "onset_event", "onset_event", "event.pulse", "out" });
    attackSpec.outputs.push_back ({ "attack_value", "attack_value", "signal.float", "out" });
    attackSpec.outputs.push_back ({ "attack_envelope", "attack_envelope", "signal.float", "out" });
    attackSpec.outputs.push_back ({ "confidence", "confidence", "signal.float", "out" });

    myworld::GraphNode attackNode;
    attackNode.id = "compound_attack4";
    attackNode.type = "compound.attack";
    attackNode.position = { 24.0, 32.0 };

    const auto attackLayout = myworld::canvasNodeSurfaceGeometry (attackNode, &attackSpec);
    expect (attackLayout.bounds.width > geometry.nodeWidth, "long analyzer labels widen node surface");
    expect (attackLayout.bounds.height > geometry.nodeHeight, "four output rows grow node surface height");
    expect (attackLayout.outputRows.size() == attackSpec.outputs.size(), "output rows follow spec outputs");

    const auto surfaceBottom = attackLayout.bounds.y + attackLayout.bounds.height;
    for (const auto& row : attackLayout.outputRows)
    {
        expect (row.rowBounds.x >= attackLayout.bounds.x, "output row x stays inside node");
        expect (row.rowBounds.x + row.rowBounds.width <= attackLayout.bounds.x + attackLayout.bounds.width,
                "output row width stays inside node");
        expect (row.center.y >= attackLayout.bounds.y, "output center y inside node");
        expect (row.center.y + 7.0 <= surfaceBottom, "output row hit strip stays inside node bottom");
        expect (row.labelBounds.x >= attackLayout.bounds.x, "output label x stays inside node");
        expect (row.labelBounds.x + row.labelBounds.width <= attackLayout.bounds.x + attackLayout.bounds.width,
                "output label width stays inside node");
    }

    const auto lastOutput = myworld::canvasPortCenterForIndex (attackNode, &attackSpec, "out", 3);
    expect (lastOutput.y == attackLayout.outputRows.back().center.y, "port center uses surface row geometry");
    expect (myworld::canvasPointInNodeBody ({ lastOutput.x - 1.0, lastOutput.y }, attackNode, &attackSpec),
            "dynamic node body contains last output row");

    std::cout << "canvas geometry contract tests ok\n";
    return 0;
}
