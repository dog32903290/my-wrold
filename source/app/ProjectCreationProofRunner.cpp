#include "ProjectCreationProofRunner.h"

#include "ActiveWorkService.h"
#include "JsonWriter.h"
#include "ProofRunSupport.h"
#include "StorageContract.h"

#include <sstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* displayName = "project creation";
constexpr const char* directoryName = "project-creation-proof";
constexpr const char* reportFileName = "project_creation_report.json";

CreateActiveWorkProjectRequest makeProjectCreationRequest (const std::filesystem::path& outputDirectory)
{
    CreateActiveWorkProjectRequest request;
    request.projectDirectory = outputDirectory / "created-project";
    request.workId = "work.project2-proof";
    request.workTitle = "PROJECT2 Proof Work";
    request.patchId = "patch.project2-main";
    request.patchTitle = "PROJECT2 Main Patch";
    return request;
}

std::string makeProjectCreationReportJson (const CreateActiveWorkProjectResult& created,
                                           const CreateActiveWorkProjectResult& duplicate,
                                           const WorkProjectLoadResult& loadedWork,
                                           const PatchDocumentLoadResult& loadedPatch)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"projectCreationReport\",\n";
    out << "  \"ok\": " << (created.ok && loadedWork.ok && loadedPatch.ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (created.status) << ",\n";
    out << "  \"manifestPath\": " << jsonQuoted (created.workManifestPath) << ",\n";
    out << "  \"patchPath\": " << jsonQuoted (created.patchPath) << ",\n";
    out << "  \"manifestExists\": " << (std::filesystem::exists (created.workManifestPath) ? "true" : "false") << ",\n";
    out << "  \"patchExists\": " << (std::filesystem::exists (created.patchPath) ? "true" : "false") << ",\n";
    out << "  \"workId\": " << jsonQuoted (loadedWork.ok ? loadedWork.manifest.id : std::string {}) << ",\n";
    out << "  \"workTitle\": " << jsonQuoted (loadedWork.ok ? loadedWork.manifest.title : std::string {}) << ",\n";
    out << "  \"mainPatchPath\": " << jsonQuoted (loadedWork.ok ? loadedWork.manifest.mainPatchPath : std::string {}) << ",\n";
    out << "  \"patchId\": " << jsonQuoted (loadedPatch.ok ? loadedPatch.document.id : std::string {}) << ",\n";
    out << "  \"patchTitle\": " << jsonQuoted (loadedPatch.ok ? loadedPatch.document.title : std::string {}) << ",\n";
    out << "  \"duplicateStatus\": " << jsonQuoted (duplicate.status) << ",\n";
    out << "  \"duplicateError\": " << jsonQuoted (duplicate.error) << ",\n";
    out << "  \"error\": " << jsonQuoted (created.error) << ",\n";
    out << "  \"diagnostics\": ";
    appendJsonStringArray (out, created.diagnostics);
    out << "\n";
    out << "}\n";
    return out.str();
}
}

const char* projectCreationProofDisplayName()
{
    return displayName;
}

const char* projectCreationProofDirectoryName()
{
    return directoryName;
}

ProjectCreationProofRunResult runProjectCreationProof (const ProjectCreationProofRunRequest& request)
{
    ProjectCreationProofRunResult result;
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

    const auto createRequest = makeProjectCreationRequest (request.outputDirectory);
    std::error_code removeError;
    std::filesystem::remove_all (createRequest.projectDirectory, removeError);
    if (removeError)
        return fail ("could not reset project creation proof directory: " + removeError.message());

    const auto created = createActiveWorkProject (createRequest);
    const auto duplicate = createActiveWorkProject (createRequest);
    const auto loadedWork = loadWorkProjectManifest (created.workManifestPath);
    const auto loadedPatch = loadMainPatchDocumentForWork (created.workManifestPath);

    if (const auto error = writeProofTextFile (result.reportPath,
                                               makeProjectCreationReportJson (created,
                                                                              duplicate,
                                                                              loadedWork,
                                                                              loadedPatch));
        ! error.empty())
    {
        return fail (error);
    }

    result.ok = created.ok && loadedWork.ok && loadedPatch.ok;
    result.status = result.ok ? "dumped" : "failed";
    result.error = result.ok ? std::string {} : created.error;
    return result;
}
}
