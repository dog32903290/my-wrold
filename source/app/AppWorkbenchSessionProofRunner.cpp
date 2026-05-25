#include "AppWorkbenchSessionProofRunner.h"

#include "JsonWriter.h"
#include "ProofRunSupport.h"
#include "WorkbenchSession.h"

#include <sstream>

namespace myworld
{
namespace
{
constexpr const char* displayName = "app workbench session";
constexpr const char* directoryName = "app-workbench-session-proof";
constexpr const char* reportFileName = "workbench_open_status_report.json";
constexpr const char* preparationReportFileName = "active_work_preparation_report.json";

std::string makeActiveWorkPreparationReportJson (const ActiveWorkPreparationResult& preparation)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"activeWorkPreparationReport\",\n";
    out << "  \"ok\": " << (preparation.ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (preparation.status) << ",\n";
    out << "  \"workManifestPath\": " << jsonQuoted (preparation.workManifestPath) << ",\n";
    out << "  \"error\": " << jsonQuoted (preparation.error) << ",\n";
    out << "  \"diagnostics\": ";
    appendJsonStringArray (out, preparation.diagnostics);
    out << "\n";
    out << "}\n";
    return out.str();
}
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
    const auto preparationReportPath = request.outputDirectory / preparationReportFileName;
    result.artifactPaths = { result.reportPath, preparationReportPath };

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

    if (const auto error = writeProofTextFile (preparationReportPath,
                                               makeActiveWorkPreparationReportJson (request.activeWorkPreparation));
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
