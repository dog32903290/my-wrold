#pragma once

#include "StorageContract.h"
#include "WorkProjectLifecycle.h"

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct WorkProjectResolveRequest
{
    std::filesystem::path activeWorkManifestPath;
    std::vector<std::filesystem::path> candidateRoots;
    std::filesystem::path fallbackWorkManifestPath = "fixtures/storage/c2-compound-work/myworld.work.json";
};

struct WorkProjectResolveResult
{
    bool ok = false;
    std::string error;
    PatchDocument document;
    std::string workManifestPath;
    std::string activeWorkManifestPath;
    WorkProjectLifecycle lifecycle;
    std::vector<std::string> workDiagnostics;
};

WorkProjectResolveResult resolveWorkProjectForWorkbench (const WorkProjectResolveRequest& request);
}
