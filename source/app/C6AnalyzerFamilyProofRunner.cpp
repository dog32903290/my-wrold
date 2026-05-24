#include "C6AnalyzerFamilyProofRunner.h"

#include "CompoundModule.h"
#include "GraphContract.h"
#include "InteractionContract.h"
#include "ProofReports.h"
#include "RuntimeRegistry.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* displayName = "C6 analyzer family";
constexpr const char* directoryName = "c6-analyzer-family-proof";
constexpr const char* moduleLibraryPath = "fixtures/module-libraries/analyzer-family.module-library.json";
constexpr const char* reportFileName = "analyzer_family_report.json";
constexpr const char* rawEnergyNodeType = "compound.raw-energy";
constexpr const char* loudnessNodeType = "compound.loudness";
constexpr const char* rawEnergyNodeId = "raw_energy1";
constexpr const char* rmsOutputId = "rms";
constexpr const char* peakOutputId = "peak";
constexpr const char* sampleCountOutputId = "sampleCount";

bool nearlyEqual (double lhs, double rhs)
{
    return std::abs (lhs - rhs) < 0.000001;
}

const RuntimeOutputValue* findRuntimeOutput (const std::vector<RuntimeOutputValue>& outputs, const std::string& id)
{
    for (const auto& output : outputs)
        if (output.id == id)
            return &output;

    return nullptr;
}

double runtimeOutputValueOrZero (const std::vector<RuntimeOutputValue>& outputs, const std::string& id)
{
    const auto* output = findRuntimeOutput (outputs, id);
    return output == nullptr ? 0.0 : output->value;
}

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

C6AnalyzerFamilyProofRunResult makeInitialResult (const C6AnalyzerFamilyProofRunRequest& request)
{
    C6AnalyzerFamilyProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath };
    return result;
}
}

const char* c6AnalyzerFamilyProofDisplayName()
{
    return displayName;
}

const char* c6AnalyzerFamilyProofDirectoryName()
{
    return directoryName;
}

C6AnalyzerFamilyProofRunResult runC6AnalyzerFamilyProof (const C6AnalyzerFamilyProofRunRequest& request)
{
    auto result = makeInitialResult (request);
    const std::vector<RuntimeOutputValue> emptyOutputs;

    const auto writeReport = [&] (bool ok,
                                  size_t familyEntryCount,
                                  bool visibleRegistryContainsRawEnergy,
                                  bool runtimeRegistryContainsRawEnergy,
                                  const std::string& runtimeCoverageStatus,
                                  bool createdRawEnergyNode,
                                  const std::string& graphCommandLogStatus,
                                  bool loudnessStillPresent,
                                  const std::vector<RuntimeOutputValue>& rawEnergyPublicOutputs,
                                  const std::string& error)
    {
        return writeTextFile (result.reportPath,
                              makeC6AnalyzerFamilyReportJson (ok,
                                                              moduleLibraryPath,
                                                              familyEntryCount,
                                                              visibleRegistryContainsRawEnergy,
                                                              runtimeRegistryContainsRawEnergy,
                                                              runtimeCoverageStatus,
                                                              createdRawEnergyNode,
                                                              graphCommandLogStatus,
                                                              loudnessStillPresent,
                                                              rawEnergyPublicOutputs,
                                                              error));
    };

    const auto fail = [&] (const std::string& message)
    {
        const auto writeError = writeReport (false, 0, false, false, {}, false, {}, false, emptyOutputs, message);
        result.ok = false;
        result.status = "failed";
        result.error = writeError.empty() ? message : writeError;
        return result;
    };

    if (const auto error = clearDirectoryIfExists (request.outputDirectory); ! error.empty())
        return fail (error);

    if (const auto error = createDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    CompoundModuleNodeSpecsResult loadedSpecs;
    std::string loadedLibraryPath;
    std::string lastLoadError;

    for (const auto& candidate : candidatePaths (request.candidateRoots, moduleLibraryPath))
    {
        loadedSpecs = loadCompoundModuleNodeSpecsFromLibrary (candidate.string());
        if (loadedSpecs.ok)
        {
            loadedLibraryPath = candidate.string();
            break;
        }

        lastLoadError = loadedSpecs.error;
    }

    if (! loadedSpecs.ok && loadedSpecs.error.empty())
        loadedSpecs.error = lastLoadError.empty() ? "could not load analyzer family module library" : lastLoadError;

    const auto familyEntryCount = loadedSpecs.ok ? loadedSpecs.specs.size() : 0;
    const auto visibleRegistryContainsRawEnergy = loadedSpecs.ok
        && findNodeSpec (loadedSpecs.specs, rawEnergyNodeType) != nullptr;
    const auto loudnessStillPresent = loadedSpecs.ok
        && findNodeSpec (loadedSpecs.specs, loudnessNodeType) != nullptr;

    const auto runtime = loadedSpecs.ok ? loadRuntimeRegistryFromModuleLibrary (loadedLibraryPath)
                                        : RuntimeRegistryLoadResult {};
    const auto runtimeRegistryContainsRawEnergy = runtime.ok
        && std::any_of (runtime.registry.entries.begin(),
                        runtime.registry.entries.end(),
                        [] (const auto& entry) {
                            return entry.nodeType == rawEnergyNodeType;
                        });
    const auto coverage = runtime.ok ? inspectRuntimeOpCoverage (runtime.registry) : RuntimeOpCoverageResult {};
    const auto diagnostics = coverage.snapshot.entries.empty()
        ? std::vector<RuntimeOpModuleDiagnostic> {}
        : makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    const auto runtimeCoverageStatus = [&diagnostics]
    {
        for (const auto& diagnostic : diagnostics)
        {
            if (diagnostic.nodeType != rawEnergyNodeType)
                continue;

            return diagnostic.status == "runtime-op-ready" ? std::string { "ready" } : diagnostic.status;
        }

        return std::string {};
    }();

    RuntimeSyntheticAudioInput input;
    input.channels = {
        { 0.0f, 1.0f, -1.0f, 0.0f },
        { 0.0f, 0.5f, -0.5f, 0.0f }
    };
    input.analysisGain = 1.5f;

    const auto execution = runtime.ok ? executeRuntimeRegistryWithSyntheticAudio (runtime.registry, input)
                                      : RuntimeExecutionResult {};
    std::vector<RuntimeOutputValue> rawEnergyPublicOutputs;
    std::string rawEnergyExecutionStatus;

    if (execution.ok)
    {
        for (const auto& entry : execution.snapshot.entries)
        {
            if (entry.nodeType != rawEnergyNodeType)
                continue;

            rawEnergyExecutionStatus = entry.status;
            rawEnergyPublicOutputs = entry.publicOutputs;
            break;
        }
    }

    const auto rawOutputsOk = rawEnergyExecutionStatus == "computed"
        && findRuntimeOutput (rawEnergyPublicOutputs, rmsOutputId) != nullptr
        && findRuntimeOutput (rawEnergyPublicOutputs, peakOutputId) != nullptr
        && findRuntimeOutput (rawEnergyPublicOutputs, sampleCountOutputId) != nullptr
        && nearlyEqual (runtimeOutputValueOrZero (rawEnergyPublicOutputs, rmsOutputId), std::sqrt (0.28125))
        && nearlyEqual (runtimeOutputValueOrZero (rawEnergyPublicOutputs, peakOutputId), 0.75)
        && nearlyEqual (runtimeOutputValueOrZero (rawEnergyPublicOutputs, sampleCountOutputId), 4.0);

    auto session = makeGraphSession (makeDefaultShaderOutputGraph());
    CommandResult createResult { false, "raw-energy node spec not loaded" };
    if (loadedSpecs.ok)
    {
        const auto visibleRegistry = mergeNodeSpecs (makeSeedNodeSpecs(), loadedSpecs.specs);
        createResult = createNode (session,
                                   visibleRegistry,
                                   rawEnergyNodeType,
                                   rawEnergyNodeId,
                                   { 300.0, 320.0 });
    }

    const auto graphCommandLogStatus = session.commandLog.empty() ? std::string {}
                                                                  : session.commandLog.back();
    const auto createdRawEnergyNode = createResult.ok && graphCommandLogStatus == "create_node";
    const auto ok = loadedSpecs.ok
                    && familyEntryCount == 2
                    && visibleRegistryContainsRawEnergy
                    && runtime.ok
                    && runtimeRegistryContainsRawEnergy
                    && coverage.ok
                    && runtimeCoverageStatus == "ready"
                    && execution.ok
                    && rawOutputsOk
                    && createdRawEnergyNode
                    && loudnessStillPresent;
    const auto error = ok ? std::string {}
                          : ! loadedSpecs.ok ? loadedSpecs.error
                          : ! runtime.ok ? runtime.error
                          : ! coverage.ok ? coverage.error
                          : ! execution.ok ? execution.error
                          : ! createResult.ok ? createResult.message
                          : "C6 analyzer family proof did not match expected raw-energy evidence";

    if (const auto writeError = writeReport (ok,
                                             familyEntryCount,
                                             visibleRegistryContainsRawEnergy,
                                             runtimeRegistryContainsRawEnergy,
                                             runtimeCoverageStatus,
                                             createdRawEnergyNode,
                                             graphCommandLogStatus,
                                             loudnessStillPresent,
                                             rawEnergyPublicOutputs,
                                             error);
        ! writeError.empty())
    {
        result.ok = false;
        result.status = "failed";
        result.error = writeError;
        return result;
    }

    result.ok = ok;
    result.status = ok ? "dumped" : "mismatch";
    result.error = error;
    return result;
}
}
