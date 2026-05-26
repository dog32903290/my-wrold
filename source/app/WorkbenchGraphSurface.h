#pragma once

#include "WorkbenchSession.h"

#include <string>
#include <vector>

namespace myworld
{
struct WorkbenchGraphSurfaceRow
{
    std::string id;
    std::string label;
    std::string value;
    std::string text;
    std::string tone;
};

struct WorkbenchGraphSurface
{
    bool ok = false;
    std::string headline;
    std::vector<WorkbenchGraphSurfaceRow> rows;
};

WorkbenchGraphSurface makeWorkbenchGraphSurface (const WorkbenchSessionSnapshot& snapshot);
}
