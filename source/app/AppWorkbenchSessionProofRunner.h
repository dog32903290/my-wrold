#pragma once

#include "ActiveWorkService.h"
#include "WorkbenchSession.h"

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct AppWorkbenchSessionProofRunRequest
{
    std::filesystem::path outputDirectory;
    WorkbenchSessionSnapshot snapshot;
    ActiveWorkPreparationResult activeWorkPreparation;
};

struct AppWorkbenchSessionProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* appWorkbenchSessionProofDisplayName();
const char* appWorkbenchSessionProofDirectoryName();

AppWorkbenchSessionProofRunResult runAppWorkbenchSessionProof (
    const AppWorkbenchSessionProofRunRequest& request);
}
