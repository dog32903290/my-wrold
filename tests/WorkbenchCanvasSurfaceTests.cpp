#include "WorkbenchCanvasSurface.h"

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

const myworld::WorkbenchCanvasSurfaceRow& rowAt (const myworld::WorkbenchCanvasSurface& surface,
                                                 std::size_t index)
{
    expect (surface.rows.size() > index, "surface row index exists");
    return surface.rows[index];
}
}

int main()
{
    myworld::WorkbenchSessionSnapshot blocked;
    blocked.status = "blocked";
    blocked.message = "no current workbench session";

    const auto blockedSurface = myworld::makeWorkbenchCanvasSurface (blocked);
    expect (! blockedSurface.ok, "blocked surface ok");
    expect (blockedSurface.headline == "canvas blocked: no current workbench session", "blocked headline");
    expect (blockedSurface.rows.size() == 6, "blocked row count");
    expect (rowAt (blockedSurface, 0).text == "canvas no workbench session", "blocked canvas row");
    expect (rowAt (blockedSurface, 1).text == "nodes 0", "blocked nodes row");
    expect (rowAt (blockedSurface, 2).text == "routes 0", "blocked routes row");
    expect (rowAt (blockedSurface, 3).text == "bounds none", "blocked bounds row");
    expect (rowAt (blockedSurface, 4).text == "route none", "blocked route row");
    expect (rowAt (blockedSurface, 5).text == "points none", "blocked points row");

    myworld::WorkbenchSessionSnapshot ready;
    ready.ok = true;
    ready.status = "ready";
    ready.documentId = "patch.canvas1-main";
    ready.editorNodeCount = 2;
    ready.editorEdgeCount = 1;
    ready.editorNodes = {
        { "shader1", "shader.fragment", 80.0, 80.0, false, 4, 0, 0 },
        { "out1", "output.preview", 320.0, 80.0, false, 0, 0, 0 }
    };
    ready.editorEdges = {
        { "edge.shader1.output.out1.input",
          "shader1.output",
          "out1.input",
          "texture.rgba",
          "continuous" }
    };

    const auto surface = myworld::makeWorkbenchCanvasSurface (ready);
    expect (surface.ok, "ready surface ok");
    expect (surface.headline == "canvas ready patch.canvas1-main nodes 2 routes 1",
            "ready headline");
    expect (surface.nodeSurfaces.size() == 2, "node surface count");
    expect (surface.edgeRoutes.size() == 1, "edge route count");

    const auto& shaderNode = surface.nodeSurfaces.front();
    expect (shaderNode.id == "shader1", "first node id");
    expect (shaderNode.type == "shader.fragment", "first node type");
    expect (shaderNode.x == 80.0, "first node x");
    expect (shaderNode.y == 80.0, "first node y");
    expect (shaderNode.width == 140.0, "first node width");
    expect (shaderNode.height == 60.0, "first node height");

    const auto& route = surface.edgeRoutes.front();
    expect (route.id == "edge.shader1.output.out1.input", "route id");
    expect (route.from == "shader1.output", "route from");
    expect (route.to == "out1.input", "route to");
    expect (route.fromX == 220.0, "route from x");
    expect (route.fromY == 110.0, "route from y");
    expect (route.toX == 320.0, "route to x");
    expect (route.toY == 110.0, "route to y");

    expect (surface.rows.size() == 6, "ready row count");
    expect (rowAt (surface, 0).text == "canvas patch.canvas1-main", "ready canvas row");
    expect (rowAt (surface, 1).text == "nodes 2", "ready nodes row");
    expect (rowAt (surface, 2).text == "routes 1", "ready routes row");
    expect (rowAt (surface, 3).text == "bounds shader1 80,80 140x60", "ready bounds row");
    expect (rowAt (surface, 4).text == "route shader1.output -> out1.input", "ready route row");
    expect (rowAt (surface, 5).text == "points 220,110 -> 320,110", "ready points row");

    std::cout << "workbench canvas surface ok\n";
    return 0;
}
