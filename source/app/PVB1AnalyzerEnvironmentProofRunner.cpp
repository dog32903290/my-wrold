#include "PVB1AnalyzerEnvironmentProofRunner.h"

#include "AnalyzerVisibleCatalog.h"

#include <algorithm>
#include <fstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* displayName = "PV-B1 analyzer environment";
constexpr const char* directoryName = "pv-b1-analyzer-environment-proof";
constexpr const char* moduleLibraryPath = "fixtures/module-libraries/pv-analyzer-visible.module-library.json";
constexpr const char* reportFileName = "analyzer_environment_report.json";

std::string writeTextFile (const std::filesystem::path& path, const std::string& text)
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

std::string clearDirectoryIfExists (const std::filesystem::path& directory)
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

std::string createDirectoryIfMissing (const std::filesystem::path& directory)
{
    std::error_code error;
    std::filesystem::create_directories (directory, error);
    if (error)
        return "could not create " + directory.string() + ": " + error.message();

    return {};
}

std::vector<std::filesystem::path> candidatePaths (const std::vector<std::filesystem::path>& roots,
                                                   const char* relativePath)
{
    std::vector<std::filesystem::path> paths;

    for (const auto& root : roots)
    {
        if (! root.empty())
            paths.push_back (root / relativePath);
    }

    paths.push_back (std::filesystem::current_path() / relativePath);
    paths.push_back (std::filesystem::path (relativePath));

    std::vector<std::filesystem::path> uniquePaths;
    for (const auto& path : paths)
    {
        if (std::find (uniquePaths.begin(), uniquePaths.end(), path) == uniquePaths.end())
            uniquePaths.push_back (path);
    }

    return uniquePaths;
}

PVB1AnalyzerEnvironmentProofRunResult makeInitialResult (
    const PVB1AnalyzerEnvironmentProofRunRequest& request)
{
    PVB1AnalyzerEnvironmentProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };
    return result;
}
}

const char* pvB1AnalyzerEnvironmentProofDisplayName()
{
    return displayName;
}

const char* pvB1AnalyzerEnvironmentProofDirectoryName()
{
    return directoryName;
}

PVB1AnalyzerEnvironmentProofRunResult runPVB1AnalyzerEnvironmentProof (
    const PVB1AnalyzerEnvironmentProofRunRequest& request)
{
    auto result = makeInitialResult (request);

    const auto writeReport = [&] (const AnalyzerVisibleCatalogProof& proof)
    {
        return writeTextFile (result.reportPath, makeAnalyzerVisibleCatalogProofJson (proof));
    };

    const auto fail = [&] (const std::string& message)
    {
        AnalyzerVisibleCatalogProof failure;
        failure.libraryPath = moduleLibraryPath;
        failure.requiredNodeTypes = pvB1AnalyzerRequiredNodeTypes();
        failure.error = message;

        const auto writeError = writeReport (failure);
        result.ok = false;
        result.status = "failed";
        result.error = writeError.empty() ? message : writeError;
        return result;
    };

    if (const auto error = clearDirectoryIfExists (request.outputDirectory); ! error.empty())
        return fail (error);

    if (const auto error = createDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    AnalyzerVisibleCatalogProof proof;
    std::string lastError;

    for (const auto& candidate : candidatePaths (request.candidateRoots, moduleLibraryPath))
    {
        proof = proveAnalyzerVisibleCatalog (candidate.string());
        if (proof.ok)
            break;

        lastError = proof.error;
    }

    if (! proof.ok && proof.error.empty())
        proof.error = lastError.empty() ? "could not load PV-B1 analyzer visible module library" : lastError;

    if (const auto error = writeReport (proof); ! error.empty())
    {
        result.ok = false;
        result.status = "failed";
        result.error = error;
        return result;
    }

    result.ok = proof.ok;
    result.status = proof.ok ? "dumped" : "mismatch";
    result.error = proof.error;
    return result;
}
}
