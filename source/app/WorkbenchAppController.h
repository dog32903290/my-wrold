#pragma once

#include "AppWorkbenchSessionProofRunner.h"
#include "InteractionContract.h"
#include "WorkbenchSessionOpenStatus.h"

#include <filesystem>
#include <string>

namespace myworld
{
using WorkbenchAppControllerOpenRequest = WorkbenchSessionOpenStatusRequest;
using WorkbenchAppControllerOpenResult = WorkbenchSessionOpenStatusResult;

class WorkbenchAppController
{
public:
    WorkbenchAppControllerOpenResult openCurrentSession (const WorkbenchAppControllerOpenRequest& request);
    CommandResult saveCurrentSession (GraphSession& session);
    const WorkbenchSessionSnapshot& currentSession() const;
    std::string statusText() const;

    AppWorkbenchSessionProofRunRequest makeOpenStatusProofRequest (
        const std::filesystem::path& outputDirectory) const;

private:
    WorkbenchSessionSnapshot currentSessionSnapshot;
    std::string currentStatusText = "workbench blocked: no session";
};
}
