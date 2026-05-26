#pragma once

#include "WorkbenchSession.h"

#include <string>
#include <vector>

namespace myworld
{
struct WorkbenchCookPlanSurfaceRow
{
    std::string id;
    std::string label;
    std::string value;
    std::string text;
    std::string tone;
};

struct WorkbenchCookPlanSurface
{
    bool ok = false;
    std::string headline;
    std::vector<std::string> cookOrder;
    std::vector<WorkbenchCookPlanSurfaceRow> rows;
};

WorkbenchCookPlanSurface makeWorkbenchCookPlanSurface (const WorkbenchSessionSnapshot& snapshot);
}
