#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
enum class C5ModulePublishProofKind
{
    modulePublish,
    aiWorkerModulePublish,
    visibleModulePublish
};

struct C5ModulePublishProofRunRequest
{
    C5ModulePublishProofKind kind = C5ModulePublishProofKind::modulePublish;
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct C5ModulePublishProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* c5ModulePublishProofDisplayName (C5ModulePublishProofKind kind);
const char* c5ModulePublishProofDirectoryName (C5ModulePublishProofKind kind);

C5ModulePublishProofRunResult runC5ModulePublishProof (const C5ModulePublishProofRunRequest& request);
}
