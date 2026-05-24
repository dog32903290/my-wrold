#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct C6AIRepairLoopProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct C6AIRepairLoopProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* c6AIRepairLoopProofDisplayName();
const char* c6AIRepairLoopProofDirectoryName();

C6AIRepairLoopProofRunResult runC6AIRepairLoopProof (const C6AIRepairLoopProofRunRequest& request);
}
