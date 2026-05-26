#include "WorkbenchStatusSurface.h"

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

const myworld::WorkbenchStatusSurfaceRow& rowAt (const myworld::WorkbenchStatusSurface& surface,
                                                 std::size_t index)
{
    expect (surface.rows.size() > index, "surface row index exists");
    return surface.rows[index];
}
}

int main()
{
    myworld::WorkbenchAppStatusSnapshot blocked;
    blocked.status = "blocked";
    blocked.message = "no current workbench session";
    blocked.statusText = "workbench blocked: no session";

    const auto blockedSurface = myworld::makeWorkbenchStatusSurface (blocked);
    expect (! blockedSurface.ok, "blocked surface ok");
    expect (blockedSurface.headline == "workbench blocked: no session", "blocked headline");
    expect (blockedSurface.rows.size() == 6, "blocked row count");
    expect (rowAt (blockedSurface, 0).id == "work", "blocked work row id");
    expect (rowAt (blockedSurface, 0).text == "work no workbench session", "blocked work row text");
    expect (rowAt (blockedSurface, 1).text == "source unknown", "blocked source row text");
    expect (rowAt (blockedSurface, 2).text == "save unknown", "blocked save row text");

    myworld::WorkbenchAppStatusSnapshot ready;
    ready.ok = true;
    ready.status = "ready";
    ready.statusText = "workbench ready patch.ui-main source active-work-opened mappings 1/1";
    ready.documentId = "patch.ui-main";
    ready.workSourceStatus = "active-work-opened";
    ready.saveStatus = "save-ok commit-pending";
    ready.graphIOMappingStatus = "valid";
    ready.validGraphIOMappingCount = 1;
    ready.graphIOMappingCount = 1;
    ready.proofStatus = "ui1-ready";
    ready.previewStatus = "preview-ready";

    const auto readySurface = myworld::makeWorkbenchStatusSurface (ready);
    expect (readySurface.ok, "ready surface ok");
    expect (readySurface.headline == ready.statusText, "ready headline");
    expect (readySurface.rows.size() == 6, "ready row count");
    expect (rowAt (readySurface, 0).text == "work patch.ui-main", "ready work row");
    expect (rowAt (readySurface, 1).text == "source active-work-opened", "ready source row");
    expect (rowAt (readySurface, 2).text == "save save-ok commit-pending", "ready save row");
    expect (rowAt (readySurface, 3).text == "mapping valid 1/1", "ready mapping row");
    expect (rowAt (readySurface, 4).text == "proof ui1-ready", "ready proof row");
    expect (rowAt (readySurface, 5).text == "preview preview-ready", "ready preview row");

    std::cout << "workbench status surface ok\n";
    return 0;
}
