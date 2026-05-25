#include "LiveIOProofRunner.h"

#include "AudioAnalyzerState.h"
#include "LiveIOBus.h"
#include "LiveIOControlDispatcher.h"
#include "LiveIOControlPump.h"
#include "LiveIOControlTimer.h"
#include "LiveIOMidiOutputInventory.h"
#include "LiveIOMidiSendProof.h"
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
constexpr const char* liveIOMidiSendReportFileName = "live_io_midi_send_report.json";
constexpr const char* liveIOControlDispatchReportFileName = "live_io_control_dispatch_report.json";
constexpr const char* liveIOControlPumpReportFileName = "live_io_control_pump_report.json";
constexpr const char* liveIOAppTimerMidiReportFileName = "live_io_app_timer_midi_report.json";
constexpr const char* liveIOAppTimerOscLoopbackReportFileName = "live_io_app_timer_osc_loopback_report.json";
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

struct AppTimerOscLoopbackProof
{
    bool ok = false;
    std::string status = "blocked";
    std::string error;
    LiveIOControlTimerState timerState;
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
        request.outputDirectory / liveIOMidiSendReportFileName,
        request.outputDirectory / liveIOControlDispatchReportFileName,
        request.outputDirectory / liveIOControlPumpReportFileName,
        request.outputDirectory / liveIOAppTimerMidiReportFileName,
        request.outputDirectory / liveIOAppTimerOscLoopbackReportFileName,
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

LiveIOValueFrame makeLiveIOFrameFromLoudness (double loudness)
{
    LiveIOValueFrame frame;
    frame.values = {
        { "out", loudness, "compound.loudness.publicOutputs" }
    };
    return frame;
}

std::vector<LiveIOControlFrame> makeProofControlFrames()
{
    return {
        { 0, makeLiveIOFrameFromLoudness (0.1) },
        { 10, makeLiveIOFrameFromLoudness (0.2) },
        { 50, makeLiveIOFrameFromLoudness (0.5) },
        { 120, makeLiveIOFrameFromLoudness (0.75) }
    };
}

std::vector<LiveIOControlPumpTick> makeProofControlPumpTicks()
{
    auto makeTick = [] (std::int64_t timestampMs,
                        float loudness,
                        bool active,
                        std::uint64_t sampleCounter)
    {
        auto snapshot = makeProofAnalyzerSnapshot (loudness);
        snapshot.active = active;
        snapshot.gate = active ? 1.0f : 0.0f;
        snapshot.confidence = snapshot.gate;
        snapshot.sampleCounter = sampleCounter;
        return LiveIOControlPumpTick { timestampMs, snapshot };
    };

    return {
        makeTick (0, 0.1f, true, 64),
        makeTick (20, 0.2f, true, 128),
        makeTick (50, 0.5f, true, 192),
        makeTick (120, 0.75f, true, 256),
        makeTick (170, 0.0f, false, 320)
    };
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

LiveIOMidiOutputSendReport makeMidiSendReport (const LiveIOMidiOutputInventory& inventory,
                                               const LiveIOBusReport& busReport,
                                               const LiveIOMidiOutputSender& sender)
{
    for (const auto& event : busReport.events)
    {
        if (event.targetKind != LiveIOTargetKind::midiCc)
            continue;

        LiveIOMidiCcMessage message;
        message.bindingId = event.bindingId;
        message.channel = event.midiChannel;
        message.cc = event.midiCc;
        message.value = event.midiValue;

        const auto identifier = inventory.devices.empty()
            ? std::string ("__no_live_io_midi_outputs__")
            : inventory.devices.front().identifier;

        return executeLiveIOMidiOutputSendProof (
            inventory,
            makeLiveIOMidiOutputSendRequestByIdentifier (identifier, message),
            sender);
    }

    LiveIOMidiOutputSendReport report;
    report.status = "blocked";
    report.message = "live io bus has no midi cc event";
    report.errors.push_back (report.message);
    return report;
}

LiveIOControlDispatchReport makeControlDispatchReport (const LiveIOProofRunRequest& request)
{
    LiveIOControlDispatchRequest dispatchRequest;
    dispatchRequest.frames = makeProofControlFrames();
    dispatchRequest.bindings = makeProofBindings();
    dispatchRequest.minIntervalMs = 50;
    dispatchRequest.midiOutputInventory = request.midiOutputInventory;
    dispatchRequest.midiOutputIdentifier = request.midiOutputInventory.devices.empty()
        ? std::string ("__no_live_io_midi_outputs__")
        : request.midiOutputInventory.devices.front().identifier;
    dispatchRequest.midiSender = request.midiOutputSender;
    dispatchRequest.oscSender = request.controlOscSender;
    return executeLiveIOControlDispatch (dispatchRequest);
}

LiveIOControlPumpReport makeControlPumpReport (const LiveIOProofRunRequest& request)
{
    LiveIOControlPumpRequest pumpRequest;
    pumpRequest.ticks = makeProofControlPumpTicks();
    pumpRequest.bindings = makeProofBindings();
    pumpRequest.tickIntervalMs = 50;
    pumpRequest.dispatchMinIntervalMs = 50;
    pumpRequest.midiOutputInventory = request.midiOutputInventory;
    pumpRequest.midiOutputIdentifier = request.midiOutputInventory.devices.empty()
        ? std::string ("__no_live_io_midi_outputs__")
        : request.midiOutputInventory.devices.front().identifier;
    pumpRequest.midiSender = request.midiOutputSender;
    pumpRequest.oscSender = request.controlOscSender;
    return executeLiveIOControlPump (pumpRequest);
}

LiveIOControlTimerState makeAppTimerMidiProofState (const LiveIOProofRunRequest& request)
{
    LiveIOControlTimerConfig config;
    config.bindings = makeProofBindings();
    config.tickIntervalMs = 50;
    config.dispatchMinIntervalMs = 0;
    config.enabled = true;
    config.sendMode = LiveIOControlTimerSendMode::controlledSend;
    config.midiOutputInventory = request.midiOutputInventory;
    config.midiOutputIdentifier = request.midiOutputInventory.devices.empty()
        ? std::string ("__no_live_io_midi_outputs__")
        : request.midiOutputInventory.devices.front().identifier;
    config.midiSender = request.midiOutputSender;
    config.oscEnabled = false;

    LiveIOControlTimerState state;
    const auto result = tickLiveIOControlTimer (state, config, 0, makeProofAnalyzerSnapshot (request.loudness));

    if (! result.ok && state.errors.empty())
        state.errors.push_back (result.message);

    return state;
}

LiveIOBusReport makeBusReportFromOscMessage (const LiveIOOscFloatMessage& message)
{
    LiveIOBusReport report;
    report.ok = true;
    report.status = "mapped";
    report.message = "live_io_mapped";

    LiveIOEvent event;
    event.bindingId = message.bindingId;
    event.sourceId = "out";
    event.source = "LiveIOControlTimer.oscSender";
    event.targetKind = LiveIOTargetKind::oscFloat;
    event.inputValue = message.floatValue;
    event.normalizedValue = message.floatValue;
    event.floatValue = message.floatValue;
    event.oscAddress = message.oscAddress;
    report.events.push_back (event);
    return report;
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

AppTimerOscLoopbackProof runAppTimerOscLoopbackProof (const LiveIOProofRunRequest& request)
{
    AppTimerOscLoopbackProof proof;

    std::string error;
    auto receiver = openLoopbackReceiver (error);
    if (! error.empty())
    {
        proof.error = error;
        proof.errors.push_back (error);
        return proof;
    }

    proof.oscPort = receiver.port;

    LiveIOControlTimerConfig config;
    config.bindings = makeProofBindings();
    config.tickIntervalMs = 50;
    config.dispatchMinIntervalMs = 0;
    config.enabled = true;
    config.sendMode = LiveIOControlTimerSendMode::controlledSend;
    config.midiEnabled = false;
    config.oscEnabled = true;
    config.oscSender = [&proof, receiverPort = receiver.port] (const LiveIOOscFloatMessage& message)
    {
        const auto sendReport = executeLiveIOSendBoundary (
            makeBusReportFromOscMessage (message),
            makeLiveIOControlledOscLoopbackRoute (proof.oscHost, receiverPort));

        if (! sendReport.ok)
            return LiveIOOscFloatSendResult { false, sendReport.message };

        const auto sent = ! sendReport.actions.empty() && sendReport.actions.front().sent;
        return LiveIOOscFloatSendResult {
            sent,
            sent ? std::string() : std::string ("osc loopback send did not mark action sent")
        };
    };

    const auto result = tickLiveIOControlTimer (
        proof.timerState,
        config,
        0,
        makeProofAnalyzerSnapshot (request.loudness));
    if (! result.ok)
    {
        proof.error = result.message;
        proof.errors = proof.timerState.errors;
        if (proof.errors.empty())
            proof.errors.push_back (result.message);
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

std::string makeAppTimerMidiProofJson (const LiveIOControlTimerState& state,
                                       const std::string& outputOperator)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"liveIOAppTimerMidiProof\",\n";
    out << "  \"ok\": " << (state.lastStatus == "controlled_sent" ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (state.lastStatus) << ",\n";
    out << "  \"message\": " << jsonQuoted (state.lastMessage) << ",\n";
    out << "  \"sendMode\": " << jsonQuoted (state.lastSendMode) << ",\n";
    out << "  \"outputOperator\": " << jsonQuoted (outputOperator) << ",\n";
    out << "  \"tickCount\": " << state.tickCount << ",\n";
    out << "  \"pumpCount\": " << state.pumpCount << ",\n";
    out << "  \"midiDryRunCount\": " << state.midiDryRunCount << ",\n";
    out << "  \"oscDryRunCount\": " << state.oscDryRunCount << ",\n";
    out << "  \"midiControlledSendCount\": " << state.midiControlledSendCount << ",\n";
    out << "  \"oscControlledSendCount\": " << state.oscControlledSendCount << ",\n";
    out << "  \"shaderSkippedCount\": " << state.shaderSkippedCount << ",\n";
    out << "  \"hasLastShaderUniform\": " << (state.hasLastShaderUniform ? "true" : "false") << ",\n";
    out << "  \"lastShaderUniformName\": " << jsonQuoted (state.lastShaderUniformName) << ",\n";
    out << "  \"lastShaderUniformValue\": " << state.lastShaderUniformValue << ",\n";
    out << "  \"lastShaderUniformSampleCounter\": " << state.lastShaderUniformSampleCounter << ",\n";
    out << "  \"lastLoudness\": " << state.lastLoudness << ",\n";
    out << "  \"lastSampleCounter\": " << state.lastSampleCounter << ",\n";
    out << "  \"errors\": ";
    appendErrorsJson (out, state.errors);
    out << "\n";
    out << "}\n";
    return out.str();
}

std::string makeAppTimerOscLoopbackProofJson (const AppTimerOscLoopbackProof& proof,
                                              const std::string& outputOperator)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"liveIOAppTimerOscLoopbackProof\",\n";
    out << "  \"ok\": " << (proof.ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (proof.status) << ",\n";
    out << "  \"timerStatus\": " << jsonQuoted (proof.timerState.lastStatus) << ",\n";
    out << "  \"message\": " << jsonQuoted (proof.timerState.lastMessage) << ",\n";
    out << "  \"sendMode\": " << jsonQuoted (proof.timerState.lastSendMode) << ",\n";
    out << "  \"outputOperator\": " << jsonQuoted (outputOperator) << ",\n";
    out << "  \"tickCount\": " << proof.timerState.tickCount << ",\n";
    out << "  \"pumpCount\": " << proof.timerState.pumpCount << ",\n";
    out << "  \"midiControlledSendCount\": " << proof.timerState.midiControlledSendCount << ",\n";
    out << "  \"oscControlledSendCount\": " << proof.timerState.oscControlledSendCount << ",\n";
    out << "  \"shaderSkippedCount\": " << proof.timerState.shaderSkippedCount << ",\n";
    out << "  \"hasLastShaderUniform\": " << (proof.timerState.hasLastShaderUniform ? "true" : "false") << ",\n";
    out << "  \"lastShaderUniformName\": " << jsonQuoted (proof.timerState.lastShaderUniformName) << ",\n";
    out << "  \"lastShaderUniformValue\": " << proof.timerState.lastShaderUniformValue << ",\n";
    out << "  \"lastShaderUniformSampleCounter\": " << proof.timerState.lastShaderUniformSampleCounter << ",\n";
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
    const auto midiSendReport = makeMidiSendReport (
        request.midiOutputInventory,
        busReport,
        request.midiOutputSender);
    if (! midiSendReport.ok)
        return fail (midiSendReport.message);

    const auto controlDispatchReport = makeControlDispatchReport (request);
    if (! controlDispatchReport.ok)
        return fail (controlDispatchReport.message);

    const auto controlPumpReport = makeControlPumpReport (request);
    if (! controlPumpReport.ok)
        return fail (controlPumpReport.message);

    const auto appTimerMidiState = makeAppTimerMidiProofState (request);
    if (appTimerMidiState.lastStatus != "controlled_sent")
        return fail (appTimerMidiState.lastMessage.empty()
                         ? "live io app timer midi proof failed"
                         : appTimerMidiState.lastMessage);

    const auto appTimerOscLoopbackProof = runAppTimerOscLoopbackProof (request);
    if (! appTimerOscLoopbackProof.ok)
        return fail (appTimerOscLoopbackProof.error.empty()
                         ? "live io app timer osc loopback proof failed"
                         : appTimerOscLoopbackProof.error);

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
            request.outputDirectory / liveIOMidiSendReportFileName,
            makeLiveIOMidiOutputSendReportJson (midiSendReport)
        },
        std::pair<std::filesystem::path, std::string> {
            request.outputDirectory / liveIOControlDispatchReportFileName,
            makeLiveIOControlDispatchReportJson (controlDispatchReport)
        },
        std::pair<std::filesystem::path, std::string> {
            request.outputDirectory / liveIOControlPumpReportFileName,
            makeLiveIOControlPumpReportJson (controlPumpReport)
        },
        std::pair<std::filesystem::path, std::string> {
            request.outputDirectory / liveIOAppTimerMidiReportFileName,
            makeAppTimerMidiProofJson (appTimerMidiState, "midi.cc")
        },
        std::pair<std::filesystem::path, std::string> {
            request.outputDirectory / liveIOAppTimerOscLoopbackReportFileName,
            makeAppTimerOscLoopbackProofJson (appTimerOscLoopbackProof, "osc.float")
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
