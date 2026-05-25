#include "ActiveWorkService.h"

#include "AppPaths.h"
#include "StorageContract.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <system_error>

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

bool defaultActiveWorkProjectExists (const juce::File& workManifestFile)
{
    const auto patchFile = workManifestFile.getParentDirectory().getChildFile ("patches").getChildFile ("main.patch.json");
    const auto debugDirectory = workManifestFile.getParentDirectory().getParentDirectory();
    const auto moduleLibraryFile = debugDirectory.getChildFile ("module-libraries").getChildFile ("default.module-library.json");
    const auto moduleManifestFile = debugDirectory.getChildFile ("module-libraries")
                                          .getChildFile ("modules")
                                          .getChildFile ("loudness")
                                          .getChildFile ("module.json");

    return workManifestFile.existsAsFile()
           && patchFile.existsAsFile()
           && moduleLibraryFile.existsAsFile()
           && moduleManifestFile.existsAsFile();
}

std::string noneIfEmpty (const std::string& text)
{
    return text.empty() ? "none" : text;
}

void refreshPreparationDiagnostics (ActiveWorkPreparationResult& result)
{
    result.diagnostics = {
        "activeWorkPreparationOk=" + std::string (result.ok ? "true" : "false"),
        "activeWorkPreparationStatus=" + noneIfEmpty (result.status),
        "activeWorkPreparationManifestPath=" + noneIfEmpty (result.workManifestPath)
    };

    if (! result.error.empty())
        result.diagnostics.push_back ("activeWorkPreparationError=" + result.error);
}

bool regularFileExists (const std::filesystem::path& path)
{
    std::error_code error;
    return std::filesystem::is_regular_file (path, error);
}

bool writeTextFile (const std::filesystem::path& path, const std::string& text, std::string& error)
{
    std::error_code filesystemError;
    std::filesystem::create_directories (path.parent_path(), filesystemError);
    if (filesystemError)
    {
        error = "could not create directory: " + filesystemError.message();
        return false;
    }

    std::ofstream output (path, std::ios::trunc);
    if (! output)
    {
        error = "could not open file for writing: " + path.string();
        return false;
    }

    output << text;
    if (! output)
    {
        error = "could not write file: " + path.string();
        return false;
    }

    return true;
}

void refreshCreateProjectDiagnostics (CreateActiveWorkProjectResult& result)
{
    result.diagnostics = {
        "createActiveWorkProjectOk=" + std::string (result.ok ? "true" : "false"),
        "createActiveWorkProjectStatus=" + noneIfEmpty (result.status),
        "createActiveWorkProjectManifestPath=" + noneIfEmpty (result.workManifestPath),
        "createActiveWorkProjectPatchPath=" + noneIfEmpty (result.patchPath)
    };

    if (! result.error.empty())
        result.diagnostics.push_back ("createActiveWorkProjectError=" + result.error);
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

ActiveWorkPreparationResult prepareActiveWorkProjectForOpen()
{
    ActiveWorkPreparationResult result;
    const auto manifestFile = activeWorkManifestFile();
    result.workManifestPath = manifestFile.getFullPathName().toStdString();

    if (manifestFile != defaultActiveWorkManifestFile())
    {
        result.ok = true;
        result.status = "external-active-work-requested";
        refreshPreparationDiagnostics (result);
        return result;
    }

    const auto wasAlreadyPrepared = defaultActiveWorkProjectExists (manifestFile);
    std::string error;

    if (! prepareDefaultActiveWorkProject (manifestFile, error))
    {
        result.status = "default-active-work-blocked";
        result.error = error;
        refreshPreparationDiagnostics (result);
        return result;
    }

    result.ok = true;
    result.status = wasAlreadyPrepared ? "default-active-work-ready"
                                       : "default-active-work-prepared";
    refreshPreparationDiagnostics (result);
    return result;
}

CreateActiveWorkProjectResult createActiveWorkProject (const CreateActiveWorkProjectRequest& request)
{
    CreateActiveWorkProjectResult result;
    const auto manifestPath = request.projectDirectory / "myworld.work.json";
    const auto patchPath = request.projectDirectory / "patches" / "main.patch.json";
    result.workManifestPath = manifestPath.string();
    result.patchPath = patchPath.string();

    if (request.projectDirectory.empty()
        || request.workId.empty()
        || request.workTitle.empty()
        || request.patchId.empty()
        || request.patchTitle.empty())
    {
        result.status = "validation-failed";
        result.error = "project directory, work identity, and patch identity are required";
        refreshCreateProjectDiagnostics (result);
        return result;
    }

    if (! request.overwriteExisting
        && (regularFileExists (manifestPath) || regularFileExists (patchPath)))
    {
        result.status = "already-exists";
        result.error = "active work project files already exist";
        refreshCreateProjectDiagnostics (result);
        return result;
    }

    const auto work = makeMinimalWorkProject (request.workId, request.workTitle);
    const auto patch = makePatchDocument (request.patchId, request.patchTitle, makeDefaultShaderOutputGraph());

    std::string error;
    if (! writeTextFile (manifestPath, toJson (work), error))
    {
        result.status = "write-failed";
        result.error = error;
        refreshCreateProjectDiagnostics (result);
        return result;
    }

    const auto savedPatch = savePatchDocument (patchPath.string(), patch);
    if (! savedPatch.ok)
    {
        result.status = savedPatch.status;
        result.error = savedPatch.error;
        refreshCreateProjectDiagnostics (result);
        return result;
    }

    const auto loadedWork = loadWorkProjectManifest (manifestPath.string());
    if (! loadedWork.ok)
    {
        result.status = "validation-failed";
        result.error = loadedWork.error;
        refreshCreateProjectDiagnostics (result);
        return result;
    }

    const auto loadedPatch = loadMainPatchDocumentForWork (manifestPath.string());
    if (! loadedPatch.ok)
    {
        result.status = "validation-failed";
        result.error = loadedPatch.error;
        refreshCreateProjectDiagnostics (result);
        return result;
    }

    result.ok = true;
    result.status = "created";
    refreshCreateProjectDiagnostics (result);
    return result;
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
