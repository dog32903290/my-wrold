#include "AppStatusProofRunner.h"

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
constexpr const char* displayName = "app status";
constexpr const char* directoryName = "app-status-proof";
constexpr const char* reportFileName = "app_status_report.json";
constexpr const char* statusNodeId = "shader1";
constexpr const char* expectedSaveCommand = "save_work:save-ok commit-pending";

CreateActiveWorkProjectRequest makeAppStatusProofProjectRequest (
    const std::filesystem::path& outputDirectory)
{
    CreateActiveWorkProjectRequest request;
    request.projectDirectory = outputDirectory / "status-project";
    request.workId = "work.status2-proof";
    request.workTitle = "STATUS2 Proof Work";
    request.patchId = "patch.status2-main";
    request.patchTitle = "STATUS2 Main Patch";
    return request;
}

bool commandLogContainsSaveWork (const std::vector<std::string>& commandLog)
{
    return std::find (commandLog.begin(), commandLog.end(), expectedSaveCommand) != commandLog.end();
}

std::string makeAppStatusProofStatusText (const CreateActiveWorkProjectResult& created,
                                          const WorkbenchAppControllerOpenResult& opened,
                                          const CommandResult& saved,
                                          const WorkbenchAppStatusSnapshot& savedStatus)
{
    if (! created.ok)
        return "app status failed: " + created.error;

    if (! opened.ok)
        return "app status failed: " + opened.error;

    if (! saved.ok)
        return "app status failed: " + saved.message;

    return "app status ready: "
           + savedStatus.documentId
           + " source "
           + savedStatus.workSourceStatus
           + " save "
           + savedStatus.saveStatus;
}

std::string makeAppStatusReportJson (const CreateActiveWorkProjectResult& created,
                                     const WorkbenchAppControllerOpenResult& opened,
                                     const CommandResult& saved,
                                     const WorkbenchAppStatusSnapshot& initialStatus,
                                     const WorkbenchAppStatusSnapshot& openedStatus,
                                     const WorkbenchAppStatusSnapshot& savedStatus,
                                     bool dirtyBeforeSave,
                                     bool dirtyAfterSave,
                                     bool hasSaveCommand,
                                     const PatchDocumentLoadResult& reloadedPatch,
                                     const std::vector<std::string>& commandLog,
                                     const std::string& statusText)
{
    const auto ok = ! initialStatus.ok
                    && created.ok
                    && opened.ok
                    && saved.ok
                    && openedStatus.ok
                    && savedStatus.ok
                    && dirtyBeforeSave
                    && ! dirtyAfterSave
                    && hasSaveCommand
                    && reloadedPatch.ok;
    const auto error = ok ? std::string {} : statusText;

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"appStatusReport\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"statusText\": " << jsonQuoted (statusText) << ",\n";
    out << "  \"creationStatus\": " << jsonQuoted (created.status) << ",\n";
    out << "  \"openStatus\": " << jsonQuoted (opened.status) << ",\n";
    out << "  \"initialStatus\": " << jsonQuoted (initialStatus.status) << ",\n";
    out << "  \"initialStatusText\": " << jsonQuoted (initialStatus.statusText) << ",\n";
    out << "  \"initialMessage\": " << jsonQuoted (initialStatus.message) << ",\n";
    out << "  \"openedSnapshotStatus\": " << jsonQuoted (openedStatus.status) << ",\n";
    out << "  \"openedStatusText\": " << jsonQuoted (openedStatus.statusText) << ",\n";
    out << "  \"savedSnapshotStatus\": " << jsonQuoted (savedStatus.status) << ",\n";
    out << "  \"savedStatusText\": " << jsonQuoted (savedStatus.statusText) << ",\n";
    out << "  \"workManifestPath\": " << jsonQuoted (savedStatus.workManifestPath) << ",\n";
    out << "  \"patchPath\": " << jsonQuoted (created.patchPath) << ",\n";
    out << "  \"documentId\": " << jsonQuoted (savedStatus.documentId) << ",\n";
    out << "  \"documentTitle\": " << jsonQuoted (savedStatus.documentTitle) << ",\n";
    out << "  \"workSource\": " << jsonQuoted (savedStatus.workSource) << ",\n";
    out << "  \"workSourceStatus\": " << jsonQuoted (savedStatus.workSourceStatus) << ",\n";
    out << "  \"saveStatus\": " << jsonQuoted (savedStatus.saveStatus) << ",\n";
    out << "  \"proofStatus\": " << jsonQuoted (savedStatus.proofStatus) << ",\n";
    out << "  \"previewStatus\": " << jsonQuoted (savedStatus.previewStatus) << ",\n";
    out << "  \"dirtyBeforeSave\": " << (dirtyBeforeSave ? "true" : "false") << ",\n";
    out << "  \"dirtyAfterSave\": " << (dirtyAfterSave ? "true" : "false") << ",\n";
    out << "  \"graphIOMappingStatus\": " << jsonQuoted (savedStatus.graphIOMappingStatus) << ",\n";
    out << "  \"graphIOMappingCount\": " << savedStatus.graphIOMappingCount << ",\n";
    out << "  \"validGraphIOMappingCount\": " << savedStatus.validGraphIOMappingCount << ",\n";
    out << "  \"commandLogContainsSaveWork\": " << (hasSaveCommand ? "true" : "false") << ",\n";
    out << "  \"reloadedAfterSave\": " << (reloadedPatch.ok ? "true" : "false") << ",\n";
    out << "  \"error\": " << jsonQuoted (error) << ",\n";
    out << "  \"commandLog\": ";
    appendJsonStringArray (out, commandLog);
    out << ",\n";
    out << "  \"creationDiagnostics\": ";
    appendJsonStringArray (out, created.diagnostics);
    out << ",\n";
    out << "  \"workDiagnostics\": ";
    appendJsonStringArray (out, savedStatus.workDiagnostics);
    out << ",\n";
    out << "  \"diagnostics\": ";
    appendJsonStringArray (out, savedStatus.diagnostics);
    out << "\n";
    out << "}\n";
    return out.str();
}
}

const char* appStatusProofDisplayName()
{
    return displayName;
}

const char* appStatusProofDirectoryName()
{
    return directoryName;
}

AppStatusProofRunResult runAppStatusProof (const AppStatusProofRunRequest& request)
{
    AppStatusProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };

    const auto fail = [&] (const std::string& message)
    {
        result.ok = false;
        result.status = "failed";
        result.statusText = "app status failed: " + message;
        result.error = message;
        return result;
    };

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    const auto createRequest = makeAppStatusProofProjectRequest (request.outputDirectory);
    std::error_code removeError;
    std::filesystem::remove_all (createRequest.projectDirectory, removeError);
    if (removeError)
        return fail ("could not reset app status proof directory: " + removeError.message());

    WorkbenchAppController controller;
    const auto initialStatus = controller.appStatusSnapshot();

    const auto created = createActiveWorkProject (createRequest);

    WorkbenchAppControllerOpenRequest openRequest;
    openRequest.activeWorkManifestPath = created.workManifestPath;
    openRequest.candidateRoots = request.candidateRoots;
    openRequest.saveStatus = "clean";
    openRequest.proofStatus = "status2-ready";
    openRequest.previewStatus = "preview-ready";
    const auto opened = controller.openCurrentSession (openRequest);
    const auto openedStatus = controller.appStatusSnapshot();

    auto loadedPatch = loadMainPatchDocumentForWork (created.workManifestPath);
    if (! loadedPatch.ok)
        return fail (loadedPatch.error);

    auto session = makeGraphSession (loadedPatch.document.graph);
    const auto moved = moveNode (session, statusNodeId, 21.0, 8.0);
    if (! moved.ok)
        return fail (moved.message);

    const auto dirtyBeforeSave = session.dirty;
    const auto saved = controller.saveCurrentSession (session);
    const auto dirtyAfterSave = session.dirty;
    const auto savedStatus = controller.appStatusSnapshot();
    const auto reloadedPatch = loadMainPatchDocumentForWork (created.workManifestPath);
    const auto hasSaveCommand = commandLogContainsSaveWork (session.commandLog);
    result.statusText = makeAppStatusProofStatusText (created, opened, saved, savedStatus);

    if (const auto error = writeProofTextFile (
            result.reportPath,
            makeAppStatusReportJson (created,
                                     opened,
                                     saved,
                                     initialStatus,
                                     openedStatus,
                                     savedStatus,
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

    result.ok = ! initialStatus.ok
                && created.ok
                && opened.ok
                && saved.ok
                && openedStatus.ok
                && savedStatus.ok
                && dirtyBeforeSave
                && ! dirtyAfterSave
                && hasSaveCommand
                && reloadedPatch.ok;
    result.status = result.ok ? "dumped" : "failed";
    result.error = result.ok ? std::string {} : result.statusText;
    return result;
}
}
