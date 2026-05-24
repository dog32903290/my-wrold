#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct C6AnalyzerFamilyProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct C6AnalyzerFamilyProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* c6AnalyzerFamilyProofDisplayName();
const char* c6AnalyzerFamilyProofDirectoryName();

C6AnalyzerFamilyProofRunResult runC6AnalyzerFamilyProof (const C6AnalyzerFamilyProofRunRequest& request);
}
