#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct CreatedProjectOpenProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct CreatedProjectOpenProofRunResult
{
    bool ok = false;
    std::string status;
    std::string statusText;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* createdProjectOpenProofDisplayName();
const char* createdProjectOpenProofDirectoryName();

CreatedProjectOpenProofRunResult runCreatedProjectOpenProof (
    const CreatedProjectOpenProofRunRequest& request);
}
