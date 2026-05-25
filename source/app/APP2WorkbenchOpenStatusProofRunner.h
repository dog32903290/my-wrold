#pragma once

#include "WorkbenchSession.h"

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct APP2WorkbenchOpenStatusProofRunRequest
{
    std::filesystem::path outputDirectory;
    WorkbenchSessionSnapshot snapshot;
};

struct APP2WorkbenchOpenStatusProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* app2WorkbenchOpenStatusProofDisplayName();
const char* app2WorkbenchOpenStatusProofDirectoryName();

APP2WorkbenchOpenStatusProofRunResult runAPP2WorkbenchOpenStatusProof (
    const APP2WorkbenchOpenStatusProofRunRequest& request);
}
