#pragma once

#include "WorkbenchSession.h"

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct WorkbenchSessionOpenStatusRequest
{
    std::filesystem::path activeWorkManifestPath;
    std::vector<std::filesystem::path> candidateRoots;
    bool dirty = false;
    std::string saveStatus = "clean";
    std::string proofStatus = "not-run";
    std::string previewStatus = "preview-ready";
};

struct WorkbenchSessionOpenStatusResult
{
    bool ok = false;
    std::string status;
    std::string error;
    WorkbenchSessionSnapshot snapshot;
};

WorkbenchSessionOpenStatusResult openCurrentWorkbenchSession (
    const WorkbenchSessionOpenStatusRequest& request);

std::string makeWorkbenchSessionStatusText (const WorkbenchSessionSnapshot& snapshot);
}
