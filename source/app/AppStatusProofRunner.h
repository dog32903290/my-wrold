#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct AppStatusProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct AppStatusProofRunResult
{
    bool ok = false;
    std::string status;
    std::string statusText;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* appStatusProofDisplayName();
const char* appStatusProofDirectoryName();

AppStatusProofRunResult runAppStatusProof (const AppStatusProofRunRequest& request);
}
