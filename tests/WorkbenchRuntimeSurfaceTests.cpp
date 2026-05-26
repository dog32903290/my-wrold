#include "WorkbenchRuntimeSurface.h"

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

const myworld::WorkbenchRuntimeSurfaceRow& rowAt (const myworld::WorkbenchRuntimeSurface& surface,
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

    const auto blockedSurface = myworld::makeWorkbenchRuntimeSurface (blocked);
    expect (! blockedSurface.ok, "blocked surface ok");
    expect (blockedSurface.headline == "runtime blocked: no current workbench session",
            "blocked headline");
    expect (blockedSurface.rows.size() == 6, "blocked row count");
    expect (rowAt (blockedSurface, 0).text == "runtime no workbench session",
            "blocked runtime row");
    expect (rowAt (blockedSurface, 1).text == "graph 0 nodes/0 edges", "blocked graph row");
    expect (rowAt (blockedSurface, 2).text == "output none", "blocked output row");
    expect (rowAt (blockedSurface, 3).text == "mapping unknown 0/0", "blocked mapping row");
    expect (rowAt (blockedSurface, 4).text == "readiness blocked", "blocked readiness row");
    expect (rowAt (blockedSurface, 5).text == "cook parked", "blocked cook row");

    myworld::WorkbenchSessionSnapshot ready;
    ready.ok = true;
    ready.status = "ready";
    ready.documentId = "patch.runtime1-main";
    ready.runtimeNodeCount = 2;
    ready.runtimeEdgeCount = 1;
    ready.activeOutputNodeId = "out1";
    ready.graphIOMappingStatus = "valid";
    ready.graphIOMappingCount = 1;
    ready.validGraphIOMappingCount = 1;

    const auto readySurface = myworld::makeWorkbenchRuntimeSurface (ready);
    expect (readySurface.ok, "ready surface ok");
    expect (readySurface.headline == "runtime ready patch.runtime1-main nodes 2 edges 1",
            "ready headline");
    expect (readySurface.rows.size() == 6, "ready row count");
    expect (rowAt (readySurface, 0).text == "runtime patch.runtime1-main", "ready runtime row");
    expect (rowAt (readySurface, 1).text == "graph 2 nodes/1 edges", "ready graph row");
    expect (rowAt (readySurface, 2).text == "output out1", "ready output row");
    expect (rowAt (readySurface, 3).text == "mapping valid 1/1", "ready mapping row");
    expect (rowAt (readySurface, 4).text == "readiness ready", "ready readiness row");
    expect (rowAt (readySurface, 5).text == "cook parked", "ready cook row");

    myworld::WorkbenchSessionSnapshot emptyRuntime = ready;
    emptyRuntime.runtimeNodeCount = 0;
    emptyRuntime.runtimeEdgeCount = 0;
    emptyRuntime.activeOutputNodeId.clear();

    const auto emptySurface = myworld::makeWorkbenchRuntimeSurface (emptyRuntime);
    expect (emptySurface.ok, "empty runtime surface ok");
    expect (emptySurface.headline == "runtime idle patch.runtime1-main nodes 0 edges 0",
            "empty runtime headline");
    expect (rowAt (emptySurface, 2).text == "output none", "empty output row");
    expect (rowAt (emptySurface, 4).text == "readiness no-runtime", "empty readiness row");
    expect (rowAt (emptySurface, 5).text == "cook parked", "empty cook row");

    std::cout << "workbench runtime surface ok\n";
    return 0;
}
