#include "WorkbenchCookPlanSurface.h"

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

const myworld::WorkbenchCookPlanSurfaceRow& rowAt (const myworld::WorkbenchCookPlanSurface& surface,
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

    const auto blockedSurface = myworld::makeWorkbenchCookPlanSurface (blocked);
    expect (! blockedSurface.ok, "blocked surface ok");
    expect (blockedSurface.headline == "cook blocked: no current workbench session",
            "blocked headline");
    expect (blockedSurface.cookOrder.empty(), "blocked cook order");
    expect (blockedSurface.rows.size() == 6, "blocked row count");
    expect (rowAt (blockedSurface, 0).text == "cook no workbench session", "blocked cook row");
    expect (rowAt (blockedSurface, 1).text == "order none", "blocked order row");
    expect (rowAt (blockedSurface, 2).text == "target none", "blocked target row");
    expect (rowAt (blockedSurface, 3).text == "graph 0 nodes/0 edges", "blocked graph row");
    expect (rowAt (blockedSurface, 4).text == "readiness blocked", "blocked readiness row");
    expect (rowAt (blockedSurface, 5).text == "execution parked", "blocked execution row");

    myworld::WorkbenchSessionSnapshot ready;
    ready.ok = true;
    ready.status = "ready";
    ready.documentId = "patch.cook1-main";
    ready.runtimeNodeCount = 2;
    ready.runtimeEdgeCount = 1;
    ready.activeOutputNodeId = "out1";
    ready.runtimeNodes = {
        { "shader1", "shader.fragment", 80.0, 80.0, false, 4, 0, 0 },
        { "out1", "output.preview", 320.0, 80.0, false, 0, 0, 0 }
    };
    ready.runtimeEdges = {
        { "edge.shader1.output.out1.input", "shader1.output", "out1.input", "texture.rgba", "continuous" }
    };

    const auto readySurface = myworld::makeWorkbenchCookPlanSurface (ready);
    expect (readySurface.ok, "ready surface ok");
    expect (readySurface.headline == "cook ready patch.cook1-main order shader1 -> out1",
            "ready headline");
    expect (readySurface.cookOrder.size() == 2, "ready cook order count");
    expect (readySurface.cookOrder[0] == "shader1", "ready cook order first node");
    expect (readySurface.cookOrder[1] == "out1", "ready cook order second node");
    expect (readySurface.rows.size() == 6, "ready row count");
    expect (rowAt (readySurface, 0).text == "cook patch.cook1-main", "ready cook row");
    expect (rowAt (readySurface, 1).text == "order shader1 -> out1", "ready order row");
    expect (rowAt (readySurface, 2).text == "target out1", "ready target row");
    expect (rowAt (readySurface, 3).text == "graph 2 nodes/1 edges", "ready graph row");
    expect (rowAt (readySurface, 4).text == "readiness ready", "ready readiness row");
    expect (rowAt (readySurface, 5).text == "execution parked", "ready execution row");

    myworld::WorkbenchSessionSnapshot emptyRuntime = ready;
    emptyRuntime.runtimeNodeCount = 0;
    emptyRuntime.runtimeEdgeCount = 0;
    emptyRuntime.runtimeNodes.clear();
    emptyRuntime.runtimeEdges.clear();
    emptyRuntime.activeOutputNodeId.clear();

    const auto emptySurface = myworld::makeWorkbenchCookPlanSurface (emptyRuntime);
    expect (emptySurface.ok, "empty runtime surface ok");
    expect (emptySurface.headline == "cook idle patch.cook1-main order none",
            "empty runtime headline");
    expect (emptySurface.cookOrder.empty(), "empty runtime cook order");
    expect (rowAt (emptySurface, 2).text == "target none", "empty target row");
    expect (rowAt (emptySurface, 4).text == "readiness no-runtime", "empty readiness row");
    expect (rowAt (emptySurface, 5).text == "execution parked", "empty execution row");

    std::cout << "workbench cook plan surface ok\n";
    return 0;
}
