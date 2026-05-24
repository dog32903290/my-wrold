#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct C4AIWorkerSaveWorkProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct C4AIWorkerSaveWorkProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* c4AIWorkerSaveWorkProofDisplayName();
const char* c4AIWorkerSaveWorkProofDirectoryName();

C4AIWorkerSaveWorkProofRunResult runC4AIWorkerSaveWorkProof (
    const C4AIWorkerSaveWorkProofRunRequest& request);
}
