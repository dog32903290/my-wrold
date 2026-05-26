#include "WorkbenchSessionOpenStatus.h"

#include "GraphIOMappingStorage.h"
#include "ProofRunSupport.h"
#include "WorkProjectResolver.h"

#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* workFixturePath = "fixtures/storage/c2-compound-work/myworld.work.json";
constexpr const char* mappingFixturePath = "fixtures/graphs/g1_loudness_to_shader_uniform.graph.json";

bool explicitFileExists (const std::filesystem::path& path)
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

    WorkProjectResolveRequest workRequest;
    workRequest.activeWorkManifestPath = request.activeWorkManifestPath;
    workRequest.candidateRoots = request.candidateRoots;
    workRequest.fallbackWorkManifestPath = workFixturePath;

    const auto resolvedWork = resolveWorkProjectForWorkbench (workRequest);
    if (! resolvedWork.ok)
    {
        result.status = "failed";
        result.error = resolvedWork.error;
        result.snapshot.ok = false;
        result.snapshot.status = "blocked";
        result.snapshot.message = result.error;
        result.snapshot.workManifestPath = resolvedWork.workManifestPath;
        result.snapshot.workSource = resolvedWork.lifecycle.workSource;
        result.snapshot.activeWorkManifestPath = resolvedWork.activeWorkManifestPath;
        result.snapshot.workSourceStatus = resolvedWork.lifecycle.workSourceStatus;
        result.snapshot.workDiagnostics = resolvedWork.workDiagnostics;
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
        result.snapshot.workManifestPath = resolvedWork.workManifestPath;
        result.snapshot.workSource = resolvedWork.lifecycle.workSource;
        result.snapshot.workSourceStatus = resolvedWork.lifecycle.workSourceStatus;
        result.snapshot.activeWorkManifestPath = resolvedWork.activeWorkManifestPath;
        result.snapshot.workDiagnostics = resolvedWork.workDiagnostics;
        return result;
    }

    auto document = resolvedWork.document;
    document.outputView.followedNodeId = "out1";

    WorkbenchSessionRequest sessionRequest;
    sessionRequest.workManifestPath = resolvedWork.workManifestPath;
    sessionRequest.workSource = resolvedWork.lifecycle.workSource;
    sessionRequest.workSourceStatus = resolvedWork.lifecycle.workSourceStatus;
    sessionRequest.activeWorkManifestPath = resolvedWork.activeWorkManifestPath;
    sessionRequest.graphIOMappingSourcePath = mappingSourcePath;
    sessionRequest.document = document;
    sessionRequest.graphIOMappings = loadedMappings.mappings;
    sessionRequest.dirty = request.dirty;
    sessionRequest.saveStatus = request.saveStatus;
    sessionRequest.proofStatus = request.proofStatus;
    sessionRequest.previewStatus = request.previewStatus;
    sessionRequest.workDiagnostics = resolvedWork.workDiagnostics;

    result.snapshot = makeWorkbenchSessionSnapshot (sessionRequest);
    result.ok = result.snapshot.ok;
    result.status = result.ok ? "ready" : "blocked";
    result.error = result.ok ? std::string {} : result.snapshot.message;
    return result;
}

ExplicitWorkbenchOpenResult openExplicitWorkbenchSession (
    const ExplicitWorkbenchOpenRequest& request)
{
    ExplicitWorkbenchOpenResult result;

    if (request.workManifestPath.empty())
    {
        result.status = "validation-failed";
        result.error = "work manifest path is required";
        result.statusText = "explicit open failed: " + result.error;
        result.snapshot.ok = false;
        result.snapshot.status = "blocked";
        result.snapshot.message = result.error;
        result.snapshot.workSource = "explicit-work";
        result.snapshot.workSourceStatus = "explicit-work-blocked";
        return result;
    }

    if (! explicitFileExists (request.workManifestPath))
    {
        result.status = "explicit-open-blocked";
        result.error = "explicit work manifest does not exist: " + request.workManifestPath.string();
        result.statusText = "explicit open failed: " + result.error;
        result.snapshot.ok = false;
        result.snapshot.status = "blocked";
        result.snapshot.message = result.error;
        result.snapshot.workManifestPath = request.workManifestPath.string();
        result.snapshot.workSource = "explicit-work";
        result.snapshot.workSourceStatus = "explicit-work-blocked";
        return result;
    }

    WorkbenchSessionOpenStatusRequest currentRequest;
    currentRequest.activeWorkManifestPath = request.workManifestPath;
    currentRequest.candidateRoots = request.candidateRoots;
    currentRequest.dirty = request.dirty;
    currentRequest.saveStatus = request.saveStatus;
    currentRequest.proofStatus = request.proofStatus;
    currentRequest.previewStatus = request.previewStatus;

    const auto opened = openCurrentWorkbenchSession (currentRequest);
    result.ok = opened.ok;
    result.status = opened.ok ? "explicit-opened" : "explicit-open-blocked";
    result.error = opened.error;
    result.snapshot = opened.snapshot;
    result.snapshot.workSource = "explicit-work";
    result.snapshot.workSourceStatus = opened.ok ? "explicit-work-opened" : "explicit-work-blocked";
    result.snapshot.activeWorkManifestPath.clear();

    result.statusText = opened.ok
                            ? "explicit open ready: "
                                  + result.snapshot.documentId
                                  + " source "
                                  + result.snapshot.workSourceStatus
                                  + " mappings "
                                  + std::to_string (result.snapshot.validGraphIOMappingCount)
                                  + "/"
                                  + std::to_string (result.snapshot.graphIOMappingCount)
                            : "explicit open failed: " + result.error;

    return result;
}

std::string makeWorkbenchSessionStatusText (const WorkbenchSessionSnapshot& snapshot)
{
    if (! snapshot.ok)
        return "workbench blocked source "
               + snapshot.workSourceStatus
               + ": "
               + snapshot.message;

    return "workbench "
           + snapshot.status
           + " "
           + snapshot.documentId
           + " source "
           + snapshot.workSourceStatus
           + " mappings "
           + std::to_string (snapshot.validGraphIOMappingCount)
           + "/"
           + std::to_string (snapshot.graphIOMappingCount);
}
}
