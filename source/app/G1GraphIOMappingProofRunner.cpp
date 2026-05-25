#include "G1GraphIOMappingProofRunner.h"

#include "GraphIOMapping.h"
#include "GraphIOMappingStorage.h"
#include "ProofRunSupport.h"

namespace myworld
{
namespace
{
constexpr const char* displayName = "G1 graph IO mapping";
constexpr const char* directoryName = "g1-graph-io-mapping-proof";
constexpr const char* reportFileName = "graph_io_mapping_report.json";
constexpr const char* graphFixturePath = "fixtures/graphs/g1_loudness_to_shader_uniform.graph.json";

G1GraphIOMappingProofRunResult makeInitialResult (const G1GraphIOMappingProofRunRequest& request)
{
    G1GraphIOMappingProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };
    return result;
}
}

const char* g1GraphIOMappingProofDisplayName()
{
    return displayName;
}

const char* g1GraphIOMappingProofDirectoryName()
{
    return directoryName;
}

G1GraphIOMappingProofRunResult runG1GraphIOMappingProof (const G1GraphIOMappingProofRunRequest& request)
{
    auto result = makeInitialResult (request);

    const auto fail = [&] (const std::string& message)
    {
        result.ok = false;
        result.status = "failed";
        result.error = message;
        return result;
    };

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    GraphIOMappingLoadResult loaded;
    std::string lastError;
    for (const auto& candidate : proofCandidatePaths (request.candidateRoots, graphFixturePath))
    {
        loaded = loadGraphIOMappingsFromFile (candidate.string());
        if (loaded.ok)
            break;

        lastError = loaded.error;
    }

    if (! loaded.ok)
        return fail (lastError.empty() ? "could not load G1 graph IO mapping fixture" : lastError);

    if (loaded.mappings.empty())
        return fail ("G1 graph IO mapping fixture contains no mappings");

    LiveIOValueFrame frame;
    frame.values = {
        { loaded.mappings.front().source.endpoint, request.sourceValue, loaded.mappings.front().source.endpoint }
    };

    const auto report = evaluateGraphIOMapping (loaded.mappings.front(), frame);
    if (const auto error = writeProofTextFile (result.reportPath, makeGraphIOMappingReportJson (report));
        ! error.empty())
    {
        return fail (error);
    }

    result.ok = report.ok;
    result.status = report.ok ? "dumped" : "failed";
    result.error = report.ok ? std::string {} : report.message;
    return result;
}
}
