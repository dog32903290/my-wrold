#include "AppSaveProofRunner.h"

#include "ActiveWorkService.h"
#include "InteractionContract.h"
#include "JsonWriter.h"
#include "ProofRunSupport.h"
#include "StorageContract.h"
#include "WorkbenchAppController.h"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* displayName = "app save";
constexpr const char* directoryName = "app-save-proof";
constexpr const char* reportFileName = "app_save_report.json";
constexpr const char* saveNodeId = "shader1";
constexpr const char* expectedSaveCommand = "save_work:save-ok commit-pending";

CreateActiveWorkProjectRequest makeAppSaveProofProjectRequest (const std::filesystem::path& outputDirectory)
{
    CreateActiveWorkProjectRequest request;
    request.projectDirectory = outputDirectory / "save-project";
    request.workId = "work.save2-proof";
    request.workTitle = "SAVE2 Proof Work";
    request.patchId = "patch.save2-main";
    request.patchTitle = "SAVE2 Main Patch";
    return request;
}

std::filesystem::path saveLogPathForProofWork (const std::filesystem::path& workManifestPath)
{
    return workManifestPath.parent_path() / ".myworld" / "save_log.jsonl";
}

bool commandLogContainsSaveWork (const std::vector<std::string>& commandLog)
{
    return std::find (commandLog.begin(), commandLog.end(), expectedSaveCommand) != commandLog.end();
}

std::string makeAppSaveStatusText (const CreateActiveWorkProjectResult& created,
                                   const WorkbenchAppControllerOpenResult& opened,
                                   const CommandResult& saved)
{
    if (! created.ok)
        return "app save failed: " + created.error;

    if (! opened.ok)
        return "app save failed: " + opened.error;

    if (! saved.ok)
        return "app save failed: " + saved.message;

    return "app save ready: "
           + opened.snapshot.documentId
           + " source "
           + opened.snapshot.workSourceStatus
           + " save "
           + saved.message;
}

std::string makeAppSaveReportJson (const CreateActiveWorkProjectResult& created,
                                   const WorkbenchAppControllerOpenResult& opened,
                                   const CommandResult& saved,
                                   const std::string& controllerStatusText,
                                   const std::filesystem::path& saveLogPath,
                                   bool dirtyBeforeSave,
                                   bool dirtyAfterSave,
                                   bool hasSaveCommand,
                                   const PatchDocumentLoadResult& reloadedPatch,
                                   const std::vector<std::string>& commandLog,
                                   const std::string& statusText)
{
    const auto ok = created.ok
                    && opened.ok
                    && saved.ok
                    && dirtyBeforeSave
                    && ! dirtyAfterSave
                    && hasSaveCommand
                    && reloadedPatch.ok;
    const auto error = ok ? std::string {}
                          : (created.ok ? (opened.ok ? saved.message : opened.error) : created.error);

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"appSaveReport\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"creationStatus\": " << jsonQuoted (created.status) << ",\n";
    out << "  \"openStatus\": " << jsonQuoted (opened.status) << ",\n";
    out << "  \"saveStatus\": " << jsonQuoted (saved.message) << ",\n";
    out << "  \"controllerStatusText\": " << jsonQuoted (controllerStatusText) << ",\n";
    out << "  \"workManifestPath\": " << jsonQuoted (opened.snapshot.workManifestPath) << ",\n";
    out << "  \"patchPath\": " << jsonQuoted (created.patchPath) << ",\n";
    out << "  \"saveLogPath\": " << jsonQuoted (saveLogPath.string()) << ",\n";
    out << "  \"saveLogExists\": " << (std::filesystem::exists (saveLogPath) ? "true" : "false") << ",\n";
    out << "  \"documentId\": " << jsonQuoted (opened.snapshot.documentId) << ",\n";
    out << "  \"workSource\": " << jsonQuoted (opened.snapshot.workSource) << ",\n";
    out << "  \"workSourceStatus\": " << jsonQuoted (opened.snapshot.workSourceStatus) << ",\n";
    out << "  \"dirtyBeforeSave\": " << (dirtyBeforeSave ? "true" : "false") << ",\n";
    out << "  \"dirtyAfterSave\": " << (dirtyAfterSave ? "true" : "false") << ",\n";
    out << "  \"commandLogContainsSaveWork\": " << (hasSaveCommand ? "true" : "false") << ",\n";
    out << "  \"reloadedAfterSave\": " << (reloadedPatch.ok ? "true" : "false") << ",\n";
    out << "  \"statusText\": " << jsonQuoted (statusText) << ",\n";
    out << "  \"error\": " << jsonQuoted (error) << ",\n";
    out << "  \"commandLog\": ";
    appendJsonStringArray (out, commandLog);
    out << ",\n";
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

const char* appSaveProofDisplayName()
{
    return displayName;
}

const char* appSaveProofDirectoryName()
{
    return directoryName;
}

AppSaveProofRunResult runAppSaveProof (const AppSaveProofRunRequest& request)
{
    AppSaveProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };

    const auto fail = [&] (const std::string& message)
    {
        result.ok = false;
        result.status = "failed";
        result.statusText = "app save failed: " + message;
        result.error = message;
        return result;
    };

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    const auto createRequest = makeAppSaveProofProjectRequest (request.outputDirectory);
    std::error_code removeError;
    std::filesystem::remove_all (createRequest.projectDirectory, removeError);
    if (removeError)
        return fail ("could not reset app save proof directory: " + removeError.message());

    const auto created = createActiveWorkProject (createRequest);

    WorkbenchAppController controller;
    WorkbenchAppControllerOpenRequest openRequest;
    openRequest.activeWorkManifestPath = created.workManifestPath;
    openRequest.candidateRoots = request.candidateRoots;
    openRequest.saveStatus = "clean";
    openRequest.proofStatus = "save2-ready";
    openRequest.previewStatus = "preview-ready";
    const auto opened = controller.openCurrentSession (openRequest);

    auto loadedPatch = loadMainPatchDocumentForWork (created.workManifestPath);
    if (! loadedPatch.ok)
        return fail (loadedPatch.error);

    auto session = makeGraphSession (loadedPatch.document.graph);
    const auto moved = moveNode (session, saveNodeId, 13.0, 7.0);
    if (! moved.ok)
        return fail (moved.message);

    const auto dirtyBeforeSave = session.dirty;
    const auto saved = controller.saveCurrentSession (session);
    const auto dirtyAfterSave = session.dirty;
    const auto reloadedPatch = loadMainPatchDocumentForWork (created.workManifestPath);
    const auto hasSaveCommand = commandLogContainsSaveWork (session.commandLog);
    result.statusText = makeAppSaveStatusText (created, opened, saved);

    const auto saveLogPath = saveLogPathForProofWork (created.workManifestPath);
    if (const auto error = writeProofTextFile (
            result.reportPath,
            makeAppSaveReportJson (created,
                                   opened,
                                   saved,
                                   controller.statusText(),
                                   saveLogPath,
                                   dirtyBeforeSave,
                                   dirtyAfterSave,
                                   hasSaveCommand,
                                   reloadedPatch,
                                   session.commandLog,
                                   result.statusText));
        ! error.empty())
    {
        return fail (error);
    }

    result.ok = created.ok
                && opened.ok
                && saved.ok
                && dirtyBeforeSave
                && ! dirtyAfterSave
                && hasSaveCommand
                && reloadedPatch.ok;
    result.status = result.ok ? "dumped" : "failed";
    result.error = result.ok ? std::string {} : result.statusText;
    return result;
}
}
