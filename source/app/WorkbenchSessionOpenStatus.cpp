#include "WorkbenchSessionOpenStatus.h"

#include "GraphIOMappingStorage.h"
#include "ProofRunSupport.h"
#include "StorageContract.h"
#include "WorkProjectLifecycle.h"

#include <filesystem>

namespace myworld
{
namespace
{
constexpr const char* workFixturePath = "fixtures/storage/c2-compound-work/myworld.work.json";
constexpr const char* mappingFixturePath = "fixtures/graphs/g1_loudness_to_shader_uniform.graph.json";

struct LoadedWorkbenchInputs
{
    PatchDocument document;
    std::string workManifestPath;
    std::string workSource;
    std::string workSourceStatus;
};

bool fileExists (const std::filesystem::path& path)
{
    std::error_code error;
    return std::filesystem::is_regular_file (path, error);
}
}

WorkbenchSessionOpenStatusResult openCurrentWorkbenchSession (
    const WorkbenchSessionOpenStatusRequest& request)
{
    WorkbenchSessionOpenStatusResult result;
    std::string lastError;
    LoadedWorkbenchInputs loadedInputs;

    if (! request.activeWorkManifestPath.empty() && fileExists (request.activeWorkManifestPath))
    {
        const auto loaded = loadMainPatchDocumentForWork (request.activeWorkManifestPath.string());
        if (! loaded.ok)
        {
            const auto lifecycle = makeWorkProjectLifecycle (WorkProjectLifecycleStatus::activeWorkBlocked);

            result.status = "failed";
            result.error = loaded.error;
            result.snapshot.ok = false;
            result.snapshot.status = "blocked";
            result.snapshot.message = loaded.error;
            result.snapshot.workManifestPath = request.activeWorkManifestPath.string();
            result.snapshot.workSource = lifecycle.workSource;
            result.snapshot.workSourceStatus = lifecycle.workSourceStatus;
            result.snapshot.activeWorkManifestPath = request.activeWorkManifestPath.string();
            return result;
        }

        const auto lifecycle = makeWorkProjectLifecycle (WorkProjectLifecycleStatus::activeWorkOpened);
        loadedInputs.document = loaded.document;
        loadedInputs.workManifestPath = request.activeWorkManifestPath.string();
        loadedInputs.workSource = lifecycle.workSource;
        loadedInputs.workSourceStatus = lifecycle.workSourceStatus;
    }
    else
    {
        for (const auto& candidate : proofCandidatePaths (request.candidateRoots, workFixturePath))
        {
            const auto loaded = loadMainPatchDocumentForWork (candidate.string());
            if (loaded.ok)
            {
                const auto lifecycle = makeWorkProjectLifecycle (
                    request.activeWorkManifestPath.empty()
                        ? WorkProjectLifecycleStatus::fixtureFallbackNoActiveRequest
                        : WorkProjectLifecycleStatus::fixtureFallbackActiveMissing);

                loadedInputs.document = loaded.document;
                loadedInputs.workManifestPath = candidate.string();
                loadedInputs.workSource = lifecycle.workSource;
                loadedInputs.workSourceStatus = lifecycle.workSourceStatus;
                break;
            }

            lastError = loaded.error;
        }
    }

    if (loadedInputs.workManifestPath.empty())
    {
        const auto lifecycle = makeWorkProjectLifecycle (
            request.activeWorkManifestPath.empty() ? WorkProjectLifecycleStatus::fixtureBlockedNoActiveRequest
                                                   : WorkProjectLifecycleStatus::fixtureBlockedActiveMissing);

        result.status = "failed";
        result.error = lastError.empty() ? "could not open current workbench session work" : lastError;
        result.snapshot.ok = false;
        result.snapshot.status = "blocked";
        result.snapshot.message = result.error;
        result.snapshot.workSource = lifecycle.workSource;
        result.snapshot.activeWorkManifestPath = request.activeWorkManifestPath.string();
        result.snapshot.workSourceStatus = lifecycle.workSourceStatus;
        return result;
    }

    GraphIOMappingLoadResult loadedMappings;
    std::string mappingSourcePath;
    for (const auto& candidate : proofCandidatePaths (request.candidateRoots, mappingFixturePath))
    {
        loadedMappings = loadGraphIOMappingsFromFile (candidate.string());
        if (loadedMappings.ok)
        {
            mappingSourcePath = candidate.string();
            break;
        }

        lastError = loadedMappings.error;
    }

    if (! loadedMappings.ok)
    {
        result.status = "failed";
        result.error = lastError.empty() ? "could not load G1 graph IO mapping" : lastError;
        result.snapshot.ok = false;
        result.snapshot.status = "blocked";
        result.snapshot.message = result.error;
        result.snapshot.workManifestPath = loadedInputs.workManifestPath;
        result.snapshot.workSource = loadedInputs.workSource;
        result.snapshot.workSourceStatus = loadedInputs.workSourceStatus;
        result.snapshot.activeWorkManifestPath = request.activeWorkManifestPath.string();
        return result;
    }

    loadedInputs.document.outputView.followedNodeId = "out1";

    WorkbenchSessionRequest sessionRequest;
    sessionRequest.workManifestPath = loadedInputs.workManifestPath;
    sessionRequest.workSource = loadedInputs.workSource;
    sessionRequest.workSourceStatus = loadedInputs.workSourceStatus;
    sessionRequest.activeWorkManifestPath = request.activeWorkManifestPath.string();
    sessionRequest.graphIOMappingSourcePath = mappingSourcePath;
    sessionRequest.document = loadedInputs.document;
    sessionRequest.graphIOMappings = loadedMappings.mappings;
    sessionRequest.dirty = request.dirty;
    sessionRequest.saveStatus = request.saveStatus;
    sessionRequest.proofStatus = request.proofStatus;
    sessionRequest.previewStatus = request.previewStatus;

    result.snapshot = makeWorkbenchSessionSnapshot (sessionRequest);
    result.ok = result.snapshot.ok;
    result.status = result.ok ? "ready" : "blocked";
    result.error = result.ok ? std::string {} : result.snapshot.message;
    return result;
}

std::string makeWorkbenchSessionStatusText (const WorkbenchSessionSnapshot& snapshot)
{
    if (! snapshot.ok)
        return "workbench blocked: " + snapshot.message;

    return "workbench "
           + snapshot.status
           + " "
           + snapshot.documentId
           + " mappings "
           + std::to_string (snapshot.validGraphIOMappingCount)
           + "/"
           + std::to_string (snapshot.graphIOMappingCount);
}
}
