#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct WorkbenchCookPlanSurfaceProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct WorkbenchCookPlanSurfaceProofRunResult
{
    bool ok = false;
    std::string status;
    std::string statusText;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* workbenchCookPlanProofDisplayName();
const char* workbenchCookPlanProofDirectoryName();

WorkbenchCookPlanSurfaceProofRunResult runWorkbenchCookPlanSurfaceProof (
    const WorkbenchCookPlanSurfaceProofRunRequest& request);
}
