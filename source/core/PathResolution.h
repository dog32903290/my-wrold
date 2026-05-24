#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct PathResolutionResult
{
    std::string anchorPath;
    std::string candidatePath;
    std::string resolvedPath;
    bool found = false;
    std::vector<std::string> attemptedPaths;
};

PathResolutionResult resolvePathNearWithReport (const std::string& anchorPath,
                                                const std::string& candidatePath,
                                                int maxParentDepth = 8);
std::string resolvePathNear (const std::string& anchorPath, const std::string& candidatePath);
std::string describePathResolutionFailure (const std::string& label, const PathResolutionResult& result);
}
