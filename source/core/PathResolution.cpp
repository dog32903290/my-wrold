#include "PathResolution.h"

#include <filesystem>
#include <sstream>
#include <system_error>

namespace myworld
{
namespace
{
bool pathExists (const std::filesystem::path& path)
{
    std::error_code error;
    return std::filesystem::exists (path, error);
}

std::filesystem::path currentPath()
{
    std::error_code error;
    auto path = std::filesystem::current_path (error);
    return error ? std::filesystem::path { "." } : path;
}
}

PathResolutionResult resolvePathNearWithReport (const std::string& anchorPath,
                                                const std::string& candidatePath,
                                                int maxParentDepth)
{
    namespace fs = std::filesystem;

    PathResolutionResult result;
    result.anchorPath = anchorPath;
    result.candidatePath = candidatePath;

    const fs::path candidate { candidatePath };
    result.resolvedPath = candidate.string();

    result.attemptedPaths.push_back (candidate.string());
    if (candidate.is_absolute() || pathExists (candidate))
    {
        result.found = pathExists (candidate);
        return result;
    }

    auto directory = fs::path { anchorPath };
    directory = directory.has_parent_path() ? directory.parent_path() : currentPath();

    for (int depth = 0; depth < maxParentDepth; ++depth)
    {
        const auto resolved = directory / candidate;
        const auto resolvedText = resolved.string();

        if (result.attemptedPaths.empty() || result.attemptedPaths.back() != resolvedText)
            result.attemptedPaths.push_back (resolvedText);

        if (pathExists (resolved))
        {
            result.found = true;
            result.resolvedPath = resolvedText;
            return result;
        }

        if (! directory.has_parent_path() || directory == directory.parent_path())
            break;

        directory = directory.parent_path();
    }

    return result;
}

std::string resolvePathNear (const std::string& anchorPath, const std::string& candidatePath)
{
    return resolvePathNearWithReport (anchorPath, candidatePath).resolvedPath;
}

std::string describePathResolutionFailure (const std::string& label, const PathResolutionResult& result)
{
    std::ostringstream out;
    out << "could not resolve " << label << ": " << result.candidatePath;

    if (! result.anchorPath.empty())
        out << " near " << result.anchorPath;

    out << "; attempted:";

    for (const auto& attempted : result.attemptedPaths)
        out << " " << attempted;

    return out.str();
}
}
