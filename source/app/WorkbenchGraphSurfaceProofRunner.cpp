#include "WorkbenchGraphSurfaceProofRunner.h"

#include "ActiveWorkService.h"
#include "JsonWriter.h"
#include "ProofRunSupport.h"
#include "WorkbenchAppController.h"
#include "WorkbenchGraphSurface.h"

#include <filesystem>
#include <sstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* displayName = "workbench graph surface";
constexpr const char* directoryName = "workbench-graph-surface-proof";
constexpr const char* reportFileName = "graph_surface_report.json";

CreateActiveWorkProjectRequest makeGraphSurfaceProofProjectRequest (
    const std::filesystem::path& outputDirectory)
{
    CreateActiveWorkProjectRequest request;
    request.projectDirectory = outputDirectory / "graph-surface-project";
    request.workId = "work.graph2-proof";
    request.workTitle = "GRAPH2 Graph Surface Proof Work";
    request.patchId = "patch.graph2-main";
    request.patchTitle = "GRAPH2 Main Patch";
    return request;
}

std::vector<std::string> rowTextsFor (const WorkbenchGraphSurface& surface)
{
    std::vector<std::string> texts;
    texts.reserve (surface.rows.size());

    for (const auto& row : surface.rows)
        texts.push_back (row.text);

    return texts;
}

std::string firstNodeId (const WorkbenchSessionSnapshot& snapshot)
{
    return snapshot.editorNodes.empty() ? "" : snapshot.editorNodes.front().id;
}

std::string firstNodeType (const WorkbenchSessionSnapshot& snapshot)
{
    return snapshot.editorNodes.empty() ? "" : snapshot.editorNodes.front().type;
}

std::string firstEdgeFrom (const WorkbenchSessionSnapshot& snapshot)
{
    return snapshot.editorEdges.empty() ? "" : snapshot.editorEdges.front().from;
}

std::string firstEdgeTo (const WorkbenchSessionSnapshot& snapshot)
{
    return snapshot.editorEdges.empty() ? "" : snapshot.editorEdges.front().to;
}

std::string makeGraphSurfaceProofStatusText (const CreateActiveWorkProjectResult& created,
                                             const WorkbenchAppControllerOpenResult& opened,
                                             const WorkbenchGraphSurface& surface)
{
    if (! created.ok)
        return "workbench graph surface failed: " + created.error;

    if (! opened.ok)
        return "workbench graph surface failed: " + opened.error;

    if (! surface.ok)
        return "workbench graph surface failed: graph surface is blocked";

    return "workbench graph surface ready: "
           + opened.snapshot.documentId
           + " editor "
           + std::to_string (opened.snapshot.editorNodeCount)
           + "/"
           + std::to_string (opened.snapshot.editorEdgeCount);
}

void appendRowsJson (std::ostream& out, const WorkbenchGraphSurface& surface)
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

void appendNodesJson (std::ostream& out, const std::vector<WorkbenchSessionSnapshot::GraphNodeSummary>& nodes)
{
    out << "[\n";

    for (std::size_t index = 0; index < nodes.size(); ++index)
    {
        const auto& node = nodes[index];
        out << "    {\n";
        out << "      \"id\": " << jsonQuoted (node.id) << ",\n";
        out << "      \"type\": " << jsonQuoted (node.type) << ",\n";
        out << "      \"x\": " << node.x << ",\n";
        out << "      \"y\": " << node.y << ",\n";
        out << "      \"collapsed\": " << (node.collapsed ? "true" : "false") << ",\n";
        out << "      \"systemUniformCount\": " << node.systemUniformCount << ",\n";
        out << "      \"paramCount\": " << node.paramCount << ",\n";
        out << "      \"portBindingCount\": " << node.portBindingCount << "\n";
        out << "    }";

        if (index + 1 < nodes.size())
            out << ",";

        out << "\n";
    }

    out << "  ]";
}

void appendEdgesJson (std::ostream& out, const std::vector<WorkbenchSessionSnapshot::GraphEdgeSummary>& edges)
{
    out << "[\n";

    for (std::size_t index = 0; index < edges.size(); ++index)
    {
        const auto& edge = edges[index];
        out << "    {\n";
        out << "      \"id\": " << jsonQuoted (edge.id) << ",\n";
        out << "      \"from\": " << jsonQuoted (edge.from) << ",\n";
        out << "      \"to\": " << jsonQuoted (edge.to) << ",\n";
        out << "      \"dataType\": " << jsonQuoted (edge.dataType) << ",\n";
        out << "      \"streamKind\": " << jsonQuoted (edge.streamKind) << "\n";
        out << "    }";

        if (index + 1 < edges.size())
            out << ",";

        out << "\n";
    }

    out << "  ]";
}

std::string makeGraphSurfaceReportJson (const CreateActiveWorkProjectResult& created,
                                        const WorkbenchAppControllerOpenResult& opened,
                                        const WorkbenchGraphSurface& surface,
                                        const std::string& statusText)
{
    const auto& snapshot = opened.snapshot;
    const auto ok = created.ok
                    && opened.ok
                    && snapshot.ok
                    && surface.ok
                    && surface.rows.size() == 6
                    && snapshot.editorNodeCount > 0
                    && snapshot.editorEdgeCount > 0
                    && ! snapshot.editorNodes.empty()
                    && ! snapshot.editorEdges.empty();
    const auto error = ok ? std::string {} : statusText;
    const auto rowTexts = rowTextsFor (surface);

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"workbenchGraphSurfaceReport\",\n";
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
    out << "  \"editorNodeCount\": " << snapshot.editorNodeCount << ",\n";
    out << "  \"editorEdgeCount\": " << snapshot.editorEdgeCount << ",\n";
    out << "  \"runtimeNodeCount\": " << snapshot.runtimeNodeCount << ",\n";
    out << "  \"runtimeEdgeCount\": " << snapshot.runtimeEdgeCount << ",\n";
    out << "  \"activeOutputNodeId\": " << jsonQuoted (snapshot.activeOutputNodeId) << ",\n";
    out << "  \"firstNodeId\": " << jsonQuoted (firstNodeId (snapshot)) << ",\n";
    out << "  \"firstNodeType\": " << jsonQuoted (firstNodeType (snapshot)) << ",\n";
    out << "  \"firstEdgeFrom\": " << jsonQuoted (firstEdgeFrom (snapshot)) << ",\n";
    out << "  \"firstEdgeTo\": " << jsonQuoted (firstEdgeTo (snapshot)) << ",\n";
    out << "  \"graphIOMappingStatus\": " << jsonQuoted (snapshot.graphIOMappingStatus) << ",\n";
    out << "  \"graphIOMappingCount\": " << snapshot.graphIOMappingCount << ",\n";
    out << "  \"validGraphIOMappingCount\": " << snapshot.validGraphIOMappingCount << ",\n";
    out << "  \"error\": " << jsonQuoted (error) << ",\n";
    out << "  \"rowTexts\": ";
    appendJsonStringArray (out, rowTexts);
    out << ",\n";
    out << "  \"rows\": ";
    appendRowsJson (out, surface);
    out << ",\n";
    out << "  \"nodes\": ";
    appendNodesJson (out, snapshot.editorNodes);
    out << ",\n";
    out << "  \"edges\": ";
    appendEdgesJson (out, snapshot.editorEdges);
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

const char* workbenchGraphSurfaceProofDisplayName()
{
    return displayName;
}

const char* workbenchGraphSurfaceProofDirectoryName()
{
    return directoryName;
}

WorkbenchGraphSurfaceProofRunResult runWorkbenchGraphSurfaceProof (
    const WorkbenchGraphSurfaceProofRunRequest& request)
{
    WorkbenchGraphSurfaceProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };

    const auto fail = [&] (const std::string& message)
    {
        result.ok = false;
        result.status = "failed";
        result.statusText = "workbench graph surface failed: " + message;
        result.error = message;
        return result;
    };

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    const auto createRequest = makeGraphSurfaceProofProjectRequest (request.outputDirectory);
    std::error_code removeError;
    std::filesystem::remove_all (createRequest.projectDirectory, removeError);
    if (removeError)
        return fail ("could not reset workbench graph surface proof directory: " + removeError.message());

    const auto created = createActiveWorkProject (createRequest);

    WorkbenchAppController controller;
    WorkbenchAppControllerOpenRequest openRequest;
    openRequest.activeWorkManifestPath = created.workManifestPath;
    openRequest.candidateRoots = request.candidateRoots;
    openRequest.saveStatus = "clean";
    openRequest.proofStatus = "graph2-ready";
    openRequest.previewStatus = "preview-ready";
    const auto opened = controller.openCurrentSession (openRequest);
    const auto surface = makeWorkbenchGraphSurface (controller.currentSession());
    result.statusText = makeGraphSurfaceProofStatusText (created, opened, surface);

    if (const auto error = writeProofTextFile (
            result.reportPath,
            makeGraphSurfaceReportJson (created, opened, surface, result.statusText));
        ! error.empty())
    {
        return fail (error);
    }

    result.ok = created.ok
                && opened.ok
                && opened.snapshot.ok
                && surface.ok
                && surface.rows.size() == 6
                && opened.snapshot.editorNodeCount > 0
                && opened.snapshot.editorEdgeCount > 0
                && ! opened.snapshot.editorNodes.empty()
                && ! opened.snapshot.editorEdges.empty();
    result.status = result.ok ? "dumped" : "failed";
    result.error = result.ok ? std::string {} : result.statusText;
    return result;
}
}
