#include "WorkbenchRunProofRunner.h"

#include "GraphContract.h"
#include "JsonWriter.h"
#include "OutputViewState.h"
#include "ProofRunSupport.h"
#include "StorageContract.h"
#include "WorkbenchAppController.h"
#include "WorkbenchRunSurface.h"

#include <filesystem>
#include <sstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* displayName = "workbench run";
constexpr const char* directoryName = "workbench-run-proof";
constexpr const char* reportFileName = "run_report.json";

struct RunProofProjectResult
{
    bool ok = false;
    std::string status;
    std::string workManifestPath;
    std::string patchPath;
    std::string error;
    std::vector<std::string> diagnostics;
};

std::string noneIfEmpty (const std::string& text)
{
    return text.empty() ? "none" : text;
}

void refreshRunProofProjectDiagnostics (RunProofProjectResult& result)
{
    result.diagnostics = {
        "runProofProjectOk=" + std::string (result.ok ? "true" : "false"),
        "runProofProjectStatus=" + noneIfEmpty (result.status),
        "runProofProjectManifestPath=" + noneIfEmpty (result.workManifestPath),
        "runProofProjectPatchPath=" + noneIfEmpty (result.patchPath)
    };

    if (! result.error.empty())
        result.diagnostics.push_back ("runProofProjectError=" + result.error);
}

GraphContract makeRunProofGraph()
{
    GraphNode constant;
    constant.id = "const1";
    constant.type = "image.constant";
    constant.position = { 80.0, 80.0 };
    constant.params = {
        { "color", "[0.02, 0.02, 0.02, 1.0]" },
        { "resolution", "[1280, 720]" }
    };

    GraphNode output;
    output.id = "out1";
    output.type = "output.texture_summary";
    output.position = { 320.0, 80.0 };

    GraphEdge edge;
    edge.id = "edge.const1.out.out1.input";
    edge.from = "const1.out";
    edge.to = "out1.input";
    edge.dataType = "texture.rgba";
    edge.streamKind = "continuous";

    return { 1, { { constant, output }, { edge } }, { { constant, output }, { edge } } };
}

RunProofProjectResult createRunProofWork (const std::filesystem::path& outputDirectory)
{
    RunProofProjectResult result;
    const auto projectDirectory = outputDirectory / "run-project";
    const auto manifestPath = projectDirectory / "myworld.work.json";
    const auto patchPath = projectDirectory / "patches" / "main.patch.json";
    result.workManifestPath = manifestPath.string();
    result.patchPath = patchPath.string();

    if (outputDirectory.empty())
    {
        result.status = "validation-failed";
        result.error = "output directory is required";
        refreshRunProofProjectDiagnostics (result);
        return result;
    }

    const auto manifest = makeMinimalWorkProject ("work.run2-proof", "RUN2 Workbench Run Proof Work");
    if (const auto error = writeProofTextFile (manifestPath, toJson (manifest)); ! error.empty())
    {
        result.status = "write-failed";
        result.error = error;
        refreshRunProofProjectDiagnostics (result);
        return result;
    }

    OutputViewState outputView;
    outputView.followedNodeId = "out1";
    const auto patch = makePatchDocument ("patch.run2-main",
                                          "RUN2 Main Patch",
                                          makeRunProofGraph(),
                                          outputView);
    const auto saved = savePatchDocument (patchPath.string(), patch);
    if (! saved.ok)
    {
        result.status = saved.status;
        result.error = saved.error;
        refreshRunProofProjectDiagnostics (result);
        return result;
    }

    const auto loadedWork = loadWorkProjectManifest (manifestPath.string());
    if (! loadedWork.ok)
    {
        result.status = "validation-failed";
        result.error = loadedWork.error;
        refreshRunProofProjectDiagnostics (result);
        return result;
    }

    const auto loadedPatch = loadMainPatchDocumentForWork (manifestPath.string());
    if (! loadedPatch.ok)
    {
        result.status = "validation-failed";
        result.error = loadedPatch.error;
        refreshRunProofProjectDiagnostics (result);
        return result;
    }

    result.ok = true;
    result.status = "created";
    refreshRunProofProjectDiagnostics (result);
    return result;
}

std::vector<std::string> rowTextsFor (const WorkbenchRunSurface& surface)
{
    std::vector<std::string> texts;
    texts.reserve (surface.rows.size());

    for (const auto& row : surface.rows)
        texts.push_back (row.text);

    return texts;
}

std::string rowValueFor (const WorkbenchRunSurface& surface,
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

std::string runOrderTextFor (const std::vector<std::string>& runOrder)
{
    if (runOrder.empty())
        return "none";

    std::string text;
    for (const auto& id : runOrder)
    {
        if (! text.empty())
            text += " -> ";

        text += id;
    }

    return text;
}

bool regularFileExists (const std::string& path)
{
    if (path.empty())
        return false;

    std::error_code error;
    return std::filesystem::is_regular_file (path, error);
}

std::vector<std::filesystem::path> artifactPathsFor (const std::filesystem::path& reportPath,
                                                     const WorkbenchHeadlessRunResult& run)
{
    std::vector<std::filesystem::path> paths { reportPath };
    paths.insert (paths.end(), run.artifactPaths.begin(), run.artifactPaths.end());
    return paths;
}

std::vector<std::string> artifactPathTexts (const std::vector<std::filesystem::path>& paths)
{
    std::vector<std::string> texts;
    texts.reserve (paths.size());

    for (const auto& path : paths)
        texts.push_back (path.string());

    return texts;
}

std::string makeRunProofStatusText (const RunProofProjectResult& created,
                                    const WorkbenchAppControllerOpenResult& opened,
                                    const WorkbenchHeadlessRunResult& run)
{
    if (! created.ok)
        return "workbench run failed: " + created.error;

    if (! opened.ok)
        return "workbench run failed: " + opened.error;

    if (! run.ok)
        return run.statusText.empty() ? "workbench run failed: headless run blocked"
                                      : run.statusText;

    return run.statusText;
}

void appendRowsJson (std::ostream& out, const WorkbenchRunSurface& surface)
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

bool runProofOk (const RunProofProjectResult& created,
                 const WorkbenchAppControllerOpenResult& opened,
                 const WorkbenchHeadlessRunResult& run)
{
    const auto readiness = rowValueFor (run.surface, "readiness", "unknown");
    const auto executionStatus = rowValueFor (run.surface, "execution", "unknown");

    return created.ok
           && opened.ok
           && opened.snapshot.ok
           && run.ok
           && run.surface.ok
           && run.surface.rows.size() == 6
           && opened.snapshot.runtimeNodeCount == 2
           && opened.snapshot.runtimeEdgeCount == 1
           && opened.snapshot.activeOutputNodeId == "out1"
           && ! run.surface.runOrder.empty()
           && readiness == "ready"
           && executionStatus == "ran"
           && regularFileExists (run.headless.textureSummaryPath)
           && regularFileExists (run.headless.cookOrderPath)
           && regularFileExists (run.headless.nodeStatsPath)
           && regularFileExists (run.headless.thumbnailPath)
           && regularFileExists (run.headless.thumbnailStatsPath)
           && regularFileExists (run.headless.errorsPath);
}

std::string makeRunReportJson (const RunProofProjectResult& created,
                               const WorkbenchAppControllerOpenResult& opened,
                               const WorkbenchHeadlessRunResult& run,
                               const std::string& statusText,
                               const std::vector<std::filesystem::path>& artifactPaths)
{
    const auto& snapshot = opened.snapshot;
    const auto runOrderText = runOrderTextFor (run.surface.runOrder);
    const auto targetNodeId = rowValueFor (run.surface, "target", "none");
    const auto readiness = rowValueFor (run.surface, "readiness", "unknown");
    const auto executionStatus = rowValueFor (run.surface, "execution", "unknown");
    const auto ok = runProofOk (created, opened, run);
    const auto error = ok ? std::string {} : statusText;
    const auto rowTexts = rowTextsFor (run.surface);
    const auto artifactTexts = artifactPathTexts (artifactPaths);

    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"workbenchRunReport\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"statusText\": " << jsonQuoted (statusText) << ",\n";
    out << "  \"headline\": " << jsonQuoted (run.surface.headline) << ",\n";
    out << "  \"rowCount\": " << run.surface.rows.size() << ",\n";
    out << "  \"creationStatus\": " << jsonQuoted (created.status) << ",\n";
    out << "  \"openStatus\": " << jsonQuoted (opened.status) << ",\n";
    out << "  \"runStatus\": " << jsonQuoted (run.status) << ",\n";
    out << "  \"workManifestPath\": " << jsonQuoted (snapshot.workManifestPath) << ",\n";
    out << "  \"patchPath\": " << jsonQuoted (created.patchPath) << ",\n";
    out << "  \"documentId\": " << jsonQuoted (snapshot.documentId) << ",\n";
    out << "  \"documentTitle\": " << jsonQuoted (snapshot.documentTitle) << ",\n";
    out << "  \"workSource\": " << jsonQuoted (snapshot.workSource) << ",\n";
    out << "  \"workSourceStatus\": " << jsonQuoted (snapshot.workSourceStatus) << ",\n";
    out << "  \"runtimeNodeCount\": " << snapshot.runtimeNodeCount << ",\n";
    out << "  \"runtimeEdgeCount\": " << snapshot.runtimeEdgeCount << ",\n";
    out << "  \"activeOutputNodeId\": " << jsonQuoted (snapshot.activeOutputNodeId) << ",\n";
    out << "  \"runOrder\": ";
    appendJsonStringArray (out, run.surface.runOrder);
    out << ",\n";
    out << "  \"runOrderText\": " << jsonQuoted (runOrderText) << ",\n";
    out << "  \"targetNodeId\": " << jsonQuoted (targetNodeId) << ",\n";
    out << "  \"readiness\": " << jsonQuoted (readiness) << ",\n";
    out << "  \"executionStatus\": " << jsonQuoted (executionStatus) << ",\n";
    out << "  \"headlessTextureSummaryPath\": " << jsonQuoted (run.headless.textureSummaryPath) << ",\n";
    out << "  \"headlessCookOrderPath\": " << jsonQuoted (run.headless.cookOrderPath) << ",\n";
    out << "  \"headlessNodeStatsPath\": " << jsonQuoted (run.headless.nodeStatsPath) << ",\n";
    out << "  \"headlessThumbnailPath\": " << jsonQuoted (run.headless.thumbnailPath) << ",\n";
    out << "  \"headlessThumbnailStatsPath\": " << jsonQuoted (run.headless.thumbnailStatsPath) << ",\n";
    out << "  \"headlessErrorsPath\": " << jsonQuoted (run.headless.errorsPath) << ",\n";
    out << "  \"error\": " << jsonQuoted (error) << ",\n";
    out << "  \"rowTexts\": ";
    appendJsonStringArray (out, rowTexts);
    out << ",\n";
    out << "  \"rows\": ";
    appendRowsJson (out, run.surface);
    out << ",\n";
    out << "  \"creationDiagnostics\": ";
    appendJsonStringArray (out, created.diagnostics);
    out << ",\n";
    out << "  \"artifactPaths\": ";
    appendJsonStringArray (out, artifactTexts);
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

const char* workbenchRunProofDisplayName()
{
    return displayName;
}

const char* workbenchRunProofDirectoryName()
{
    return directoryName;
}

WorkbenchRunProofRunResult runWorkbenchRunProof (const WorkbenchRunProofRunRequest& request)
{
    WorkbenchRunProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;

    const auto fail = [&] (const std::string& message)
    {
        result.ok = false;
        result.status = "failed";
        result.statusText = "workbench run failed: " + message;
        result.error = message;
        return result;
    };

    if (request.outputDirectory.empty())
        return fail ("output directory is required");

    if (const auto error = clearProofDirectoryIfExists (request.outputDirectory); ! error.empty())
        return fail (error);

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    const auto created = createRunProofWork (request.outputDirectory);

    WorkbenchAppController controller;
    WorkbenchAppControllerOpenRequest openRequest;
    openRequest.activeWorkManifestPath = created.workManifestPath;
    openRequest.candidateRoots = request.candidateRoots;
    openRequest.saveStatus = "clean";
    openRequest.proofStatus = "run2-ready";
    openRequest.previewStatus = "preview-ready";
    const auto opened = controller.openCurrentSession (openRequest);
    const auto run = runWorkbenchHeadlessRender (controller.currentSession(), request.outputDirectory);

    result.statusText = makeRunProofStatusText (created, opened, run);
    result.artifactPaths = artifactPathsFor (result.reportPath, run);

    if (const auto error = writeProofTextFile (
            result.reportPath,
            makeRunReportJson (created, opened, run, result.statusText, result.artifactPaths));
        ! error.empty())
    {
        return fail (error);
    }

    result.ok = runProofOk (created, opened, run);
    result.status = result.ok ? "dumped" : "failed";
    result.error = result.ok ? std::string {} : result.statusText;
    return result;
}
}
