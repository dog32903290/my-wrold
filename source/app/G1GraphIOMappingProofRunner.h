#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct G1GraphIOMappingProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
    double sourceValue = 0.5;
};

struct G1GraphIOMappingProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* g1GraphIOMappingProofDisplayName();
const char* g1GraphIOMappingProofDirectoryName();

G1GraphIOMappingProofRunResult runG1GraphIOMappingProof (const G1GraphIOMappingProofRunRequest& request);
}
