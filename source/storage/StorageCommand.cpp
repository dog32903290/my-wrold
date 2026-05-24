#include "StorageCommand.h"

#include "InteractionContract.h"
#include "JsonWriter.h"
#include "StorageContract.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <memory>
#include <sstream>
#include <utility>

#if ! defined(_WIN32)
#include <sys/wait.h>
#endif

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
                    const std::string& commitStatus,
                    const std::string& commitId,
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
           << "\"commitStatus\": " << jsonQuoted (commitStatus);

    if (! commitId.empty())
        output << ", \"commitId\": " << jsonQuoted (commitId);

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

std::string shellQuoted (const std::string& value)
{
    std::string quoted = "'";

    for (const auto character : value)
    {
        if (character == '\'')
            quoted += "'\\''";
        else
            quoted.push_back (character);
    }

    quoted += "'";
    return quoted;
}

std::string trimmed (std::string text)
{
    while (! text.empty() && std::isspace (static_cast<unsigned char> (text.back())) != 0)
        text.pop_back();

    auto first = text.begin();
    while (first != text.end() && std::isspace (static_cast<unsigned char> (*first)) != 0)
        ++first;

    text.erase (text.begin(), first);
    return text;
}

struct ShellResult
{
    int exitCode = 1;
    std::string output;
};

ShellResult runShellCommand (const std::string& command)
{
    auto* pipe = popen ((command + " 2>&1").c_str(), "r");
    if (pipe == nullptr)
        return { 1, "could not start command" };

    std::string output;
    char buffer[256];
    while (fgets (buffer, sizeof (buffer), pipe) != nullptr)
        output += buffer;

    const auto closeStatus = pclose (pipe);

#if defined(_WIN32)
    const auto exitCode = closeStatus;
#else
    const auto exitCode = WIFEXITED (closeStatus) ? WEXITSTATUS (closeStatus) : closeStatus;
#endif

    return { exitCode, output };
}

std::string jsonStringMember (const std::string& text, const std::string& key)
{
    const auto keyToken = "\"" + key + "\"";
    const auto keyPosition = text.find (keyToken);
    if (keyPosition == std::string::npos)
        return {};

    const auto colon = text.find (':', keyPosition + keyToken.size());
    if (colon == std::string::npos)
        return {};

    auto valuePosition = colon + 1;
    while (valuePosition < text.size() && std::isspace (static_cast<unsigned char> (text[valuePosition])) != 0)
        ++valuePosition;

    if (valuePosition >= text.size() || text[valuePosition] != '"')
        return {};

    std::string result;
    ++valuePosition;

    while (valuePosition < text.size())
    {
        const auto current = text[valuePosition++];

        if (current == '"')
            return result;

        if (current != '\\' || valuePosition >= text.size())
        {
            result.push_back (current);
            continue;
        }

        const auto escaped = text[valuePosition++];
        switch (escaped)
        {
            case '"':  result.push_back ('"'); break;
            case '\\': result.push_back ('\\'); break;
            case 'n':  result.push_back ('\n'); break;
            case 'r':  result.push_back ('\r'); break;
            case 't':  result.push_back ('\t'); break;
            default:   result.push_back (escaped); break;
        }
    }

    return {};
}

SaveLogEntry parseSaveLogEntry (const std::string& line)
{
    return { jsonStringMember (line, "timestamp"),
             jsonStringMember (line, "command"),
             jsonStringMember (line, "status"),
             jsonStringMember (line, "workManifestPath"),
             jsonStringMember (line, "patchPath"),
             jsonStringMember (line, "commitStatus"),
             jsonStringMember (line, "commitId"),
             jsonStringMember (line, "error") };
}

SaveWorkCommitResult commitSavedWorkInGit (const std::string& workManifestPath,
                                           const std::string& patchPath,
                                           const std::filesystem::path& saveLogPath,
                                           const std::string& commitMessage)
{
    const auto workRoot = std::filesystem::path (workManifestPath).parent_path();
    const auto patchRelative = std::filesystem::relative (std::filesystem::path (patchPath), workRoot).generic_string();
    const auto logRelative = std::filesystem::relative (saveLogPath, workRoot).generic_string();
    const auto gitPrefix = "git -C " + shellQuoted (workRoot.string()) + " ";

    const auto addResult = runShellCommand (gitPrefix + "add -- "
                                            + shellQuoted (patchRelative) + " "
                                            + shellQuoted (logRelative));
    if (addResult.exitCode != 0)
    {
        std::string logError;
        appendSaveLog (saveLogPath,
                       workManifestPath,
                       patchPath,
                       "save-ok commit-failed",
                       "save-ok commit-failed",
                       {},
                       addResult.output,
                       logError);
        return { false, "save-ok commit-failed", {}, addResult.output };
    }

    const auto diffResult = runShellCommand (gitPrefix + "diff --cached --quiet -- "
                                             + shellQuoted (patchRelative) + " "
                                             + shellQuoted (logRelative));
    if (diffResult.exitCode == 0)
        return { true, "clean", {}, {} };

    if (diffResult.exitCode != 1)
    {
        std::string logError;
        appendSaveLog (saveLogPath,
                       workManifestPath,
                       patchPath,
                       "save-ok commit-failed",
                       "save-ok commit-failed",
                       {},
                       diffResult.output,
                       logError);
        return { false, "save-ok commit-failed", {}, diffResult.output };
    }

    const auto commitResult = runShellCommand (gitPrefix + "commit -m "
                                               + shellQuoted (commitMessage) + " -- "
                                               + shellQuoted (patchRelative) + " "
                                               + shellQuoted (logRelative));
    if (commitResult.exitCode != 0)
    {
        std::string logError;
        appendSaveLog (saveLogPath,
                       workManifestPath,
                       patchPath,
                       "save-ok commit-failed",
                       "save-ok commit-failed",
                       {},
                       commitResult.output,
                       logError);
        return { false, "save-ok commit-failed", {}, commitResult.output };
    }

    const auto revParse = runShellCommand (gitPrefix + "rev-parse HEAD");
    const auto commitId = trimmed (revParse.output);
    if (revParse.exitCode != 0 || commitId.empty())
    {
        std::string logError;
        appendSaveLog (saveLogPath,
                       workManifestPath,
                       patchPath,
                       "save-ok commit-failed",
                       "save-ok commit-failed",
                       {},
                       revParse.output,
                       logError);
        return { false, "save-ok commit-failed", {}, revParse.output };
    }

    std::string logError;
    if (! appendSaveLog (saveLogPath,
                         workManifestPath,
                         patchPath,
                         "saved-and-committed",
                         "saved-and-committed",
                         commitId,
                         {},
                         logError))
    {
        return { false, "save-ok commit-failed", commitId, logError };
    }

    return { true, "saved-and-committed", commitId, {} };
}

SaveWorkResult finishSaveWork (GraphSession& session,
                               bool ok,
                               const std::string& status,
                               const std::string& commitStatus,
                               const std::string& workManifestPath,
                               const std::string& patchPath,
                               const std::filesystem::path& saveLogPath,
                               const std::string& error,
                               std::shared_ptr<SaveWorkCommitJob> commitJob = {})
{
    session.commandLog.push_back ("save_work:" + status);
    return { ok, status, commitStatus, workManifestPath, patchPath, saveLogPath.string(), error, std::move (commitJob) };
}
}

SaveWorkCommitJob::SaveWorkCommitJob (std::future<SaveWorkCommitResult> nextFuture)
    : future (nextFuture.share())
{
}

SaveWorkCommitResult SaveWorkCommitJob::wait() const
{
    return future.get();
}

SaveWorkResult saveWork (GraphSession& session, const std::string& workManifestPath)
{
    return saveWork (session, workManifestPath, {});
}

SaveWorkResult saveWork (GraphSession& session, const std::string& workManifestPath, const SaveWorkOptions& options)
{
    if (workManifestPath.empty())
        return finishSaveWork (session, false, "validation-failed", "not-started", workManifestPath, {}, {}, "work manifest path is required");

    const auto work = loadWorkProjectManifest (workManifestPath);
    const auto saveLogPath = saveLogPathForWork (workManifestPath);
    if (! work.ok)
        return finishSaveWork (session, false, "validation-failed", "not-started", workManifestPath, {}, saveLogPath, work.error);

    const auto patchPath = resolveNearManifest (workManifestPath, work.manifest.mainPatchPath).string();

    if (! session.dirty)
    {
        std::string logError;
        if (! appendSaveLog (saveLogPath, workManifestPath, patchPath, "clean", "not-started", {}, {}, logError))
            return finishSaveWork (session, false, "write-failed", "not-started", workManifestPath, patchPath, saveLogPath, logError);

        return finishSaveWork (session, true, "clean", "not-started", workManifestPath, patchPath, saveLogPath, {});
    }

    const auto activePatch = loadPatchDocument (patchPath);
    if (! activePatch.ok)
        return finishSaveWork (session, false, "validation-failed", "not-started", workManifestPath, patchPath, saveLogPath, activePatch.error);

    const auto document = makePatchDocument (activePatch.document.id, activePatch.document.title, session.graph);
    const auto saveResult = savePatchDocument (patchPath, document);
    if (! saveResult.ok)
    {
        std::string logError;
        appendSaveLog (saveLogPath, workManifestPath, patchPath, saveResult.status, "not-started", {}, saveResult.error, logError);
        return finishSaveWork (session, false, saveResult.status, "not-started", workManifestPath, patchPath, saveLogPath, saveResult.error);
    }

    const auto reloadedPatch = loadPatchDocument (patchPath);
    if (! reloadedPatch.ok)
    {
        std::string logError;
        appendSaveLog (saveLogPath, workManifestPath, patchPath, "validation-failed", "not-started", {}, reloadedPatch.error, logError);
        return finishSaveWork (session, false, "validation-failed", "not-started", workManifestPath, patchPath, saveLogPath, reloadedPatch.error);
    }

    std::string logError;
    const auto initialCommitStatus = options.startLocalGitCommit ? "commit-pending" : "not-started";
    if (! appendSaveLog (saveLogPath, workManifestPath, patchPath, saveResult.status, initialCommitStatus, {}, {}, logError))
        return finishSaveWork (session, false, "write-failed", "not-started", workManifestPath, patchPath, saveLogPath, logError);

    session.dirty = false;

    if (! options.startLocalGitCommit)
        return finishSaveWork (session, true, saveResult.status, "not-started", workManifestPath, patchPath, saveLogPath, {});

    auto future = std::async (std::launch::async, [workManifestPath, patchPath, saveLogPath, commitMessage = options.commitMessage] {
        return commitSavedWorkInGit (workManifestPath, patchPath, saveLogPath, commitMessage);
    });
    auto commitJob = std::make_shared<SaveWorkCommitJob> (std::move (future));
    return finishSaveWork (session,
                           true,
                           saveResult.status,
                           "commit-pending",
                           workManifestPath,
                           patchPath,
                           saveLogPath,
                           {},
                           commitJob);
}

SaveLogLoadResult loadSaveLog (const std::string& saveLogPath)
{
    std::ifstream input (saveLogPath);
    if (! input)
        return { false, {}, "could not open save log: " + saveLogPath };

    SaveLogLoadResult result;
    result.ok = true;

    std::string line;
    while (std::getline (input, line))
    {
        if (line.find_first_not_of (" \t\r\n") == std::string::npos)
            continue;

        auto entry = parseSaveLogEntry (line);
        if (entry.command.empty() || entry.status.empty())
            return { false, {}, "could not parse save log entry: " + line };

        result.entries.push_back (std::move (entry));
    }

    return result;
}
}
