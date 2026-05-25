#include "LiveIOProofRunner.h"

#include "AudioAnalyzerState.h"
#include "LiveIOBus.h"
#include "LiveIOMidiOutputInventory.h"
#include "LiveIOSendAdapter.h"
#include "ProofRunSupport.h"
#include "RuntimeRegistry.h"

#include <cstring>
#include <filesystem>
#include <iomanip>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sstream>
#include <utility>
#include <unistd.h>

namespace myworld
{
namespace
{
constexpr const char* displayName = "P-LIVE1 live IO";
constexpr const char* directoryName = "p-live1-live-io-proof";
constexpr const char* liveIOReportFileName = "live_io_report.json";
constexpr const char* liveIOSendReportFileName = "live_io_send_report.json";
constexpr const char* liveIOOscLoopbackReportFileName = "live_io_osc_loopback_report.json";
constexpr const char* liveIOMidiInventoryReportFileName = "live_io_midi_inventory_report.json";
constexpr const char* runtimeExecutionFileName = "live_io_runtime_execution.json";
constexpr const char* moduleLibraryPath = "fixtures/module-libraries/default.module-library.json";

struct LoopbackReceiver
{
    int socketFd = -1;
    int port = 0;

    LoopbackReceiver() = default;
    LoopbackReceiver (const LoopbackReceiver&) = delete;
    LoopbackReceiver& operator= (const LoopbackReceiver&) = delete;

    LoopbackReceiver (LoopbackReceiver&& other) noexcept
        : socketFd (other.socketFd),
          port (other.port)
    {
        other.socketFd = -1;
        other.port = 0;
    }

    LoopbackReceiver& operator= (LoopbackReceiver&& other) noexcept
    {
        if (this != &other)
        {
            if (socketFd >= 0)
                ::close (socketFd);

            socketFd = other.socketFd;
            port = other.port;
            other.socketFd = -1;
            other.port = 0;
        }

        return *this;
    }

    ~LoopbackReceiver()
    {
        if (socketFd >= 0)
            ::close (socketFd);
    }
};

struct OscLoopbackProof
{
    bool ok = false;
    std::string status = "blocked";
    std::string error;
    LiveIOSendReport sendReport;
    bool received = false;
    std::string oscHost = "127.0.0.1";
    int oscPort = 0;
    std::string receivedAddress;
    double receivedFloatValue = 0.0;
    std::vector<std::string> errors;
};

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
        request.outputDirectory / liveIOOscLoopbackReportFileName,
        request.outputDirectory / liveIOMidiInventoryReportFileName,
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

LiveIOMidiOutputRouteReport makeSelectedMidiRouteReport (const LiveIOMidiOutputInventory& inventory)
{
    if (inventory.devices.empty())
        return evaluateLiveIOMidiOutputRoute (
            inventory,
            makeLiveIOMidiOutputRouteByIdentifier ("__no_live_io_midi_outputs__"));

    return evaluateLiveIOMidiOutputRoute (
        inventory,
        makeLiveIOMidiOutputRouteByIdentifier (inventory.devices.front().identifier));
}

LiveIOMidiOutputRouteReport makeUnavailableMidiRouteReport (const LiveIOMidiOutputInventory& inventory)
{
    return evaluateLiveIOMidiOutputRoute (
        inventory,
        makeLiveIOMidiOutputRouteByIdentifier ("__missing_live_io_midi_output__"));
}

LoopbackReceiver openLoopbackReceiver (std::string& error)
{
    LoopbackReceiver receiver;
    receiver.socketFd = ::socket (AF_INET, SOCK_DGRAM, 0);
    if (receiver.socketFd < 0)
    {
        error = "could not open osc loopback receiver";
        return receiver;
    }

    sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
    address.sin_port = 0;

    if (::bind (receiver.socketFd, reinterpret_cast<sockaddr*> (&address), sizeof (address)) != 0)
    {
        error = "could not bind osc loopback receiver";
        return receiver;
    }

    socklen_t size = sizeof (address);
    if (::getsockname (receiver.socketFd, reinterpret_cast<sockaddr*> (&address), &size) != 0)
    {
        error = "could not read osc loopback receiver port";
        return receiver;
    }

    receiver.port = ntohs (address.sin_port);
    return receiver;
}

std::vector<unsigned char> receiveLoopbackDatagram (int socketFd, std::string& error)
{
    fd_set readSet;
    FD_ZERO (&readSet);
    FD_SET (socketFd, &readSet);

    timeval timeout {};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    const auto ready = ::select (socketFd + 1, &readSet, nullptr, nullptr, &timeout);
    if (ready <= 0)
    {
        error = "did not receive osc loopback packet";
        return {};
    }

    std::vector<unsigned char> buffer (256);
    const auto bytes = ::recv (socketFd, buffer.data(), buffer.size(), 0);
    if (bytes <= 0)
    {
        error = "could not read osc loopback packet";
        return {};
    }

    buffer.resize (static_cast<size_t> (bytes));
    return buffer;
}

size_t paddedOscStringSize (const std::string& text)
{
    const auto lengthWithNull = text.size() + 1;
    return ((lengthWithNull + 3) / 4) * 4;
}

float readOscFloat (const std::vector<unsigned char>& datagram, size_t offset, std::string& error)
{
    if (offset + 4 > datagram.size())
    {
        error = "osc loopback packet is missing float payload";
        return 0.0f;
    }

    const uint32_t bits = (static_cast<uint32_t> (datagram[offset]) << 24)
        | (static_cast<uint32_t> (datagram[offset + 1]) << 16)
        | (static_cast<uint32_t> (datagram[offset + 2]) << 8)
        | static_cast<uint32_t> (datagram[offset + 3]);

    float value = 0.0f;
    std::memcpy (&value, &bits, sizeof (value));
    return value;
}

OscLoopbackProof runOscLoopbackProof (const LiveIOBusReport& busReport)
{
    OscLoopbackProof proof;

    std::string error;
    auto receiver = openLoopbackReceiver (error);
    if (! error.empty())
    {
        proof.error = error;
        proof.errors.push_back (error);
        return proof;
    }

    proof.oscPort = receiver.port;
    proof.sendReport = executeLiveIOSendBoundary (
        busReport,
        makeLiveIOControlledOscLoopbackRoute (proof.oscHost, receiver.port));
    if (! proof.sendReport.ok)
    {
        proof.error = proof.sendReport.message;
        proof.errors = proof.sendReport.errors;
        return proof;
    }

    const auto datagram = receiveLoopbackDatagram (receiver.socketFd, error);
    if (! error.empty())
    {
        proof.error = error;
        proof.errors.push_back (error);
        return proof;
    }

    proof.receivedAddress = std::string (reinterpret_cast<const char*> (datagram.data()));
    const auto typeOffset = paddedOscStringSize (proof.receivedAddress);
    const auto typeTag = std::string (reinterpret_cast<const char*> (datagram.data() + typeOffset));
    if (typeTag != ",f")
    {
        proof.error = "osc loopback packet has unexpected type tag";
        proof.errors.push_back (proof.error);
        return proof;
    }

    const auto valueOffset = typeOffset + paddedOscStringSize (typeTag);
    proof.receivedFloatValue = readOscFloat (datagram, valueOffset, error);
    if (! error.empty())
    {
        proof.error = error;
        proof.errors.push_back (error);
        return proof;
    }

    proof.ok = true;
    proof.status = "received";
    proof.received = true;
    return proof;
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

std::string makeOscLoopbackProofJson (const OscLoopbackProof& proof)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"liveIOOscLoopbackProof\",\n";
    out << "  \"ok\": " << (proof.ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (proof.status) << ",\n";
    out << "  \"sendStatus\": " << jsonQuoted (proof.sendReport.status) << ",\n";
    out << "  \"sent\": " << (! proof.sendReport.actions.empty() && proof.sendReport.actions.front().sent ? "true" : "false") << ",\n";
    out << "  \"received\": " << (proof.received ? "true" : "false") << ",\n";
    out << "  \"oscHost\": " << jsonQuoted (proof.oscHost) << ",\n";
    out << "  \"oscPort\": " << proof.oscPort << ",\n";
    out << "  \"oscAddress\": " << jsonQuoted (proof.receivedAddress) << ",\n";
    out << "  \"receivedFloatValue\": " << proof.receivedFloatValue << ",\n";
    out << "  \"errors\": ";
    appendErrorsJson (out, proof.errors);
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

    const auto loopbackProof = runOscLoopbackProof (busReport);
    if (! loopbackProof.ok)
        return fail (loopbackProof.error);

    const auto selectedMidiRoute = makeSelectedMidiRouteReport (request.midiOutputInventory);
    const auto unavailableMidiRoute = makeUnavailableMidiRouteReport (request.midiOutputInventory);
    const auto midiInventoryReport = makeLiveIOMidiOutputInventoryReportJson (
        request.midiOutputInventory,
        selectedMidiRoute,
        unavailableMidiRoute);

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
            request.outputDirectory / liveIOOscLoopbackReportFileName,
            makeOscLoopbackProofJson (loopbackProof)
        },
        std::pair<std::filesystem::path, std::string> {
            request.outputDirectory / liveIOMidiInventoryReportFileName,
            midiInventoryReport
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
