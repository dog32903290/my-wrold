#include "WorkProjectResolver.h"

#include "ProofRunSupport.h"

#include <filesystem>
#include <system_error>

namespace myworld
{
namespace
{
bool fileExists (const std::filesystem::path& path)
{
    std::error_code error;
    return std::filesystem::is_regular_file (path, error);
}

void setLifecycle (WorkProjectResolveResult& result, WorkProjectLifecycleStatus status)
{
    result.lifecycle = makeWorkProjectLifecycle (status);
}

std::string noneIfEmpty (const std::string& text)
{
    return text.empty() ? "none" : text;
}

void refreshWorkDiagnostics (WorkProjectResolveResult& result)
{
    result.workDiagnostics = {
        "workSource=" + result.lifecycle.workSource,
        "workSourceStatus=" + result.lifecycle.workSourceStatus,
        "activeWorkManifestPath=" + noneIfEmpty (result.activeWorkManifestPath),
        "workManifestPath=" + noneIfEmpty (result.workManifestPath)
    };

    if (! result.error.empty())
        result.workDiagnostics.push_back ("error=" + result.error);
}
}

WorkProjectResolveResult resolveWorkProjectForWorkbench (const WorkProjectResolveRequest& request)
{
    WorkProjectResolveResult result;
    result.activeWorkManifestPath = request.activeWorkManifestPath.string();

    if (! request.activeWorkManifestPath.empty() && fileExists (request.activeWorkManifestPath))
    {
        const auto loaded = loadMainPatchDocumentForWork (request.activeWorkManifestPath.string());
        result.workManifestPath = request.activeWorkManifestPath.string();

        if (! loaded.ok)
        {
            setLifecycle (result, WorkProjectLifecycleStatus::activeWorkBlocked);
            result.error = loaded.error;
            refreshWorkDiagnostics (result);
            return result;
        }

        setLifecycle (result, WorkProjectLifecycleStatus::activeWorkOpened);
        result.ok = true;
        result.document = loaded.document;
        refreshWorkDiagnostics (result);
        return result;
    }

    std::string lastError;
    for (const auto& candidate : proofCandidatePaths (request.candidateRoots, request.fallbackWorkManifestPath))
    {
        const auto loaded = loadMainPatchDocumentForWork (candidate.string());
        if (loaded.ok)
        {
            setLifecycle (result,
                          request.activeWorkManifestPath.empty()
                              ? WorkProjectLifecycleStatus::fixtureFallbackNoActiveRequest
                              : WorkProjectLifecycleStatus::fixtureFallbackActiveMissing);
            result.ok = true;
            result.document = loaded.document;
            result.workManifestPath = candidate.string();
            refreshWorkDiagnostics (result);
            return result;
        }

        lastError = loaded.error;
    }

    setLifecycle (result,
                  request.activeWorkManifestPath.empty()
                      ? WorkProjectLifecycleStatus::fixtureBlockedNoActiveRequest
                      : WorkProjectLifecycleStatus::fixtureBlockedActiveMissing);
    result.error = lastError.empty() ? "could not open current workbench session work" : lastError;
    refreshWorkDiagnostics (result);
    return result;
}
}
