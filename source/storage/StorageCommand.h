#pragma once

#include <string>

namespace myworld
{
struct GraphSession;

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
}
