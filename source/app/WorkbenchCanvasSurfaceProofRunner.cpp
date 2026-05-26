#include "WorkbenchCanvasSurfaceProofRunner.h"

#include "ActiveWorkService.h"
#include "JsonWriter.h"
#include "ProofRunSupport.h"
#include "WorkbenchAppController.h"
#include "WorkbenchCanvasSurface.h"

#include <filesystem>
#include <sstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* displayName = "workbench canvas surface";
constexpr const char* directoryName = "workbench-canvas-surface-proof";
constexpr const char* reportFileName = "canvas_surface_report.json";

CreateActiveWorkProjectRequest makeCanvasSurfaceProofProjectRequest (
    const std::filesystem::path& outputDirectory)
{
    CreateActiveWorkProjectRequest request;
    request.projectDirectory = outputDirectory / "canvas-surface-project";
    request.workId = "work.canvas2-proof";
    request.workTitle = "CANVAS2 Canvas Surface Proof Work";
    request.patchId = "patch.canvas2-main";
    request.patchTitle = "CANVAS2 Main Patch";
    return request;
}

std::vector<std::string> rowTextsFor (const WorkbenchCanvasSurface& surface)
{
    std::vector<std::string> texts;
    texts.reserve (surface.rows.size());

    for (const auto& row : surface.rows)
        texts.push_back (row.text);

    return texts;
}

const WorkbenchCanvasNodeSurface& firstNodeSurface (const WorkbenchCanvasSurface& surface)
{
    static const WorkbenchCanvasNodeSurface empty;
    return surface.nodeSurfaces.empty() ? empty : surface.nodeSurfaces.front();
}

const WorkbenchCanvasEdgeRoute& firstEdgeRoute (const WorkbenchCanvasSurface& surface)
{
    static const WorkbenchCanvasEdgeRoute empty;
    return surface.edgeRoutes.empty() ? empty : surface.edgeRoutes.front();
}

std::string makeCanvasSurfaceProofStatusText (const CreateActiveWorkProjectResult& created,
                                              const WorkbenchAppControllerOpenResult& opened,
                                              const WorkbenchCanvasSurface& surface)
{
    if (! created.ok)
        return "workbench canvas surface failed: " + created.error;

    if (! opened.ok)
        return "workbench canvas surface failed: " + opened.error;

    if (! surface.ok)
        return "workbench canvas surface failed: canvas surface is blocked";

    return "workbench canvas surface ready: "
           + opened.snapshot.documentId
           + " nodes "
           + std::to_string (surface.nodeSurfaces.size())
           + " routes "
           + std::to_string (surface.edgeRoutes.size());
}

void appendRowsJson (std::ostream& out, const WorkbenchCanvasSurface& surface)
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

void appendNodeSurfacesJson (std::ostream& out, const WorkbenchCanvasSurface& surface)
{
    out << "[\n";

    for (std::size_t index = 0; index < surface.nodeSurfaces.size(); ++index)
    {
        const auto& node = surface.nodeSurfaces[index];
        out << "    {\n";
        out << "      \"id\": " << jsonQuoted (node.id) << ",\n";
        out << "      \"type\": " << jsonQuoted (node.type) << ",\n";
        out << "      \"x\": " << node.x << ",\n";
        out << "      \"y\": " << node.y << ",\n";
        out << "      \"width\": " << node.width << ",\n";
        out << "      \"height\": " << node.height << "\n";
        out << "    }";

        if (index + 1 < surface.nodeSurfaces.size())
            out << ",";

        out << "\n";
    }

    out << "  ]";
}

void appendEdgeRoutesJson (std::ostream& out, const WorkbenchCanvasSurface& surface)
{
    out << "[\n";

    for (std::size_t index = 0; index < surface.edgeRoutes.size(); ++index)
    {
        const auto& route = surface.edgeRoutes[index];
        out << "    {\n";
        out << "      \"id\": " << jsonQuoted (route.id) << ",\n";
        out << "      \"from\": " << jsonQuoted (route.from) << ",\n";
        out << "      \"to\": " << jsonQuoted (route.to) << ",\n";
        out << "      \"dataType\": " << jsonQuoted (route.dataType) << ",\n";
        out << "      \"streamKind\": " << jsonQuoted (route.streamKind) << ",\n";
        out << "      \"fromX\": " << route.fromX << ",\n";
        out << "      \"fromY\": " << route.fromY << ",\n";
        out << "      \"toX\": " << route.toX << ",\n";
        out << "      \"toY\": " << route.toY << "\n";
        out << "    }";

        if (index + 1 < surface.edgeRoutes.size())
            out << ",";

        out << "\n";
    }

    out << "  ]";
}

std::string makeCanvasSurfaceReportJson (const CreateActiveWorkProjectResult& created,
                                         const WorkbenchAppControllerOpenResult& opened,
                                         const WorkbenchCanvasSurface& surface,
                                         const std::string& statusText)
{
    const auto& snapshot = opened.snapshot;
    const auto& firstNode = firstNodeSurface (surface);
    const auto& firstRoute = firstEdgeRoute (surface);
    const auto ok = created.ok
                    && opened.ok
                    && snapshot.ok
                    && surface.ok
                    && surface.rows.size() == 6
                    && ! surface.nodeSurfaces.empty()
                    && ! surface.edgeRoutes.empty();
    const auto error = ok ? std::string {} : statusText;
    const auto rowTexts = rowTextsFor (surface);

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"workbenchCanvasSurfaceReport\",\n";
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
    out << "  \"nodeSurfaceCount\": " << surface.nodeSurfaces.size() << ",\n";
    out << "  \"edgeRouteCount\": " << surface.edgeRoutes.size() << ",\n";
    out << "  \"firstNodeId\": " << jsonQuoted (firstNode.id) << ",\n";
    out << "  \"firstNodeType\": " << jsonQuoted (firstNode.type) << ",\n";
    out << "  \"firstNodeX\": " << firstNode.x << ",\n";
    out << "  \"firstNodeY\": " << firstNode.y << ",\n";
    out << "  \"firstNodeWidth\": " << firstNode.width << ",\n";
    out << "  \"firstNodeHeight\": " << firstNode.height << ",\n";
    out << "  \"firstRouteFrom\": " << jsonQuoted (firstRoute.from) << ",\n";
    out << "  \"firstRouteTo\": " << jsonQuoted (firstRoute.to) << ",\n";
    out << "  \"firstRouteDataType\": " << jsonQuoted (firstRoute.dataType) << ",\n";
    out << "  \"firstRouteStreamKind\": " << jsonQuoted (firstRoute.streamKind) << ",\n";
    out << "  \"firstRouteFromX\": " << firstRoute.fromX << ",\n";
    out << "  \"firstRouteFromY\": " << firstRoute.fromY << ",\n";
    out << "  \"firstRouteToX\": " << firstRoute.toX << ",\n";
    out << "  \"firstRouteToY\": " << firstRoute.toY << ",\n";
    out << "  \"error\": " << jsonQuoted (error) << ",\n";
    out << "  \"rowTexts\": ";
    appendJsonStringArray (out, rowTexts);
    out << ",\n";
    out << "  \"rows\": ";
    appendRowsJson (out, surface);
    out << ",\n";
    out << "  \"nodeSurfaces\": ";
    appendNodeSurfacesJson (out, surface);
    out << ",\n";
    out << "  \"edgeRoutes\": ";
    appendEdgeRoutesJson (out, surface);
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

const char* workbenchCanvasSurfaceProofDisplayName()
{
    return displayName;
}

const char* workbenchCanvasSurfaceProofDirectoryName()
{
    return directoryName;
}

WorkbenchCanvasSurfaceProofRunResult runWorkbenchCanvasSurfaceProof (
    const WorkbenchCanvasSurfaceProofRunRequest& request)
{
    WorkbenchCanvasSurfaceProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };

    const auto fail = [&] (const std::string& message)
    {
        result.ok = false;
        result.status = "failed";
        result.statusText = "workbench canvas surface failed: " + message;
        result.error = message;
        return result;
    };

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    const auto createRequest = makeCanvasSurfaceProofProjectRequest (request.outputDirectory);
    std::error_code removeError;
    std::filesystem::remove_all (createRequest.projectDirectory, removeError);
    if (removeError)
        return fail ("could not reset workbench canvas surface proof directory: " + removeError.message());

    const auto created = createActiveWorkProject (createRequest);

    WorkbenchAppController controller;
    WorkbenchAppControllerOpenRequest openRequest;
    openRequest.activeWorkManifestPath = created.workManifestPath;
    openRequest.candidateRoots = request.candidateRoots;
    openRequest.saveStatus = "clean";
    openRequest.proofStatus = "canvas2-ready";
    openRequest.previewStatus = "preview-ready";
    const auto opened = controller.openCurrentSession (openRequest);
    const auto surface = makeWorkbenchCanvasSurface (controller.currentSession());
    result.statusText = makeCanvasSurfaceProofStatusText (created, opened, surface);

    if (const auto error = writeProofTextFile (
            result.reportPath,
            makeCanvasSurfaceReportJson (created, opened, surface, result.statusText));
        ! error.empty())
    {
        return fail (error);
    }

    result.ok = created.ok
                && opened.ok
                && opened.snapshot.ok
                && surface.ok
                && surface.rows.size() == 6
                && ! surface.nodeSurfaces.empty()
                && ! surface.edgeRoutes.empty();
    result.status = result.ok ? "dumped" : "failed";
    result.error = result.ok ? std::string {} : result.statusText;
    return result;
}
}
