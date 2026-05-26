#pragma once

#include "WorkbenchAppController.h"

#include <string>
#include <vector>

namespace myworld
{
struct WorkbenchStatusSurfaceRow
{
    std::string id;
    std::string label;
    std::string value;
    std::string text;
    std::string tone;
};

struct WorkbenchStatusSurface
{
    bool ok = false;
    std::string headline;
    std::vector<WorkbenchStatusSurfaceRow> rows;
};

WorkbenchStatusSurface makeWorkbenchStatusSurface (const WorkbenchAppStatusSnapshot& snapshot);
}
