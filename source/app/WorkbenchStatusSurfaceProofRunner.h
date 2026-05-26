#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct WorkbenchStatusSurfaceProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct WorkbenchStatusSurfaceProofRunResult
{
    bool ok = false;
    std::string status;
    std::string statusText;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* workbenchStatusSurfaceProofDisplayName();
const char* workbenchStatusSurfaceProofDirectoryName();

WorkbenchStatusSurfaceProofRunResult runWorkbenchStatusSurfaceProof (
    const WorkbenchStatusSurfaceProofRunRequest& request);
}
