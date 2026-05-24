#include "ActiveWorkService.h"

#include "AppPaths.h"
#include "StorageContract.h"

#include <cctype>

namespace myworld
{
namespace
{
bool copyTextFile (const juce::File& source, const juce::File& target)
{
    if (! target.getParentDirectory().createDirectory())
        return false;

    if (target.existsAsFile() && ! target.deleteFile())
        return false;

    return source.copyFileTo (target);
}

bool copyFirstRepoCandidate (const juce::String& relativePath, const juce::File& target, std::string& error)
{
    for (const auto& candidate : repoCandidatePaths (relativePath))
    {
        const auto source = juce::File (candidate);
        if (copyTextFile (source, target))
            return true;

        error = "could not copy " + relativePath.toStdString() + " from " + candidate;
    }

    if (error.empty())
        error = "could not copy " + relativePath.toStdString();

    return false;
}

bool prepareDefaultActiveWorkProject (const juce::File& workManifestFile, std::string& error)
{
    const auto patchFile = workManifestFile.getParentDirectory().getChildFile ("patches").getChildFile ("main.patch.json");
    const auto debugDirectory = workManifestFile.getParentDirectory().getParentDirectory();
    const auto moduleLibraryFile = debugDirectory.getChildFile ("module-libraries").getChildFile ("default.module-library.json");
    const auto moduleManifestFile = debugDirectory.getChildFile ("module-libraries")
                                          .getChildFile ("modules")
                                          .getChildFile ("loudness")
                                          .getChildFile ("module.json");

    if (! workManifestFile.existsAsFile()
        && ! copyFirstRepoCandidate ("fixtures/storage/c2-compound-work/myworld.work.json", workManifestFile, error))
    {
        return false;
    }

    if (! patchFile.existsAsFile()
        && ! copyFirstRepoCandidate ("fixtures/storage/c2-compound-work/patches/main.patch.json", patchFile, error))
    {
        return false;
    }

    if (! moduleLibraryFile.existsAsFile()
        && ! copyFirstRepoCandidate ("fixtures/module-libraries/default.module-library.json", moduleLibraryFile, error))
    {
        return false;
    }

    if (! moduleManifestFile.existsAsFile()
        && ! copyFirstRepoCandidate ("fixtures/modules/loudness/module.json", moduleManifestFile, error))
    {
        return false;
    }

    return true;
}

std::string safeIdentifier (const std::string& text)
{
    std::string result;
    result.reserve (text.size());

    for (const auto character : text)
    {
        const auto byte = static_cast<unsigned char> (character);
        if (std::isalnum (byte) != 0 || character == '-' || character == '_')
            result.push_back (static_cast<char> (std::tolower (byte)));
        else
            result.push_back ('-');
    }

    return result.empty() ? "module" : result;
}
}

CommandResult saveActiveWorkProject (GraphSession& session)
{
    const auto manifestFile = activeWorkManifestFile();
    std::string error;

    if (manifestFile == defaultActiveWorkManifestFile()
        && ! prepareDefaultActiveWorkProject (manifestFile, error))
    {
        return { false, error };
    }

    const auto result = saveWork (session, manifestFile.getFullPathName().toStdString());
    return { result.ok, result.ok ? result.status : result.error };
}

PublishModuleResult publishSelectedModuleFromActiveWork (GraphSession& session, const std::string& sourceNodeId)
{
    const auto manifestFile = activeWorkManifestFile();
    std::string error;
    std::string workManifestPath;

    if (manifestFile == defaultActiveWorkManifestFile())
    {
        for (const auto& candidate : repoCandidatePaths ("fixtures/storage/c2-compound-work/myworld.work.json"))
        {
            const auto work = loadWorkProjectManifest (candidate);
            if (work.ok)
            {
                workManifestPath = candidate;
                break;
            }

            error = work.error;
        }
    }
    else
    {
        workManifestPath = manifestFile.getFullPathName().toStdString();
    }

    if (workManifestPath.empty())
    {
        return { false,
                 "publish_module",
                 "validation-failed",
                 sourceNodeId,
                 {},
                 {},
                 {},
                 {},
                 {},
                 {},
                 error.empty() ? "could not resolve publish source work manifest" : error };
    }

    const auto safeNodeId = safeIdentifier (sourceNodeId);
    const auto publishDirectory = c5VisibleModulePublishDirectory();

    PublishModuleRequest request;
    request.workManifestPath = workManifestPath;
    request.sourceNodeId = sourceNodeId;
    request.moduleId = "module.visible-" + safeNodeId;
    request.moduleTitle = "Visible " + sourceNodeId;
    request.nodeType = "compound.visible-" + safeNodeId;
    request.packageDirectory = publishDirectory.getChildFile ("modules")
                                   .getChildFile (safeNodeId)
                                   .getFullPathName()
                                   .toStdString();
    request.targetLibraryPath = publishDirectory.getChildFile ("module-libraries")
                                    .getChildFile ("visible.module-library.json")
                                    .getFullPathName()
                                    .toStdString();
    request.overwriteExisting = true;

    return publishModule (session, request);
}
}
