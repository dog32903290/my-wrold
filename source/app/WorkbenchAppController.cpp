#include "WorkbenchAppController.h"

#include "StorageCommand.h"

namespace myworld
{
WorkbenchAppControllerOpenResult WorkbenchAppController::openCurrentSession (
    const WorkbenchAppControllerOpenRequest& request)
{
    const auto opened = openCurrentWorkbenchSession (request);
    currentSessionSnapshot = opened.snapshot;
    currentStatusText = makeWorkbenchSessionStatusText (currentSessionSnapshot);
    return opened;
}

CommandResult WorkbenchAppController::saveCurrentSession (GraphSession& session)
{
    if (! currentSessionSnapshot.ok || currentSessionSnapshot.workManifestPath.empty())
    {
        currentStatusText = "save_work failed: no current workbench session";
        return { false, "no current workbench session" };
    }

    const auto saved = saveWork (session, currentSessionSnapshot.workManifestPath);
    const auto message = saved.ok ? saved.status : saved.error;
    currentStatusText = (saved.ok ? "save_work: " : "save_work failed: ") + message;
    currentSessionSnapshot.dirty = session.dirty;
    currentSessionSnapshot.saveStatus = saved.status.empty() ? "failed" : saved.status;
    return { saved.ok, message };
}

const WorkbenchSessionSnapshot& WorkbenchAppController::currentSession() const
{
    return currentSessionSnapshot;
}

WorkbenchAppStatusSnapshot WorkbenchAppController::appStatusSnapshot() const
{
    WorkbenchAppStatusSnapshot status;
    status.ok = currentSessionSnapshot.ok;
    status.status = currentSessionSnapshot.status.empty()
                        ? (currentSessionSnapshot.ok ? "ready" : "blocked")
                        : currentSessionSnapshot.status;
    status.message = currentSessionSnapshot.message.empty()
                         ? (currentSessionSnapshot.ok ? "workbench_session_ready" : "no current workbench session")
                         : currentSessionSnapshot.message;
    status.statusText = currentStatusText;
    status.workManifestPath = currentSessionSnapshot.workManifestPath;
    status.workSource = currentSessionSnapshot.workSource;
    status.workSourceStatus = currentSessionSnapshot.workSourceStatus;
    status.activeWorkManifestPath = currentSessionSnapshot.activeWorkManifestPath;
    status.graphIOMappingSourcePath = currentSessionSnapshot.graphIOMappingSourcePath;
    status.documentId = currentSessionSnapshot.documentId;
    status.documentTitle = currentSessionSnapshot.documentTitle;
    status.documentVersion = currentSessionSnapshot.documentVersion;
    status.dirty = currentSessionSnapshot.dirty;
    status.saveStatus = currentSessionSnapshot.saveStatus;
    status.proofStatus = currentSessionSnapshot.proofStatus;
    status.previewStatus = currentSessionSnapshot.previewStatus;
    status.graphIOMappingCount = currentSessionSnapshot.graphIOMappingCount;
    status.validGraphIOMappingCount = currentSessionSnapshot.validGraphIOMappingCount;
    status.graphIOMappingStatus = currentSessionSnapshot.graphIOMappingStatus;
    status.workDiagnostics = currentSessionSnapshot.workDiagnostics;
    status.diagnostics = currentSessionSnapshot.diagnostics;
    return status;
}

std::string WorkbenchAppController::statusText() const
{
    return currentStatusText;
}

AppWorkbenchSessionProofRunRequest WorkbenchAppController::makeOpenStatusProofRequest (
    const std::filesystem::path& outputDirectory) const
{
    AppWorkbenchSessionProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.snapshot = currentSessionSnapshot;
    return request;
}
}
