#include "ProofRunSupport.h"

#include <algorithm>
#include <fstream>
#include <system_error>

namespace myworld
{
std::string writeProofTextFile (const std::filesystem::path& path, const std::string& text)
{
    std::error_code error;
    std::filesystem::create_directories (path.parent_path(), error);
    if (error)
        return "could not create " + path.parent_path().string() + ": " + error.message();

    std::ofstream output (path, std::ios::binary);
    if (! output)
        return "could not write " + path.string();

    output << text;
    if (! output)
        return "could not write " + path.string();

    return {};
}

std::string clearProofDirectoryIfExists (const std::filesystem::path& directory)
{
    std::error_code error;
    if (std::filesystem::exists (directory, error))
    {
        std::filesystem::remove_all (directory, error);
        if (error)
            return "could not clear " + directory.string() + ": " + error.message();
    }

    return {};
}

std::string createProofDirectoryIfMissing (const std::filesystem::path& directory)
{
    std::error_code error;
    std::filesystem::create_directories (directory, error);
    if (error)
        return "could not create " + directory.string() + ": " + error.message();

    return {};
}

std::vector<std::filesystem::path> proofCandidatePaths (const std::vector<std::filesystem::path>& roots,
                                                        std::filesystem::path relativePath)
{
    std::vector<std::filesystem::path> paths;

    for (const auto& root : roots)
    {
        if (! root.empty())
            paths.push_back (root / relativePath);
    }

    paths.push_back (std::filesystem::current_path() / relativePath);
    paths.push_back (std::move (relativePath));

    std::vector<std::filesystem::path> uniquePaths;
    for (const auto& path : paths)
    {
        if (std::find (uniquePaths.begin(), uniquePaths.end(), path) == uniquePaths.end())
            uniquePaths.push_back (path);
    }

    return uniquePaths;
}

bool copyFirstProofCandidate (const std::vector<std::filesystem::path>& roots,
                              std::filesystem::path relativePath,
                              const std::filesystem::path& target,
                              std::string& error)
{
    std::error_code createError;
    std::filesystem::create_directories (target.parent_path(), createError);
    if (createError)
    {
        error = "could not create " + target.parent_path().string() + ": " + createError.message();
        return false;
    }

    const auto relativePathText = relativePath.string();
    for (const auto& candidate : proofCandidatePaths (roots, std::move (relativePath)))
    {
        std::error_code copyError;
        std::filesystem::copy_file (candidate, target, std::filesystem::copy_options::overwrite_existing, copyError);
        if (! copyError)
            return true;

        error = "could not copy " + relativePathText + " from " + candidate.string();
    }

    if (error.empty())
        error = "could not copy " + relativePathText;

    return false;
}
}
