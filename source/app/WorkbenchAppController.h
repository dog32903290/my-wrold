#pragma once

#include "APP2WorkbenchOpenStatusProofRunner.h"
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
    const WorkbenchSessionSnapshot& currentSession() const;
    std::string statusText() const;

    APP2WorkbenchOpenStatusProofRunRequest makeOpenStatusProofRequest (
        const std::filesystem::path& outputDirectory) const;

private:
    WorkbenchSessionSnapshot currentSessionSnapshot;
    std::string currentStatusText = "workbench blocked: no session";
};
}
