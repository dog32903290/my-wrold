#pragma once

#include "WorkbenchSession.h"

#include <string>
#include <vector>

namespace myworld
{
struct WorkbenchRuntimeSurfaceRow
{
    std::string id;
    std::string label;
    std::string value;
    std::string text;
    std::string tone;
};

struct WorkbenchRuntimeSurface
{
    bool ok = false;
    std::string headline;
    std::vector<WorkbenchRuntimeSurfaceRow> rows;
};

WorkbenchRuntimeSurface makeWorkbenchRuntimeSurface (const WorkbenchSessionSnapshot& snapshot);
}
