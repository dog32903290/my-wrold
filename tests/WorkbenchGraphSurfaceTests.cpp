#include "WorkbenchGraphSurface.h"

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

const myworld::WorkbenchGraphSurfaceRow& rowAt (const myworld::WorkbenchGraphSurface& surface,
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

    const auto blockedSurface = myworld::makeWorkbenchGraphSurface (blocked);
    expect (! blockedSurface.ok, "blocked surface ok");
    expect (blockedSurface.headline == "graph blocked: no current workbench session", "blocked headline");
    expect (blockedSurface.rows.size() == 6, "blocked row count");
    expect (rowAt (blockedSurface, 0).text == "graph no workbench session", "blocked graph row");
    expect (rowAt (blockedSurface, 1).text == "editor 0 nodes/0 edges", "blocked editor row");
    expect (rowAt (blockedSurface, 4).text == "node none", "blocked node row");
    expect (rowAt (blockedSurface, 5).text == "edge none", "blocked edge row");

    myworld::WorkbenchSessionSnapshot ready;
    ready.ok = true;
    ready.status = "ready";
    ready.documentId = "patch.graph1-main";
    ready.editorNodeCount = 2;
    ready.editorEdgeCount = 1;
    ready.runtimeNodeCount = 2;
    ready.runtimeEdgeCount = 1;
    ready.activeOutputNodeId = "out1";
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

    const auto readySurface = myworld::makeWorkbenchGraphSurface (ready);
    expect (readySurface.ok, "ready surface ok");
    expect (readySurface.headline == "graph ready patch.graph1-main editor 2/1 runtime 2/1",
            "ready headline");
    expect (readySurface.rows.size() == 6, "ready row count");
    expect (rowAt (readySurface, 0).text == "graph patch.graph1-main", "ready graph row");
    expect (rowAt (readySurface, 1).text == "editor 2 nodes/1 edges", "ready editor row");
    expect (rowAt (readySurface, 2).text == "runtime 2 nodes/1 edges", "ready runtime row");
    expect (rowAt (readySurface, 3).text == "output out1", "ready output row");
    expect (rowAt (readySurface, 4).text == "node shader1 shader.fragment", "ready node row");
    expect (rowAt (readySurface, 5).text == "edge shader1.output -> out1.input", "ready edge row");

    std::cout << "workbench graph surface ok\n";
    return 0;
}
