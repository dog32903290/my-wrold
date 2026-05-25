#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct LiveIOProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
    float loudness = 0.5f;
};

struct LiveIOProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* liveIOProofDisplayName();
const char* liveIOProofDirectoryName();

LiveIOProofRunResult runLiveIOProof (const LiveIOProofRunRequest& request);
}
