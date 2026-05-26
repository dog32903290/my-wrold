#include "WorkbenchRuntimeSurfaceProofRunner.h"

#include "ActiveWorkService.h"
#include "JsonWriter.h"
#include "ProofRunSupport.h"
#include "WorkbenchAppController.h"
#include "WorkbenchRuntimeSurface.h"

#include <filesystem>
#include <sstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* displayName = "workbench runtime surface";
constexpr const char* directoryName = "workbench-runtime-surface-proof";
constexpr const char* reportFileName = "runtime_surface_report.json";

CreateActiveWorkProjectRequest makeRuntimeSurfaceProofProjectRequest (
    const std::filesystem::path& outputDirectory)
{
    CreateActiveWorkProjectRequest request;
    request.projectDirectory = outputDirectory / "runtime-surface-project";
    request.workId = "work.runtime2-proof";
    request.workTitle = "RUNTIME2 Runtime Surface Proof Work";
    request.patchId = "patch.runtime2-main";
    request.patchTitle = "RUNTIME2 Main Patch";
    return request;
}

std::vector<std::string> rowTextsFor (const WorkbenchRuntimeSurface& surface)
{
    std::vector<std::string> texts;
    texts.reserve (surface.rows.size());

    for (const auto& row : surface.rows)
        texts.push_back (row.text);

    return texts;
}

std::string rowValueFor (const WorkbenchRuntimeSurface& surface,
                         const std::string& rowId,
                         const std::string& fallbackValue)
{
    for (const auto& row : surface.rows)
    {
        if (row.id == rowId)
            return row.value;
    }

    return fallbackValue;
}

std::string makeRuntimeSurfaceProofStatusText (const CreateActiveWorkProjectResult& created,
                                               const WorkbenchAppControllerOpenResult& opened,
                                               const WorkbenchRuntimeSurface& surface)
{
    if (! created.ok)
        return "workbench runtime surface failed: " + created.error;

    if (! opened.ok)
        return "workbench runtime surface failed: " + opened.error;

    if (! surface.ok)
        return "workbench runtime surface failed: runtime surface is blocked";

    return "workbench runtime surface ready: "
           + opened.snapshot.documentId
           + " runtime "
           + std::to_string (opened.snapshot.runtimeNodeCount)
           + "/"
           + std::to_string (opened.snapshot.runtimeEdgeCount);
}

void appendRowsJson (std::ostream& out, const WorkbenchRuntimeSurface& surface)
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

std::string makeRuntimeSurfaceReportJson (const CreateActiveWorkProjectResult& created,
                                          const WorkbenchAppControllerOpenResult& opened,
                                          const WorkbenchRuntimeSurface& surface,
                                          const std::string& statusText)
{
    const auto& snapshot = opened.snapshot;
    const auto readiness = rowValueFor (surface, "readiness", "unknown");
    const auto cookStatus = rowValueFor (surface, "cook", "unknown");
    const auto ok = created.ok
                    && opened.ok
                    && snapshot.ok
                    && surface.ok
                    && surface.rows.size() == 6
                    && snapshot.runtimeNodeCount > 0
                    && snapshot.runtimeEdgeCount > 0
                    && readiness == "ready"
                    && cookStatus == "parked";
    const auto error = ok ? std::string {} : statusText;
    const auto rowTexts = rowTextsFor (surface);

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"workbenchRuntimeSurfaceReport\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"statusText\": " << jsonQuoted (statusText) << ",\n";
    out << "  \"headline\": " << jsonQuoted (surface.headline) << ",\n";
    out << "  \"rowCount\": " << surface.rows.size() << ",\n";
    out << "  \"creationStatus\": " << jsonQuoted (created.status) << ",\n";
    out << "  \"openStatus\": " << jsonQuoted (opened.status) << ",\n";
    out << "  \"workManifestPath\": " << jsonQuoted (snapshot.workManifestPath) << ",\n";
    out << "  \"patchPath\": " << jsonQuoted (created.patchPath) << ",\n";
    out << "  \"documentId\": " << jsonQuoted (snapshot.documentId) << ",\n";
    out << "  \"documentTitle\": " << jsonQuoted (snapshot.documentTitle) << ",\n";
    out << "  \"workSource\": " << jsonQuoted (snapshot.workSource) << ",\n";
    out << "  \"workSourceStatus\": " << jsonQuoted (snapshot.workSourceStatus) << ",\n";
    out << "  \"runtimeNodeCount\": " << snapshot.runtimeNodeCount << ",\n";
    out << "  \"runtimeEdgeCount\": " << snapshot.runtimeEdgeCount << ",\n";
    out << "  \"activeOutputNodeId\": " << jsonQuoted (snapshot.activeOutputNodeId) << ",\n";
    out << "  \"graphIOMappingStatus\": " << jsonQuoted (snapshot.graphIOMappingStatus) << ",\n";
    out << "  \"graphIOMappingCount\": " << snapshot.graphIOMappingCount << ",\n";
    out << "  \"validGraphIOMappingCount\": " << snapshot.validGraphIOMappingCount << ",\n";
    out << "  \"readiness\": " << jsonQuoted (readiness) << ",\n";
    out << "  \"cookStatus\": " << jsonQuoted (cookStatus) << ",\n";
    out << "  \"error\": " << jsonQuoted (error) << ",\n";
    out << "  \"rowTexts\": ";
    appendJsonStringArray (out, rowTexts);
    out << ",\n";
    out << "  \"rows\": ";
    appendRowsJson (out, surface);
    out << ",\n";
    out << "  \"creationDiagnostics\": ";
    appendJsonStringArray (out, created.diagnostics);
    out << ",\n";
    out << "  \"workDiagnostics\": ";
    appendJsonStringArray (out, snapshot.workDiagnostics);
    out << ",\n";
    out << "  \"diagnostics\": ";
    appendJsonStringArray (out, snapshot.diagnostics);
    out << "\n";
    out << "}\n";
    return out.str();
}
}

const char* workbenchRuntimeSurfaceProofDisplayName()
{
    return displayName;
}

const char* workbenchRuntimeSurfaceProofDirectoryName()
{
    return directoryName;
}

WorkbenchRuntimeSurfaceProofRunResult runWorkbenchRuntimeSurfaceProof (
    const WorkbenchRuntimeSurfaceProofRunRequest& request)
{
    WorkbenchRuntimeSurfaceProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };

    const auto fail = [&] (const std::string& message)
    {
        result.ok = false;
        result.status = "failed";
        result.statusText = "workbench runtime surface failed: " + message;
        result.error = message;
        return result;
    };

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    const auto createRequest = makeRuntimeSurfaceProofProjectRequest (request.outputDirectory);
    std::error_code removeError;
    std::filesystem::remove_all (createRequest.projectDirectory, removeError);
    if (removeError)
        return fail ("could not reset workbench runtime surface proof directory: " + removeError.message());

    const auto created = createActiveWorkProject (createRequest);

    WorkbenchAppController controller;
    WorkbenchAppControllerOpenRequest openRequest;
    openRequest.activeWorkManifestPath = created.workManifestPath;
    openRequest.candidateRoots = request.candidateRoots;
    openRequest.saveStatus = "clean";
    openRequest.proofStatus = "runtime2-ready";
    openRequest.previewStatus = "preview-ready";
    const auto opened = controller.openCurrentSession (openRequest);
    const auto surface = makeWorkbenchRuntimeSurface (controller.currentSession());
    result.statusText = makeRuntimeSurfaceProofStatusText (created, opened, surface);

    if (const auto error = writeProofTextFile (
            result.reportPath,
            makeRuntimeSurfaceReportJson (created, opened, surface, result.statusText));
        ! error.empty())
    {
        return fail (error);
    }

    const auto readiness = rowValueFor (surface, "readiness", "unknown");
    const auto cookStatus = rowValueFor (surface, "cook", "unknown");
    result.ok = created.ok
                && opened.ok
                && opened.snapshot.ok
                && surface.ok
                && surface.rows.size() == 6
                && opened.snapshot.runtimeNodeCount > 0
                && opened.snapshot.runtimeEdgeCount > 0
                && readiness == "ready"
                && cookStatus == "parked";
    result.status = result.ok ? "dumped" : "failed";
    result.error = result.ok ? std::string {} : result.statusText;
    return result;
}
}
