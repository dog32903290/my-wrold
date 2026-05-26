#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct WorkbenchGraphSurfaceProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct WorkbenchGraphSurfaceProofRunResult
{
    bool ok = false;
    std::string status;
    std::string statusText;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* workbenchGraphSurfaceProofDisplayName();
const char* workbenchGraphSurfaceProofDirectoryName();

WorkbenchGraphSurfaceProofRunResult runWorkbenchGraphSurfaceProof (
    const WorkbenchGraphSurfaceProofRunRequest& request);
}
