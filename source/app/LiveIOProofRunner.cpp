#include "LiveIOProofRunner.h"

#include "AudioAnalyzerState.h"
#include "LiveIOBus.h"
#include "LiveIOSendAdapter.h"
#include "ProofRunSupport.h"
#include "RuntimeRegistry.h"

#include <filesystem>
#include <sstream>
#include <utility>

namespace myworld
{
namespace
{
constexpr const char* displayName = "P-LIVE1 live IO";
constexpr const char* directoryName = "p-live1-live-io-proof";
constexpr const char* liveIOReportFileName = "live_io_report.json";
constexpr const char* liveIOSendReportFileName = "live_io_send_report.json";
constexpr const char* runtimeExecutionFileName = "live_io_runtime_execution.json";
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
            case '"':  out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:   out << character; break;
        }
    }

    out << '"';
    return out.str();
}

RuntimeRegistryLoadResult loadLiveIOProofRuntimeRegistry (const std::vector<std::filesystem::path>& candidateRoots)
{
    std::string lastError;

    for (const auto& path : proofCandidatePaths (candidateRoots, moduleLibraryPath))
    {
        const auto registry = loadRuntimeRegistryFromModuleLibrary (path.string());
        if (registry.ok)
            return registry;

        lastError = registry.error;
    }

    return { false, {}, lastError.empty() ? "could not load module library: " + std::string (moduleLibraryPath)
                                          : lastError };
}

LiveIOProofRunResult makeInitialResult (const LiveIOProofRunRequest& request)
{
    LiveIOProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / liveIOReportFileName;
    result.artifactPaths = {
        request.outputDirectory / liveIOReportFileName,
        request.outputDirectory / liveIOSendReportFileName,
        request.outputDirectory / runtimeExecutionFileName
    };
    return result;
}

AudioAnalyzerSnapshot makeProofAnalyzerSnapshot (float loudness)
{
    AudioAnalyzerSnapshot snapshot;
    snapshot.rms = loudness;
    snapshot.peak = loudness;
    snapshot.loudness = loudness;
    snapshot.gate = loudness > 0.0f ? 1.0f : 0.0f;
    snapshot.confidence = snapshot.gate;
    snapshot.active = snapshot.gate > 0.0f;
    snapshot.sampleCounter = 64;
    return snapshot;
}

LiveIOValueFrame makeLiveIOFrameFromRuntimeExecution (const RuntimeExecutionSnapshot& snapshot)
{
    LiveIOValueFrame frame;

    for (const auto& entry : snapshot.entries)
    {
        if (entry.status != "computed")
            continue;

        for (const auto& output : entry.publicOutputs)
            frame.values.push_back ({ output.id, output.value, output.source });

        break;
    }

    return frame;
}

std::vector<LiveIOBinding> makeProofBindings()
{
    return {
        makeLiveIOMidiCcBinding ("midi.loudness", "out", 1, 20),
        makeLiveIOOscFloatBinding ("osc.loudness", "out", "/my-world/loudness"),
        makeLiveIOShaderUniformBinding ("uniform.loudness", "out", "u_loudness")
    };
}

LiveIOSendRoute makeProofSendRoute()
{
    return makeLiveIODryRunSendRoute ("dry-run MIDI", "127.0.0.1", 9000);
}

void appendErrorsJson (std::ostringstream& out, const std::vector<std::string>& errors)
{
    out << "[";

    for (size_t index = 0; index < errors.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << jsonQuoted (errors[index]);
    }

    out << "]";
}

std::string makeLiveIOProofJson (const LiveIOBusReport& busReport)
{
    const auto busJson = makeLiveIOBusReportJson (busReport);
    const auto busJsonEnd = busJson.find_last_not_of (" \t\r\n");
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"liveIOProof\",\n";
    out << "  \"ok\": " << (busReport.ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (busReport.ok ? "dumped" : "failed") << ",\n";
    out << "  \"runtimeSource\": \"compound.loudness.publicOutputs\",\n";
    out << "  \"bus\": " << busJson.substr (0, busJsonEnd + 1) << ",\n";
    out << "  \"errors\": ";
    appendErrorsJson (out, busReport.errors);
    out << "\n";
    out << "}\n";
    return out.str();
}
}

const char* liveIOProofDisplayName()
{
    return displayName;
}

const char* liveIOProofDirectoryName()
{
    return directoryName;
}

LiveIOProofRunResult runLiveIOProof (const LiveIOProofRunRequest& request)
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

    const auto runtimeRegistry = loadLiveIOProofRuntimeRegistry (request.candidateRoots);
    if (! runtimeRegistry.ok)
        return fail (runtimeRegistry.error);

    const auto analyzerSnapshot = makeProofAnalyzerSnapshot (request.loudness);
    const auto runtimeInput = makeRuntimeSyntheticAudioInputFromAnalyzerSnapshot (analyzerSnapshot, 64);
    const auto runtimeExecution = executeRuntimeRegistryWithSyntheticAudio (runtimeRegistry.registry, runtimeInput);
    if (! runtimeExecution.ok)
        return fail (runtimeExecution.error);

    const auto frame = makeLiveIOFrameFromRuntimeExecution (runtimeExecution.snapshot);
    const auto busReport = evaluateLiveIOBus (frame, makeProofBindings());
    if (! busReport.ok)
        return fail (busReport.message);

    const auto sendReport = evaluateLiveIOSendBoundary (busReport, makeProofSendRoute());
    if (! sendReport.ok)
        return fail (sendReport.message);

    const auto writes = {
        std::pair<std::filesystem::path, std::string> {
            request.outputDirectory / liveIOReportFileName,
            makeLiveIOProofJson (busReport)
        },
        std::pair<std::filesystem::path, std::string> {
            request.outputDirectory / liveIOSendReportFileName,
            makeLiveIOSendReportJson (sendReport)
        },
        std::pair<std::filesystem::path, std::string> {
            request.outputDirectory / runtimeExecutionFileName,
            makeRuntimeExecutionJson (runtimeExecution.snapshot)
        }
    };

    for (const auto& [path, text] : writes)
    {
        if (const auto error = writeProofTextFile (path, text); ! error.empty())
            return fail (error);
    }

    result.ok = true;
    result.status = "dumped";
    return result;
}
}
