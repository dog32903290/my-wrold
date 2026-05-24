#pragma once

#include <memory>
#include <future>
#include <string>
#include <vector>

namespace myworld
{
struct GraphSession;
class SaveWorkCommitJob;

struct SaveLogEntry
{
    std::string timestamp;
    std::string command;
    std::string status;
    std::string workManifestPath;
    std::string patchPath;
    std::string commitStatus;
    std::string commitId;
    std::string error;
};

struct SaveLogLoadResult
{
    bool ok = false;
    std::vector<SaveLogEntry> entries;
    std::string error;
};

struct SaveWorkResult
{
    bool ok = false;
    std::string status;
    std::string commitStatus = "not-started";
    std::string workManifestPath;
    std::string patchPath;
    std::string saveLogPath;
    std::string error;
    std::shared_ptr<SaveWorkCommitJob> commitJob;
};

struct SaveWorkCommitResult
{
    bool ok = false;
    std::string status;
    std::string commitId;
    std::string error;
};

class SaveWorkCommitJob
{
public:
    explicit SaveWorkCommitJob (std::future<SaveWorkCommitResult> future);
    SaveWorkCommitResult wait() const;

private:
    std::shared_future<SaveWorkCommitResult> future;
};

struct SaveWorkOptions
{
    bool startLocalGitCommit = false;
    std::string commitMessage = "Save work";
};

SaveWorkResult saveWork (GraphSession& session, const std::string& workManifestPath);
SaveWorkResult saveWork (GraphSession& session, const std::string& workManifestPath, const SaveWorkOptions& options);
SaveLogLoadResult loadSaveLog (const std::string& saveLogPath);
}
