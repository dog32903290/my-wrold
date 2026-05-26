#include "WorkbenchStatusSurfaceProofRunner.h"

#include "ActiveWorkService.h"
#include "InteractionContract.h"
#include "JsonWriter.h"
#include "ProofRunSupport.h"
#include "StorageContract.h"
#include "WorkbenchAppController.h"
#include "WorkbenchStatusSurface.h"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* displayName = "workbench status surface";
constexpr const char* directoryName = "workbench-status-surface-proof";
constexpr const char* reportFileName = "workbench_status_surface_report.json";
constexpr const char* statusSurfaceNodeId = "shader1";
constexpr const char* expectedSaveCommand = "save_work:save-ok commit-pending";

CreateActiveWorkProjectRequest makeStatusSurfaceProofProjectRequest (
    const std::filesystem::path& outputDirectory)
{
    CreateActiveWorkProjectRequest request;
    request.projectDirectory = outputDirectory / "status-surface-project";
    request.workId = "work.ui2-proof";
    request.workTitle = "UI2 Status Surface Proof Work";
    request.patchId = "patch.ui2-main";
    request.patchTitle = "UI2 Main Patch";
    return request;
}

bool commandLogContainsSaveWork (const std::vector<std::string>& commandLog)
{
    return std::find (commandLog.begin(), commandLog.end(), expectedSaveCommand) != commandLog.end();
}

std::vector<std::string> rowTextsFor (const WorkbenchStatusSurface& surface)
{
    std::vector<std::string> texts;
    texts.reserve (surface.rows.size());

    for (const auto& row : surface.rows)
        texts.push_back (row.text);

    return texts;
}

std::string makeStatusSurfaceProofStatusText (const CreateActiveWorkProjectResult& created,
                                              const WorkbenchAppControllerOpenResult& opened,
                                              const CommandResult& saved,
                                              const WorkbenchAppStatusSnapshot& savedStatus,
                                              const WorkbenchStatusSurface& surface)
{
    if (! created.ok)
        return "workbench status surface failed: " + created.error;

    if (! opened.ok)
        return "workbench status surface failed: " + opened.error;

    if (! saved.ok)
        return "workbench status surface failed: " + saved.message;

    if (! savedStatus.ok)
        return "workbench status surface failed: app status snapshot is blocked";

    return "workbench status surface ready: "
           + savedStatus.documentId
           + " rows "
           + std::to_string (surface.rows.size());
}

void appendSurfaceRowsJson (std::ostream& out, const WorkbenchStatusSurface& surface)
{
    out << "[\n";

    for (std::size_t index = 0; index < surface.rows.size(); ++index)
    {
        const auto& row = surface.rows[index];
        out << "    {\n";
        out << "      \"id\": " << jsonQuoted (row.id) << ",\n";
        out << "      \"label\": " << jsonQuoted (row.label) << ",\n";
        out << "      \"value\": " << jsonQuoted (row.value) << ",\n";
        out << "      \"text\": " << jsonQuoted (row.text) << ",\n";
        out << "      \"tone\": " << jsonQuoted (row.tone) << "\n";
        out << "    }";

        if (index + 1 < surface.rows.size())
            out << ",";

        out << "\n";
    }

    out << "  ]";
}

std::string makeStatusSurfaceReportJson (const CreateActiveWorkProjectResult& created,
                                         const WorkbenchAppControllerOpenResult& opened,
                                         const CommandResult& saved,
                                         const WorkbenchAppStatusSnapshot& savedStatus,
                                         const WorkbenchStatusSurface& surface,
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
                    && savedStatus.ok
                    && surface.ok
                    && surface.rows.size() == 6
                    && dirtyBeforeSave
                    && ! dirtyAfterSave
                    && hasSaveCommand
                    && reloadedPatch.ok;
    const auto error = ok ? std::string {} : statusText;
    const auto rowTexts = rowTextsFor (surface);

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"workbenchStatusSurfaceReport\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"statusText\": " << jsonQuoted (statusText) << ",\n";
    out << "  \"headline\": " << jsonQuoted (surface.headline) << ",\n";
    out << "  \"rowCount\": " << surface.rows.size() << ",\n";
    out << "  \"creationStatus\": " << jsonQuoted (created.status) << ",\n";
    out << "  \"openStatus\": " << jsonQuoted (opened.status) << ",\n";
    out << "  \"saveStatus\": " << jsonQuoted (savedStatus.saveStatus) << ",\n";
    out << "  \"workManifestPath\": " << jsonQuoted (savedStatus.workManifestPath) << ",\n";
    out << "  \"patchPath\": " << jsonQuoted (created.patchPath) << ",\n";
    out << "  \"documentId\": " << jsonQuoted (savedStatus.documentId) << ",\n";
    out << "  \"documentTitle\": " << jsonQuoted (savedStatus.documentTitle) << ",\n";
    out << "  \"workSource\": " << jsonQuoted (savedStatus.workSource) << ",\n";
    out << "  \"workSourceStatus\": " << jsonQuoted (savedStatus.workSourceStatus) << ",\n";
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
    out << "  \"rowTexts\": ";
    appendJsonStringArray (out, rowTexts);
    out << ",\n";
    out << "  \"rows\": ";
    appendSurfaceRowsJson (out, surface);
    out << ",\n";
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

const char* workbenchStatusSurfaceProofDisplayName()
{
    return displayName;
}

const char* workbenchStatusSurfaceProofDirectoryName()
{
    return directoryName;
}

WorkbenchStatusSurfaceProofRunResult runWorkbenchStatusSurfaceProof (
    const WorkbenchStatusSurfaceProofRunRequest& request)
{
    WorkbenchStatusSurfaceProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };

    const auto fail = [&] (const std::string& message)
    {
        result.ok = false;
        result.status = "failed";
        result.statusText = "workbench status surface failed: " + message;
        result.error = message;
        return result;
    };

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    const auto createRequest = makeStatusSurfaceProofProjectRequest (request.outputDirectory);
    std::error_code removeError;
    std::filesystem::remove_all (createRequest.projectDirectory, removeError);
    if (removeError)
        return fail ("could not reset workbench status surface proof directory: " + removeError.message());

    const auto created = createActiveWorkProject (createRequest);

    WorkbenchAppController controller;
    WorkbenchAppControllerOpenRequest openRequest;
    openRequest.activeWorkManifestPath = created.workManifestPath;
    openRequest.candidateRoots = request.candidateRoots;
    openRequest.saveStatus = "clean";
    openRequest.proofStatus = "ui2-ready";
    openRequest.previewStatus = "preview-ready";
    const auto opened = controller.openCurrentSession (openRequest);

    auto loadedPatch = loadMainPatchDocumentForWork (created.workManifestPath);
    if (! loadedPatch.ok)
        return fail (loadedPatch.error);

    auto session = makeGraphSession (loadedPatch.document.graph);
    const auto moved = moveNode (session, statusSurfaceNodeId, 34.0, 12.0);
    if (! moved.ok)
        return fail (moved.message);

    const auto dirtyBeforeSave = session.dirty;
    const auto saved = controller.saveCurrentSession (session);
    const auto dirtyAfterSave = session.dirty;
    const auto savedStatus = controller.appStatusSnapshot();
    const auto surface = makeWorkbenchStatusSurface (savedStatus);
    const auto reloadedPatch = loadMainPatchDocumentForWork (created.workManifestPath);
    const auto hasSaveCommand = commandLogContainsSaveWork (session.commandLog);
    result.statusText = makeStatusSurfaceProofStatusText (created, opened, saved, savedStatus, surface);

    if (const auto error = writeProofTextFile (
            result.reportPath,
            makeStatusSurfaceReportJson (created,
                                         opened,
                                         saved,
                                         savedStatus,
                                         surface,
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
                && savedStatus.ok
                && surface.ok
                && surface.rows.size() == 6
                && dirtyBeforeSave
                && ! dirtyAfterSave
                && hasSaveCommand
                && reloadedPatch.ok;
    result.status = result.ok ? "dumped" : "failed";
    result.error = result.ok ? std::string {} : result.statusText;
    return result;
}
}
