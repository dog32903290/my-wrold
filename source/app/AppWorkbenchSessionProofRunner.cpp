#include "AppWorkbenchSessionProofRunner.h"

#include "ProofRunSupport.h"
#include "WorkbenchSession.h"

namespace myworld
{
namespace
{
constexpr const char* displayName = "app workbench session";
constexpr const char* directoryName = "app-workbench-session-proof";
constexpr const char* reportFileName = "workbench_open_status_report.json";
}

const char* appWorkbenchSessionProofDisplayName()
{
    return displayName;
}

const char* appWorkbenchSessionProofDirectoryName()
{
    return directoryName;
}

AppWorkbenchSessionProofRunResult runAppWorkbenchSessionProof (
    const AppWorkbenchSessionProofRunRequest& request)
{
    AppWorkbenchSessionProofRunResult result;
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
