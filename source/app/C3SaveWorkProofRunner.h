#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct C3SaveWorkProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct C3SaveWorkProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* c3SaveWorkProofDisplayName();
const char* c3SaveWorkProofDirectoryName();

C3SaveWorkProofRunResult runC3SaveWorkProof (const C3SaveWorkProofRunRequest& request);
}
