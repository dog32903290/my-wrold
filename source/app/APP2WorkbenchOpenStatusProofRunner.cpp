#include "APP2WorkbenchOpenStatusProofRunner.h"

namespace myworld
{
namespace
{
constexpr const char* displayName = "APP2 workbench open status";
constexpr const char* directoryName = "app2-workbench-open-status-proof";
}

const char* app2WorkbenchOpenStatusProofDisplayName()
{
    return displayName;
}

const char* app2WorkbenchOpenStatusProofDirectoryName()
{
    return directoryName;
}

APP2WorkbenchOpenStatusProofRunResult runAPP2WorkbenchOpenStatusProof (
    const APP2WorkbenchOpenStatusProofRunRequest& request)
{
    AppWorkbenchSessionProofRunRequest stableRequest;
    stableRequest.outputDirectory = request.outputDirectory;
    stableRequest.snapshot = request.snapshot;

    const auto stableResult = runAppWorkbenchSessionProof (stableRequest);

    APP2WorkbenchOpenStatusProofRunResult result;
    result.ok = stableResult.ok;
    result.status = stableResult.status;
    result.error = stableResult.error;
    result.outputDirectory = stableResult.outputDirectory;
    result.reportPath = stableResult.reportPath;
    result.artifactPaths = stableResult.artifactPaths;
    return result;
}
}
