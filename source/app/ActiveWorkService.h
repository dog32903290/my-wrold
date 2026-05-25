#pragma once

#include "InteractionContract.h"
#include "StorageCommand.h"

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

ActiveWorkPreparationResult prepareActiveWorkProjectForOpen();
CommandResult saveActiveWorkProject (GraphSession& session);
PublishModuleResult publishSelectedModuleFromActiveWork (GraphSession& session, const std::string& sourceNodeId);
}
