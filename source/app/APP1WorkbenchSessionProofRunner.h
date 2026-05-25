#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct APP1WorkbenchSessionProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
    bool dirty = true;
    std::string saveStatus = "dirty";
    std::string proofStatus = "g1-report-dumped";
    std::string previewStatus = "preview-ready";
};

struct APP1WorkbenchSessionProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* app1WorkbenchSessionProofDisplayName();
const char* app1WorkbenchSessionProofDirectoryName();

APP1WorkbenchSessionProofRunResult runAPP1WorkbenchSessionProof (
    const APP1WorkbenchSessionProofRunRequest& request);
}
