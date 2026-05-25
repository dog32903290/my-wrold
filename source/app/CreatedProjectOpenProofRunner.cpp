#include "CreatedProjectOpenProofRunner.h"

#include "ActiveWorkService.h"
#include "JsonWriter.h"
#include "ProofRunSupport.h"
#include "WorkbenchSessionOpenStatus.h"

#include <sstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* displayName = "created project open";
constexpr const char* directoryName = "created-project-open-proof";
constexpr const char* reportFileName = "created_project_open_report.json";

CreateActiveWorkProjectRequest makeOpenProofProjectRequest (const std::filesystem::path& outputDirectory)
{
    CreateActiveWorkProjectRequest request;
    request.projectDirectory = outputDirectory / "created-project";
    request.workId = "work.project-open1";
    request.workTitle = "OPEN1 Proof Work";
    request.patchId = "patch.open1-main";
    request.patchTitle = "OPEN1 Main Patch";
    return request;
}

std::string makeCreatedProjectOpenStatusText (const CreateActiveWorkProjectResult& created,
                                              const WorkbenchSessionOpenStatusResult& opened)
{
    if (! created.ok)
        return "created project open failed: " + created.error;

    if (! opened.ok)
        return "created project open failed: " + opened.error;

    return std::string ("created project open ready: ")
           + "work.project-open1"
           + " -> "
           + opened.snapshot.documentId
           + " source "
           + opened.snapshot.workSourceStatus
           + " mappings "
           + std::to_string (opened.snapshot.validGraphIOMappingCount)
           + "/"
           + std::to_string (opened.snapshot.graphIOMappingCount);
}

std::string makeCreatedProjectOpenReportJson (const CreateActiveWorkProjectResult& created,
                                              const WorkbenchSessionOpenStatusResult& opened,
                                              const std::string& statusText)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"createdProjectOpenReport\",\n";
    out << "  \"ok\": " << (created.ok && opened.ok ? "true" : "false") << ",\n";
    out << "  \"creationStatus\": " << jsonQuoted (created.status) << ",\n";
    out << "  \"openStatus\": " << jsonQuoted (opened.status) << ",\n";
    out << "  \"createdManifestPath\": " << jsonQuoted (created.workManifestPath) << ",\n";
    out << "  \"openedManifestPath\": " << jsonQuoted (opened.snapshot.workManifestPath) << ",\n";
    out << "  \"activeWorkManifestPath\": " << jsonQuoted (opened.snapshot.activeWorkManifestPath) << ",\n";
    out << "  \"workSource\": " << jsonQuoted (opened.snapshot.workSource) << ",\n";
    out << "  \"workSourceStatus\": " << jsonQuoted (opened.snapshot.workSourceStatus) << ",\n";
    out << "  \"documentId\": " << jsonQuoted (opened.snapshot.documentId) << ",\n";
    out << "  \"graphIOMappingStatus\": " << jsonQuoted (opened.snapshot.graphIOMappingStatus) << ",\n";
    out << "  \"graphIOMappingCount\": " << opened.snapshot.graphIOMappingCount << ",\n";
    out << "  \"validGraphIOMappingCount\": " << opened.snapshot.validGraphIOMappingCount << ",\n";
    out << "  \"statusText\": " << jsonQuoted (statusText) << ",\n";
    out << "  \"error\": " << jsonQuoted (created.ok ? opened.error : created.error) << ",\n";
    out << "  \"creationDiagnostics\": ";
    appendJsonStringArray (out, created.diagnostics);
    out << ",\n";
    out << "  \"workDiagnostics\": ";
    appendJsonStringArray (out, opened.snapshot.workDiagnostics);
    out << "\n";
    out << "}\n";
    return out.str();
}
}

const char* createdProjectOpenProofDisplayName()
{
    return displayName;
}

const char* createdProjectOpenProofDirectoryName()
{
    return directoryName;
}

CreatedProjectOpenProofRunResult runCreatedProjectOpenProof (
    const CreatedProjectOpenProofRunRequest& request)
{
    CreatedProjectOpenProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };

    const auto fail = [&] (const std::string& message)
    {
        result.ok = false;
        result.status = "failed";
        result.statusText = "created project open failed: " + message;
        result.error = message;
        return result;
    };

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    const auto createRequest = makeOpenProofProjectRequest (request.outputDirectory);
    std::error_code removeError;
    std::filesystem::remove_all (createRequest.projectDirectory, removeError);
    if (removeError)
        return fail ("could not reset created project open proof directory: " + removeError.message());

    const auto created = createActiveWorkProject (createRequest);

    WorkbenchSessionOpenStatusRequest openRequest;
    openRequest.activeWorkManifestPath = created.workManifestPath;
    openRequest.candidateRoots = request.candidateRoots;
    openRequest.saveStatus = "clean";
    openRequest.proofStatus = "open1-ready";
    openRequest.previewStatus = "preview-ready";
    const auto opened = openCurrentWorkbenchSession (openRequest);

    result.statusText = makeCreatedProjectOpenStatusText (created, opened);

    if (const auto error = writeProofTextFile (result.reportPath,
                                               makeCreatedProjectOpenReportJson (created, opened, result.statusText));
        ! error.empty())
    {
        return fail (error);
    }

    result.ok = created.ok && opened.ok;
    result.status = result.ok ? "dumped" : "failed";
    result.error = result.ok ? std::string {} : (created.ok ? opened.error : created.error);
    return result;
}
}
