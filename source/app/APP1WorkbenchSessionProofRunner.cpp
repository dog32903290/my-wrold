#include "APP1WorkbenchSessionProofRunner.h"

#include "GraphIOMappingStorage.h"
#include "ProofRunSupport.h"
#include "StorageContract.h"
#include "WorkbenchSession.h"

namespace myworld
{
namespace
{
constexpr const char* displayName = "APP1 workbench session";
constexpr const char* directoryName = "app1-workbench-session-proof";
constexpr const char* reportFileName = "workbench_session_report.json";
constexpr const char* workFixturePath = "fixtures/storage/c2-compound-work/myworld.work.json";
constexpr const char* mappingFixturePath = "fixtures/graphs/g1_loudness_to_shader_uniform.graph.json";

APP1WorkbenchSessionProofRunResult makeInitialResult (const APP1WorkbenchSessionProofRunRequest& request)
{
    APP1WorkbenchSessionProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };
    return result;
}
}

const char* app1WorkbenchSessionProofDisplayName()
{
    return displayName;
}

const char* app1WorkbenchSessionProofDirectoryName()
{
    return directoryName;
}

APP1WorkbenchSessionProofRunResult runAPP1WorkbenchSessionProof (
    const APP1WorkbenchSessionProofRunRequest& request)
{
    auto result = makeInitialResult (request);

    const auto fail = [&] (const std::string& message)
    {
        result.ok = false;
        result.status = "failed";
        result.error = message;
        return result;
    };

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    PatchDocumentLoadResult loadedPatch;
    std::string workManifestPath;
    std::string lastError;
    for (const auto& candidate : proofCandidatePaths (request.candidateRoots, workFixturePath))
    {
        const auto loaded = loadMainPatchDocumentForWork (candidate.string());
        if (loaded.ok)
        {
            loadedPatch = loaded;
            workManifestPath = candidate.string();
            break;
        }

        lastError = loaded.error;
    }

    if (! loadedPatch.ok)
        return fail (lastError.empty() ? "could not load APP1 work fixture" : lastError);

    GraphIOMappingLoadResult loadedMappings;
    for (const auto& candidate : proofCandidatePaths (request.candidateRoots, mappingFixturePath))
    {
        loadedMappings = loadGraphIOMappingsFromFile (candidate.string());
        if (loadedMappings.ok)
            break;

        lastError = loadedMappings.error;
    }

    if (! loadedMappings.ok)
        return fail (lastError.empty() ? "could not load APP1 graph IO mapping fixture" : lastError);

    loadedPatch.document.outputView.followedNodeId = "out1";

    WorkbenchSessionRequest sessionRequest;
    sessionRequest.workManifestPath = workManifestPath;
    sessionRequest.document = loadedPatch.document;
    sessionRequest.graphIOMappings = loadedMappings.mappings;
    sessionRequest.dirty = request.dirty;
    sessionRequest.saveStatus = request.saveStatus;
    sessionRequest.proofStatus = request.proofStatus;
    sessionRequest.previewStatus = request.previewStatus;

    const auto snapshot = makeWorkbenchSessionSnapshot (sessionRequest);
    if (const auto error = writeProofTextFile (result.reportPath, makeWorkbenchSessionReportJson (snapshot));
        ! error.empty())
    {
        return fail (error);
    }

    result.ok = snapshot.ok;
    result.status = snapshot.ok ? "dumped" : "failed";
    result.error = snapshot.ok ? std::string {} : snapshot.message;
    return result;
}
}
