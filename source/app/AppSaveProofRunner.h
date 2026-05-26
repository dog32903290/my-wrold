#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct AppSaveProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct AppSaveProofRunResult
{
    bool ok = false;
    std::string status;
    std::string statusText;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* appSaveProofDisplayName();
const char* appSaveProofDirectoryName();

AppSaveProofRunResult runAppSaveProof (const AppSaveProofRunRequest& request);
}
