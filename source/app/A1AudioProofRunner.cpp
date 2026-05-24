#include "A1AudioProofRunner.h"

#include "CompoundPatch.h"
#include "RuntimeRegistry.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* displayName = "A1 audio";
constexpr const char* directoryName = "a1-audio-proof";
constexpr const char* audioStatsFileName = "audio_stats.json";
constexpr const char* loudnessCompoundFileName = "loudness_compound.json";
constexpr const char* loudnessRuntimeExecutionFileName = "loudness_runtime_execution.json";
constexpr const char* loudnessRuntimeBridgeFileName = "loudness_runtime_bridge.json";
constexpr const char* moduleLibraryPath = "fixtures/module-libraries/default.module-library.json";

std::string jsonQuoted (const std::string& text)
{
    std::ostringstream out;
    out << '"';

    for (const auto character : text)
    {
        switch (character)
        {
            case '\\': out << "\\\\"; break;
            case '"': out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default: out << character; break;
        }
    }

    out << '"';
    return out.str();
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

RuntimeRegistryLoadResult loadAudioProofRuntimeRegistry (const std::vector<std::filesystem::path>& candidateRoots)
{
    std::string lastError;

    for (const auto& path : candidatePaths (candidateRoots, moduleLibraryPath))
    {
        const auto registry = loadRuntimeRegistryFromModuleLibrary (path.string());
        if (registry.ok)
            return registry;

        lastError = registry.error;
    }

    return { false, {}, lastError.empty() ? "could not load module library: " + std::string (moduleLibraryPath)
                                          : lastError };
}

std::string makeAudioStatsJson (const A1AudioProofRunRequest& request)
{
    const auto& snapshot = request.snapshot;
    const auto& preferences = request.preferences;

    std::ostringstream out;
    out << std::fixed;
    out << "{\n";
    out << "  \"sampleRate\": " << std::setprecision (0) << request.sampleRate << ",\n";
    out << "  \"bufferSize\": " << request.bufferSize << ",\n";
    out << "  \"rms\": " << std::setprecision (6) << snapshot.rms << ",\n";
    out << "  \"peak\": " << std::setprecision (6) << snapshot.peak << ",\n";
    out << "  \"loudness\": " << std::setprecision (6) << snapshot.loudness << ",\n";
    out << "  \"gate\": " << std::setprecision (6) << snapshot.gate << ",\n";
    out << "  \"confidence\": " << std::setprecision (6) << snapshot.confidence << ",\n";
    out << "  \"active\": " << (snapshot.active ? "true" : "false") << ",\n";
    out << "  \"analysisGain\": " << std::setprecision (3) << preferences.audio.analysisGain << ",\n";
    out << "  \"midi\": {\n";
    out << "    \"streamEnabled\": " << (preferences.midi.streamEnabled ? "true" : "false") << ",\n";
    out << "    \"mapModeEnabled\": " << (preferences.midi.mapModeEnabled ? "true" : "false") << ",\n";
    out << "    \"channel\": " << preferences.midi.channel << ",\n";
    out << "    \"loudnessCc\": " << preferences.midi.loudnessCc << ",\n";
    out << "    \"mapCc\": " << preferences.midi.mapCc << ",\n";
    out << "    \"outputName\": " << jsonQuoted (preferences.midi.outputName) << "\n";
    out << "  },\n";
    out << "  \"sampleCounter\": " << snapshot.sampleCounter << "\n";
    out << "}\n";
    return out.str();
}

A1AudioProofRunResult makeInitialResult (const A1AudioProofRunRequest& request)
{
    A1AudioProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / audioStatsFileName;
    result.artifactPaths = {
        request.outputDirectory / audioStatsFileName,
        request.outputDirectory / loudnessCompoundFileName,
        request.outputDirectory / loudnessRuntimeExecutionFileName,
        request.outputDirectory / loudnessRuntimeBridgeFileName
    };
    return result;
}
}

const char* a1AudioProofDisplayName()
{
    return displayName;
}

const char* a1AudioProofDirectoryName()
{
    return directoryName;
}

A1AudioProofRunResult runA1AudioProof (const A1AudioProofRunRequest& request)
{
    auto result = makeInitialResult (request);

    const auto fail = [&] (const std::string& message)
    {
        result.ok = false;
        result.status = "failed";
        result.error = message;
        return result;
    };

    if (const auto error = createDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    const auto runtimeRegistry = loadAudioProofRuntimeRegistry (request.candidateRoots);
    if (! runtimeRegistry.ok)
        return fail (runtimeRegistry.error);

    const auto runtimeInput = makeRuntimeSyntheticAudioInputFromAnalyzerSnapshot (request.snapshot, 64);
    const auto runtimeExecution = executeRuntimeRegistryWithSyntheticAudio (runtimeRegistry.registry, runtimeInput);
    if (! runtimeExecution.ok)
        return fail (runtimeExecution.error);

    const auto bridge = makeLoudnessRuntimeBridgeSnapshot (runtimeExecution.snapshot, request.snapshot);
    const auto writes = {
        std::pair<std::filesystem::path, std::string> {
            request.outputDirectory / audioStatsFileName,
            makeAudioStatsJson (request)
        },
        std::pair<std::filesystem::path, std::string> {
            request.outputDirectory / loudnessCompoundFileName,
            makeCompoundPatchJson (makeLoudnessCompoundPatchSpec())
        },
        std::pair<std::filesystem::path, std::string> {
            request.outputDirectory / loudnessRuntimeExecutionFileName,
            makeRuntimeExecutionJson (runtimeExecution.snapshot)
        },
        std::pair<std::filesystem::path, std::string> {
            request.outputDirectory / loudnessRuntimeBridgeFileName,
            makeLoudnessRuntimeBridgeJson (bridge)
        }
    };

    for (const auto& [path, text] : writes)
    {
        if (const auto error = writeTextFile (path, text); ! error.empty())
            return fail (error);
    }

    result.ok = true;
    result.status = "dumped";
    return result;
}
}
