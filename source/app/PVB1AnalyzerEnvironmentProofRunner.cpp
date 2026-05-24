#include "PVB1AnalyzerEnvironmentProofRunner.h"

#include "AnalyzerVisibleCatalog.h"
#include "ProofRunSupport.h"

namespace myworld
{
namespace
{
constexpr const char* displayName = "PV-B1 analyzer environment";
constexpr const char* directoryName = "pv-b1-analyzer-environment-proof";
constexpr const char* moduleLibraryPath = "fixtures/module-libraries/pv-analyzer-visible.module-library.json";
constexpr const char* reportFileName = "analyzer_environment_report.json";

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
        return writeProofTextFile (result.reportPath, makeAnalyzerVisibleCatalogProofJson (proof));
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

    if (const auto error = clearProofDirectoryIfExists (request.outputDirectory); ! error.empty())
        return fail (error);

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    AnalyzerVisibleCatalogProof proof;
    std::string lastError;

    for (const auto& candidate : proofCandidatePaths (request.candidateRoots, moduleLibraryPath))
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
