#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct WorkbenchCanvasSurfaceProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct WorkbenchCanvasSurfaceProofRunResult
{
    bool ok = false;
    std::string status;
    std::string statusText;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* workbenchCanvasSurfaceProofDisplayName();
const char* workbenchCanvasSurfaceProofDirectoryName();

WorkbenchCanvasSurfaceProofRunResult runWorkbenchCanvasSurfaceProof (
    const WorkbenchCanvasSurfaceProofRunRequest& request);
}
