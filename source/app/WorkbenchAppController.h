#pragma once

#include "AppWorkbenchSessionProofRunner.h"
#include "InteractionContract.h"
#include "WorkbenchSessionOpenStatus.h"

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
using WorkbenchAppControllerOpenRequest = WorkbenchSessionOpenStatusRequest;
using WorkbenchAppControllerOpenResult = WorkbenchSessionOpenStatusResult;

struct WorkbenchAppStatusSnapshot
{
    bool ok = false;
    std::string status = "blocked";
    std::string message;
    std::string statusText;
    std::string workManifestPath;
    std::string workSource;
    std::string workSourceStatus;
    std::string activeWorkManifestPath;
    std::string graphIOMappingSourcePath;
    std::string documentId;
    std::string documentTitle;
    int documentVersion = 0;
    bool dirty = false;
    std::string saveStatus;
    std::string proofStatus;
    std::string previewStatus;
    int graphIOMappingCount = 0;
    int validGraphIOMappingCount = 0;
    std::string graphIOMappingStatus;
    std::vector<std::string> workDiagnostics;
    std::vector<std::string> diagnostics;
};

class WorkbenchAppController
{
public:
    WorkbenchAppControllerOpenResult openCurrentSession (const WorkbenchAppControllerOpenRequest& request);
    CommandResult saveCurrentSession (GraphSession& session);
    const WorkbenchSessionSnapshot& currentSession() const;
    WorkbenchAppStatusSnapshot appStatusSnapshot() const;
    std::string statusText() const;

    AppWorkbenchSessionProofRunRequest makeOpenStatusProofRequest (
        const std::filesystem::path& outputDirectory) const;

private:
    WorkbenchSessionSnapshot currentSessionSnapshot;
    std::string currentStatusText = "workbench blocked: no session";
};
}
