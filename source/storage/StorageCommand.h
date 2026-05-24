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

struct PublishModuleRequest
{
    std::string workManifestPath;
    std::string sourceNodeId;
    std::string moduleId;
    std::string moduleTitle;
    std::string nodeType;
    std::string packageDirectory;
    std::string targetLibraryPath;
    bool overwriteExisting = false;
};

struct PublishModuleResult
{
    bool ok = false;
    std::string operation = "publish_module";
    std::string status;
    std::string sourceNodeId;
    std::string sourceNodeType;
    std::string moduleId;
    std::string publishedNodeType;
    std::string moduleManifestPath;
    std::string compoundPatchPath;
    std::string targetLibraryPath;
    std::string error;
};

SaveWorkResult saveWork (GraphSession& session, const std::string& workManifestPath);
SaveWorkResult saveWork (GraphSession& session, const std::string& workManifestPath, const SaveWorkOptions& options);
SaveLogLoadResult loadSaveLog (const std::string& saveLogPath);
PublishModuleResult publishModule (GraphSession& session, const PublishModuleRequest& request);
}
