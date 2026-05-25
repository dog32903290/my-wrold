#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct ProjectCreationProofRunRequest
{
    std::filesystem::path outputDirectory;
};

struct ProjectCreationProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* projectCreationProofDisplayName();
const char* projectCreationProofDirectoryName();

ProjectCreationProofRunResult runProjectCreationProof (const ProjectCreationProofRunRequest& request);
}
