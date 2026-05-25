#pragma once

#include "InteractionContract.h"
#include "StorageCommand.h"

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct ActiveWorkPreparationResult
{
    bool ok = false;
    std::string status;
    std::string workManifestPath;
    std::string error;
    std::vector<std::string> diagnostics;
};

struct CreateActiveWorkProjectRequest
{
    std::filesystem::path projectDirectory;
    std::string workId;
    std::string workTitle;
    std::string patchId;
    std::string patchTitle;
    bool overwriteExisting = false;
};

struct CreateActiveWorkProjectResult
{
    bool ok = false;
    std::string status;
    std::string workManifestPath;
    std::string patchPath;
    std::string error;
    std::vector<std::string> diagnostics;
};

ActiveWorkPreparationResult prepareActiveWorkProjectForOpen();
CreateActiveWorkProjectResult createActiveWorkProject (const CreateActiveWorkProjectRequest& request);
CommandResult saveActiveWorkProject (GraphSession& session);
PublishModuleResult publishSelectedModuleFromActiveWork (GraphSession& session, const std::string& sourceNodeId);
}
