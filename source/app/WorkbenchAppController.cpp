#include "WorkbenchAppController.h"

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

const WorkbenchSessionSnapshot& WorkbenchAppController::currentSession() const
{
    return currentSessionSnapshot;
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
