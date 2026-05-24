#include "PVDetectorProofRunner.h"

#include "AnalyzerAggregatePressureFixture.h"
#include "AnalyzerDetectorFixture.h"
#include "AnalyzerDensityDetectorFixture.h"
#include "AnalyzerResidueDetectorFixture.h"
#include "AnalyzerSilenceDetectorFixture.h"
#include "AnalyzerSustainDetectorFixture.h"
#include "ProofRunSupport.h"

#include <filesystem>

namespace myworld
{
namespace
{
struct PVDetectorProofDefinition
{
    PVDetectorProofKind kind;
    const char* displayName;
    const char* directoryName;
    const char* moduleLibraryPath;
    const char* fixturePath;
    const char* reportFileName;
};

constexpr PVDetectorProofDefinition attackDefinition {
    PVDetectorProofKind::attack,
    "PV attack detector",
    "pv-attack-detector-proof",
    "fixtures/module-libraries/pv-attack-detector.module-library.json",
    "fixtures/analyzer/attack_detector_cases.json",
    "attack_detector_report.json"
};

constexpr PVDetectorProofDefinition densityDefinition {
    PVDetectorProofKind::density,
    "PV density detector",
    "pv-density-detector-proof",
    "fixtures/module-libraries/pv-density-detector.module-library.json",
    "fixtures/analyzer/density_detector_cases.json",
    "density_detector_report.json"
};

constexpr PVDetectorProofDefinition silenceDefinition {
    PVDetectorProofKind::silence,
    "PV silence detector",
    "pv-silence-detector-proof",
    "fixtures/module-libraries/pv-silence-detector.module-library.json",
    "fixtures/analyzer/silence_detector_cases.json",
    "silence_detector_report.json"
};

constexpr PVDetectorProofDefinition sustainDefinition {
    PVDetectorProofKind::sustain,
    "PV sustain detector",
    "pv-sustain-detector-proof",
    "fixtures/module-libraries/pv-sustain-detector.module-library.json",
    "fixtures/analyzer/sustain_detector_cases.json",
    "sustain_detector_report.json"
};

constexpr PVDetectorProofDefinition residueDefinition {
    PVDetectorProofKind::residue,
    "PV residue detector",
    "pv-residue-detector-proof",
    "fixtures/module-libraries/pv-residue-detector.module-library.json",
    "fixtures/analyzer/residue_detector_cases.json",
    "residue_detector_report.json"
};

constexpr PVDetectorProofDefinition aggregatePressureDefinition {
    PVDetectorProofKind::aggregatePressure,
    "PV aggregate pressure",
    "pv-aggregate-pressure-proof",
    "fixtures/module-libraries/pv-aggregate-pressure.module-library.json",
    "fixtures/analyzer/aggregate_pressure_cases.json",
    "aggregate_pressure_report.json"
};

const PVDetectorProofDefinition& definitionFor (PVDetectorProofKind kind)
{
    switch (kind)
    {
        case PVDetectorProofKind::attack:
            return attackDefinition;
        case PVDetectorProofKind::density:
            return densityDefinition;
        case PVDetectorProofKind::silence:
            return silenceDefinition;
        case PVDetectorProofKind::sustain:
            return sustainDefinition;
        case PVDetectorProofKind::residue:
            return residueDefinition;
        case PVDetectorProofKind::aggregatePressure:
            return aggregatePressureDefinition;
    }

    return attackDefinition;
}

RuntimeRegistryLoadResult loadRuntimeRegistryFromCandidateLibrary (const PVDetectorProofRunRequest& request,
                                                                   const PVDetectorProofDefinition& definition)
{
    std::string lastError;

    for (const auto& path : proofCandidatePaths (request.candidateRoots, definition.moduleLibraryPath))
    {
        const auto registry = loadRuntimeRegistryFromModuleLibrary (path.string());
        if (registry.ok)
            return registry;

        lastError = registry.error;
    }

    return { false,
             {},
             lastError.empty() ? "could not load module library: " + std::string (definition.moduleLibraryPath)
                               : lastError };
}

template <typename LoadFixture>
std::string firstLoadableFixturePath (const PVDetectorProofRunRequest& request,
                                      const PVDetectorProofDefinition& definition,
                                      LoadFixture loadFixture)
{
    for (const auto& path : proofCandidatePaths (request.candidateRoots, definition.fixturePath))
    {
        if (loadFixture (path.string()).ok)
            return path.string();
    }

    return {};
}

PVDetectorProofRunResult makeInitialResult (const PVDetectorProofRunRequest& request,
                                            const PVDetectorProofDefinition& definition)
{
    PVDetectorProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / definition.reportFileName;
    result.artifactPaths = {
        result.reportPath,
        request.outputDirectory / "cook_order.json",
        request.outputDirectory / "node_stats.json",
        request.outputDirectory / "errors.json"
    };
    return result;
}

template <typename ProofResult,
          typename LoadFixture,
          typename RunProof,
          typename MakeReport,
          typename MakeCookOrder,
          typename MakeNodeStats,
          typename MakeErrors>
PVDetectorProofRunResult runDetectorProof (const PVDetectorProofRunRequest& request,
                                           const PVDetectorProofDefinition& definition,
                                           LoadFixture loadFixture,
                                           RunProof runProof,
                                           MakeReport makeReport,
                                           MakeCookOrder makeCookOrder,
                                           MakeNodeStats makeNodeStats,
                                           MakeErrors makeErrors)
{
    auto result = makeInitialResult (request, definition);

    const auto writeArtifacts = [&] (const ProofResult& proof)
    {
        for (const auto& [path, text] : {
                 std::pair<std::filesystem::path, std::string> { result.reportPath, makeReport (proof) },
                 { request.outputDirectory / "cook_order.json", makeCookOrder() },
                 { request.outputDirectory / "node_stats.json", makeNodeStats (proof) },
                 { request.outputDirectory / "errors.json", makeErrors (proof) } })
        {
            if (const auto error = writeProofTextFile (path, text); ! error.empty())
                return error;
        }

        return std::string {};
    };

    const auto fail = [&] (const std::string& message)
    {
        ProofResult failure;
        failure.fixturePath = definition.fixturePath;
        failure.error = message;
        writeArtifacts (failure);

        result.ok = false;
        result.status = "failed";
        result.error = message;
        return result;
    };

    if (const auto error = clearProofDirectoryIfExists (request.outputDirectory); ! error.empty())
        return fail (error);

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    const auto runtime = loadRuntimeRegistryFromCandidateLibrary (request, definition);
    if (! runtime.ok)
        return fail (runtime.error);

    const auto fixturePath = firstLoadableFixturePath (request, definition, loadFixture);
    if (fixturePath.empty())
        return fail ("could not load " + std::string (definition.displayName) + " fixture");

    const auto proof = runProof (runtime.registry, fixturePath);
    if (const auto error = writeArtifacts (proof); ! error.empty())
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

const char* pvDetectorProofDisplayName (PVDetectorProofKind kind)
{
    return definitionFor (kind).displayName;
}

const char* pvDetectorProofDirectoryName (PVDetectorProofKind kind)
{
    return definitionFor (kind).directoryName;
}

PVDetectorProofRunResult runPVDetectorProof (const PVDetectorProofRunRequest& request)
{
    const auto& definition = definitionFor (request.kind);

    switch (request.kind)
    {
        case PVDetectorProofKind::attack:
            return runDetectorProof<AttackDetectorProofResult> (request,
                                                                definition,
                                                                loadAttackDetectorFixture,
                                                                runAttackDetectorProof,
                                                                makeAttackDetectorProofReportJson,
                                                                makeAttackDetectorCookOrderJson,
                                                                makeAttackDetectorNodeStatsJson,
                                                                makeAttackDetectorErrorsJson);
        case PVDetectorProofKind::density:
            return runDetectorProof<DensityDetectorProofResult> (request,
                                                                 definition,
                                                                 loadDensityDetectorFixture,
                                                                 runDensityDetectorProof,
                                                                 makeDensityDetectorProofReportJson,
                                                                 makeDensityDetectorCookOrderJson,
                                                                 makeDensityDetectorNodeStatsJson,
                                                                 makeDensityDetectorErrorsJson);
        case PVDetectorProofKind::silence:
            return runDetectorProof<SilenceDetectorProofResult> (request,
                                                                 definition,
                                                                 loadSilenceDetectorFixture,
                                                                 runSilenceDetectorProof,
                                                                 makeSilenceDetectorProofReportJson,
                                                                 makeSilenceDetectorCookOrderJson,
                                                                 makeSilenceDetectorNodeStatsJson,
                                                                 makeSilenceDetectorErrorsJson);
        case PVDetectorProofKind::sustain:
            return runDetectorProof<SustainDetectorProofResult> (request,
                                                                 definition,
                                                                 loadSustainDetectorFixture,
                                                                 runSustainDetectorProof,
                                                                 makeSustainDetectorProofReportJson,
                                                                 makeSustainDetectorCookOrderJson,
                                                                 makeSustainDetectorNodeStatsJson,
                                                                 makeSustainDetectorErrorsJson);
        case PVDetectorProofKind::residue:
            return runDetectorProof<ResidueDetectorProofResult> (request,
                                                                 definition,
                                                                 loadResidueDetectorFixture,
                                                                 runResidueDetectorProof,
                                                                 makeResidueDetectorProofReportJson,
                                                                 makeResidueDetectorCookOrderJson,
                                                                 makeResidueDetectorNodeStatsJson,
                                                                 makeResidueDetectorErrorsJson);
        case PVDetectorProofKind::aggregatePressure:
            return runDetectorProof<AggregatePressureProofResult> (request,
                                                                   definition,
                                                                   loadAggregatePressureFixture,
                                                                   runAggregatePressureProof,
                                                                   makeAggregatePressureProofReportJson,
                                                                   makeAggregatePressureCookOrderJson,
                                                                   makeAggregatePressureNodeStatsJson,
                                                                   makeAggregatePressureErrorsJson);
    }

    return runDetectorProof<AttackDetectorProofResult> (request,
                                                        definition,
                                                        loadAttackDetectorFixture,
                                                        runAttackDetectorProof,
                                                        makeAttackDetectorProofReportJson,
                                                        makeAttackDetectorCookOrderJson,
                                                        makeAttackDetectorNodeStatsJson,
                                                        makeAttackDetectorErrorsJson);
}
}
