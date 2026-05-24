#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct C2StorageProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct C2StorageProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* c2StorageProofDisplayName();
const char* c2StorageProofDirectoryName();

C2StorageProofRunResult runC2StorageProof (const C2StorageProofRunRequest& request);
}
