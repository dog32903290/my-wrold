#include "CanvasGeometry.h"
#include "GraphContract.h"

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

    std::cout << "canvas geometry contract tests ok\n";
    return 0;
}
