#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct PVB1AnalyzerEnvironmentProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct PVB1AnalyzerEnvironmentProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* pvB1AnalyzerEnvironmentProofDisplayName();
const char* pvB1AnalyzerEnvironmentProofDirectoryName();

PVB1AnalyzerEnvironmentProofRunResult runPVB1AnalyzerEnvironmentProof (
    const PVB1AnalyzerEnvironmentProofRunRequest& request);
}
