#include "WorkbenchCookPlanSurfaceProofRunner.h"

#include "ActiveWorkService.h"
#include "JsonWriter.h"
#include "ProofRunSupport.h"
#include "WorkbenchAppController.h"
#include "WorkbenchCookPlanSurface.h"

#include <filesystem>
#include <sstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* displayName = "workbench cook plan";
constexpr const char* directoryName = "workbench-cook-plan-proof";
constexpr const char* reportFileName = "cook_plan_report.json";

CreateActiveWorkProjectRequest makeCookPlanProofProjectRequest (
    const std::filesystem::path& outputDirectory)
{
    CreateActiveWorkProjectRequest request;
    request.projectDirectory = outputDirectory / "cook-plan-project";
    request.workId = "work.cook2-proof";
    request.workTitle = "COOK2 Cook Plan Proof Work";
    request.patchId = "patch.cook2-main";
    request.patchTitle = "COOK2 Main Patch";
    return request;
}

std::vector<std::string> rowTextsFor (const WorkbenchCookPlanSurface& surface)
{
    std::vector<std::string> texts;
    texts.reserve (surface.rows.size());

    for (const auto& row : surface.rows)
        texts.push_back (row.text);

    return texts;
}

std::string rowValueFor (const WorkbenchCookPlanSurface& surface,
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

std::string cookOrderTextFor (const WorkbenchCookPlanSurface& surface)
{
    return rowValueFor (surface, "order", "none");
}

std::string makeCookPlanProofStatusText (const CreateActiveWorkProjectResult& created,
                                         const WorkbenchAppControllerOpenResult& opened,
                                         const WorkbenchCookPlanSurface& surface)
{
    if (! created.ok)
        return "workbench cook plan failed: " + created.error;

    if (! opened.ok)
        return "workbench cook plan failed: " + opened.error;

    if (! surface.ok)
        return "workbench cook plan failed: cook plan is blocked";

    return "workbench cook plan ready: "
           + opened.snapshot.documentId
           + " order "
           + cookOrderTextFor (surface);
}

void appendRowsJson (std::ostream& out, const WorkbenchCookPlanSurface& surface)
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

std::string makeCookPlanReportJson (const CreateActiveWorkProjectResult& created,
                                    const WorkbenchAppControllerOpenResult& opened,
                                    const WorkbenchCookPlanSurface& surface,
                                    const std::string& statusText)
{
    const auto& snapshot = opened.snapshot;
    const auto cookOrderText = cookOrderTextFor (surface);
    const auto targetNodeId = rowValueFor (surface, "target", "none");
    const auto readiness = rowValueFor (surface, "readiness", "unknown");
    const auto executionStatus = rowValueFor (surface, "execution", "unknown");
    const auto ok = created.ok
                    && opened.ok
                    && snapshot.ok
                    && surface.ok
                    && surface.rows.size() == 6
                    && snapshot.runtimeNodeCount > 0
                    && snapshot.runtimeEdgeCount > 0
                    && ! surface.cookOrder.empty()
                    && readiness == "ready"
                    && executionStatus == "parked";
    const auto error = ok ? std::string {} : statusText;
    const auto rowTexts = rowTextsFor (surface);

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"workbenchCookPlanReport\",\n";
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
    out << "  \"cookOrder\": ";
    appendJsonStringArray (out, surface.cookOrder);
    out << ",\n";
    out << "  \"cookOrderText\": " << jsonQuoted (cookOrderText) << ",\n";
    out << "  \"targetNodeId\": " << jsonQuoted (targetNodeId) << ",\n";
    out << "  \"readiness\": " << jsonQuoted (readiness) << ",\n";
    out << "  \"executionStatus\": " << jsonQuoted (executionStatus) << ",\n";
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

const char* workbenchCookPlanProofDisplayName()
{
    return displayName;
}

const char* workbenchCookPlanProofDirectoryName()
{
    return directoryName;
}

WorkbenchCookPlanSurfaceProofRunResult runWorkbenchCookPlanSurfaceProof (
    const WorkbenchCookPlanSurfaceProofRunRequest& request)
{
    WorkbenchCookPlanSurfaceProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };

    const auto fail = [&] (const std::string& message)
    {
        result.ok = false;
        result.status = "failed";
        result.statusText = "workbench cook plan failed: " + message;
        result.error = message;
        return result;
    };

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    const auto createRequest = makeCookPlanProofProjectRequest (request.outputDirectory);
    std::error_code removeError;
    std::filesystem::remove_all (createRequest.projectDirectory, removeError);
    if (removeError)
        return fail ("could not reset workbench cook plan proof directory: " + removeError.message());

    const auto created = createActiveWorkProject (createRequest);

    WorkbenchAppController controller;
    WorkbenchAppControllerOpenRequest openRequest;
    openRequest.activeWorkManifestPath = created.workManifestPath;
    openRequest.candidateRoots = request.candidateRoots;
    openRequest.saveStatus = "clean";
    openRequest.proofStatus = "cook2-ready";
    openRequest.previewStatus = "preview-ready";
    const auto opened = controller.openCurrentSession (openRequest);
    const auto surface = makeWorkbenchCookPlanSurface (controller.currentSession());
    result.statusText = makeCookPlanProofStatusText (created, opened, surface);

    if (const auto error = writeProofTextFile (
            result.reportPath,
            makeCookPlanReportJson (created, opened, surface, result.statusText));
        ! error.empty())
    {
        return fail (error);
    }

    const auto readiness = rowValueFor (surface, "readiness", "unknown");
    const auto executionStatus = rowValueFor (surface, "execution", "unknown");
    result.ok = created.ok
                && opened.ok
                && opened.snapshot.ok
                && surface.ok
                && surface.rows.size() == 6
                && opened.snapshot.runtimeNodeCount > 0
                && opened.snapshot.runtimeEdgeCount > 0
                && ! surface.cookOrder.empty()
                && readiness == "ready"
                && executionStatus == "parked";
    result.status = result.ok ? "dumped" : "failed";
    result.error = result.ok ? std::string {} : result.statusText;
    return result;
}
}
