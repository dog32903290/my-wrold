#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct WorkbenchRuntimeSurfaceProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct WorkbenchRuntimeSurfaceProofRunResult
{
    bool ok = false;
    std::string status;
    std::string statusText;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* workbenchRuntimeSurfaceProofDisplayName();
const char* workbenchRuntimeSurfaceProofDirectoryName();

WorkbenchRuntimeSurfaceProofRunResult runWorkbenchRuntimeSurfaceProof (
    const WorkbenchRuntimeSurfaceProofRunRequest& request);
}
