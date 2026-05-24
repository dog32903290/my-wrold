#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct GraphSession;

struct SaveLogEntry
{
    std::string timestamp;
    std::string command;
    std::string status;
    std::string workManifestPath;
    std::string patchPath;
    std::string commitStatus;
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
    std::string workManifestPath;
    std::string patchPath;
    std::string saveLogPath;
    std::string error;
};

SaveWorkResult saveWork (GraphSession& session, const std::string& workManifestPath);
SaveLogLoadResult loadSaveLog (const std::string& saveLogPath);
}
