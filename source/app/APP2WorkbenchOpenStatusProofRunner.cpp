#include "APP2WorkbenchOpenStatusProofRunner.h"

#include "ProofRunSupport.h"
#include "WorkbenchSession.h"

namespace myworld
{
namespace
{
constexpr const char* displayName = "APP2 workbench open status";
constexpr const char* directoryName = "app2-workbench-open-status-proof";
constexpr const char* stableDisplayName = "app workbench session";
constexpr const char* stableDirectoryName = "app-workbench-session-proof";
constexpr const char* reportFileName = "workbench_open_status_report.json";
}

const char* app2WorkbenchOpenStatusProofDisplayName()
{
    return displayName;
}

const char* app2WorkbenchOpenStatusProofDirectoryName()
{
    return directoryName;
}

const char* appWorkbenchSessionProofDisplayName()
{
    return stableDisplayName;
}

const char* appWorkbenchSessionProofDirectoryName()
{
    return stableDirectoryName;
}

APP2WorkbenchOpenStatusProofRunResult runAPP2WorkbenchOpenStatusProof (
    const APP2WorkbenchOpenStatusProofRunRequest& request)
{
    APP2WorkbenchOpenStatusProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };

    const auto fail = [&] (const std::string& message)
    {
        result.ok = false;
        result.status = "failed";
        result.error = message;
        return result;
    };

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    if (const auto error = writeProofTextFile (result.reportPath,
                                               makeWorkbenchSessionReportJson (request.snapshot));
        ! error.empty())
    {
        return fail (error);
    }

    result.ok = request.snapshot.ok;
    result.status = request.snapshot.ok ? "dumped" : "failed";
    result.error = request.snapshot.ok ? std::string {} : request.snapshot.message;
    return result;
}
}
