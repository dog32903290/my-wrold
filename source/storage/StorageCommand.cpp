#include "StorageCommand.h"

#include "InteractionContract.h"
#include "JsonWriter.h"
#include "StorageContract.h"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace myworld
{
namespace
{
std::filesystem::path resolveNearManifest (const std::string& workManifestPath,
                                           const std::string& relativeOrAbsolutePath)
{
    const std::filesystem::path targetPath (relativeOrAbsolutePath);
    if (targetPath.is_absolute())
        return targetPath;

    return std::filesystem::path (workManifestPath).parent_path() / targetPath;
}

std::filesystem::path saveLogPathForWork (const std::string& workManifestPath)
{
    return std::filesystem::path (workManifestPath).parent_path() / ".myworld" / "save_log.jsonl";
}

std::string localTimestamp()
{
    const auto now = std::time (nullptr);
    std::tm local {};

#if defined(_WIN32)
    localtime_s (&local, &now);
#else
    localtime_r (&now, &local);
#endif

    std::ostringstream out;
    out << std::put_time (&local, "%Y-%m-%dT%H:%M:%S%z");
    return out.str();
}

bool appendSaveLog (const std::filesystem::path& saveLogPath,
                    const std::string& workManifestPath,
                    const std::string& patchPath,
                    const std::string& status,
                    const std::string& errorMessage,
                    std::string& writeError)
{
    std::error_code error;
    std::filesystem::create_directories (saveLogPath.parent_path(), error);
    if (error)
    {
        writeError = "could not create save log directory: " + error.message();
        return false;
    }

    std::ofstream output (saveLogPath, std::ios::app);
    if (! output)
    {
        writeError = "could not open save log: " + saveLogPath.string();
        return false;
    }

    output << "{ "
           << "\"timestamp\": " << jsonQuoted (localTimestamp()) << ", "
           << "\"command\": \"save_work\", "
           << "\"status\": " << jsonQuoted (status) << ", "
           << "\"workManifestPath\": " << jsonQuoted (workManifestPath) << ", "
           << "\"patchPath\": " << jsonQuoted (patchPath) << ", "
           << "\"commitStatus\": \"not-started\"";

    if (! errorMessage.empty())
        output << ", \"error\": " << jsonQuoted (errorMessage);

    output << " }\n";

    if (! output)
    {
        writeError = "could not write save log: " + saveLogPath.string();
        return false;
    }

    return true;
}

SaveWorkResult finishSaveWork (GraphSession& session,
                               bool ok,
                               const std::string& status,
                               const std::string& workManifestPath,
                               const std::string& patchPath,
                               const std::filesystem::path& saveLogPath,
                               const std::string& error)
{
    session.commandLog.push_back ("save_work:" + status);
    return { ok, status, workManifestPath, patchPath, saveLogPath.string(), error };
}
}

SaveWorkResult saveWork (GraphSession& session, const std::string& workManifestPath)
{
    if (workManifestPath.empty())
        return finishSaveWork (session, false, "validation-failed", workManifestPath, {}, {}, "work manifest path is required");

    const auto work = loadWorkProjectManifest (workManifestPath);
    const auto saveLogPath = saveLogPathForWork (workManifestPath);
    if (! work.ok)
        return finishSaveWork (session, false, "validation-failed", workManifestPath, {}, saveLogPath, work.error);

    const auto patchPath = resolveNearManifest (workManifestPath, work.manifest.mainPatchPath).string();

    if (! session.dirty)
    {
        std::string logError;
        if (! appendSaveLog (saveLogPath, workManifestPath, patchPath, "clean", {}, logError))
            return finishSaveWork (session, false, "write-failed", workManifestPath, patchPath, saveLogPath, logError);

        return finishSaveWork (session, true, "clean", workManifestPath, patchPath, saveLogPath, {});
    }

    const auto activePatch = loadPatchDocument (patchPath);
    if (! activePatch.ok)
        return finishSaveWork (session, false, "validation-failed", workManifestPath, patchPath, saveLogPath, activePatch.error);

    const auto document = makePatchDocument (activePatch.document.id, activePatch.document.title, session.graph);
    const auto saveResult = savePatchDocument (patchPath, document);
    if (! saveResult.ok)
    {
        std::string logError;
        appendSaveLog (saveLogPath, workManifestPath, patchPath, saveResult.status, saveResult.error, logError);
        return finishSaveWork (session, false, saveResult.status, workManifestPath, patchPath, saveLogPath, saveResult.error);
    }

    const auto reloadedPatch = loadPatchDocument (patchPath);
    if (! reloadedPatch.ok)
    {
        std::string logError;
        appendSaveLog (saveLogPath, workManifestPath, patchPath, "validation-failed", reloadedPatch.error, logError);
        return finishSaveWork (session, false, "validation-failed", workManifestPath, patchPath, saveLogPath, reloadedPatch.error);
    }

    std::string logError;
    if (! appendSaveLog (saveLogPath, workManifestPath, patchPath, saveResult.status, {}, logError))
        return finishSaveWork (session, false, "write-failed", workManifestPath, patchPath, saveLogPath, logError);

    session.dirty = false;
    return finishSaveWork (session, true, saveResult.status, workManifestPath, patchPath, saveLogPath, {});
}
}
