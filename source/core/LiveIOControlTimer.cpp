#include "LiveIOControlTimer.h"

#include <iomanip>
#include <sstream>

namespace myworld
{
namespace
{
constexpr const char* dryRunMidiIdentifier = "live-io-timer-dry-run";

bool isControlledSendMode (LiveIOControlTimerSendMode mode)
{
    return mode == LiveIOControlTimerSendMode::controlledSend;
}

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

LiveIOMidiOutputInventory makeDryRunMidiInventory()
{
    LiveIOMidiOutputInventory inventory;
    inventory.devices.push_back ({ "Live IO Timer Dry Run", dryRunMidiIdentifier });
    return inventory;
}

LiveIOMidiOutputSender makeDryRunMidiSender()
{
    return [] (const LiveIOMidiOutputDevice&, const LiveIOMidiCcMessage&)
    {
        return LiveIOMidiOutputDeviceSendResult { true, true, "" };
    };
}

LiveIOOscFloatSender makeDryRunOscSender()
{
    return [] (const LiveIOOscFloatMessage&)
    {
        return LiveIOOscFloatSendResult { true, "" };
    };
}

void setStateStatus (LiveIOControlTimerState& state,
                     const std::string& status,
                     const std::string& message)
{
    state.lastStatus = status;
    state.lastMessage = message;
}

LiveIOControlTimerTickResult makeTickResult (const std::string& status,
                                             const std::string& message,
                                             bool pumped)
{
    return LiveIOControlTimerTickResult { true, status, message, pumped };
}

int nonNegativeInterval (int intervalMs)
{
    return intervalMs < 0 ? 0 : intervalMs;
}
}

std::string liveIOControlTimerSendModeToString (LiveIOControlTimerSendMode mode)
{
    if (mode == LiveIOControlTimerSendMode::controlledSend)
        return "controlled_send";

    return "dry_run";
}

LiveIOControlTimerTickResult tickLiveIOControlTimer (
    LiveIOControlTimerState& state,
    const LiveIOControlTimerConfig& config,
    std::int64_t timestampMs,
    const AudioAnalyzerSnapshot& snapshot)
{
    ++state.tickCount;
    state.lastSampleCounter = snapshot.sampleCounter;
    state.lastSendMode = liveIOControlTimerSendModeToString (config.sendMode);

    if (! config.enabled)
    {
        setStateStatus (state, "disabled", "live_io_timer_disabled");
        return makeTickResult ("disabled", state.lastMessage, false);
    }

    if (! snapshot.active)
    {
        ++state.inactiveTickCount;
        setStateStatus (state, "inactive", "live_io_timer_inactive");
        return makeTickResult ("inactive", state.lastMessage, false);
    }

    const auto tickIntervalMs = nonNegativeInterval (config.tickIntervalMs);
    if (state.hasLastPumpTimestamp && timestampMs - state.lastPumpTimestampMs < tickIntervalMs)
    {
        ++state.tickRateLimitedCount;
        setStateStatus (state, "tick_rate_limited", "live_io_timer_tick_rate_limited");
        return makeTickResult ("tick_rate_limited", state.lastMessage, false);
    }

    LiveIOControlPumpRequest request;
    request.ticks = { { timestampMs, snapshot } };
    request.bindings = config.bindings;
    request.tickIntervalMs = 0;
    request.dispatchMinIntervalMs = nonNegativeInterval (config.dispatchMinIntervalMs);
    request.midiEnabled = config.midiEnabled;
    request.oscEnabled = config.oscEnabled;

    if (isControlledSendMode (config.sendMode))
    {
        request.midiOutputInventory = config.midiOutputInventory;
        request.midiOutputIdentifier = config.midiOutputIdentifier;
        request.midiSender = config.midiSender;
        request.oscSender = config.oscSender;
    }
    else
    {
        request.midiOutputInventory = makeDryRunMidiInventory();
        request.midiOutputIdentifier = dryRunMidiIdentifier;
        request.midiSender = makeDryRunMidiSender();
        request.oscSender = makeDryRunOscSender();
    }

    const auto report = executeLiveIOControlPump (request);
    state.lastPumpReport = report;

    if (! report.ok)
    {
        const auto message = report.message.empty() ? "live io timer dry-run pump failed" : report.message;
        state.errors.push_back (message);
        for (const auto& error : report.errors)
            state.errors.push_back (error);

        setStateStatus (state, "failed", message);
        return LiveIOControlTimerTickResult { false, "failed", message, false };
    }

    ++state.pumpCount;
    if (isControlledSendMode (config.sendMode))
    {
        state.midiControlledSendCount += report.dispatch.midiSentCount;
        state.oscControlledSendCount += report.dispatch.oscSentCount;
    }
    else
    {
        state.midiDryRunCount += report.dispatch.midiSentCount;
        state.oscDryRunCount += report.dispatch.oscSentCount;
    }

    state.shaderSkippedCount += report.dispatch.shaderSkippedCount;
    state.lastLoudness = snapshot.loudness;
    state.hasLastPumpTimestamp = true;
    state.lastPumpTimestampMs = timestampMs;

    if (isControlledSendMode (config.sendMode))
    {
        setStateStatus (state, "controlled_sent", "live_io_timer_controlled_sent");
        return makeTickResult ("controlled_sent", state.lastMessage, true);
    }

    setStateStatus (state, "pumped", "live_io_timer_dry_run_pumped");

    return makeTickResult ("pumped", state.lastMessage, true);
}

LiveIOControlTimerTickResult tickLiveIOControlTimerDryRun (
    LiveIOControlTimerState& state,
    const LiveIOControlTimerConfig& config,
    std::int64_t timestampMs,
    const AudioAnalyzerSnapshot& snapshot)
{
    auto dryRunConfig = config;
    dryRunConfig.sendMode = LiveIOControlTimerSendMode::dryRun;
    return tickLiveIOControlTimer (state, dryRunConfig, timestampMs, snapshot);
}

std::string makeLiveIOControlTimerStateJson (const LiveIOControlTimerState& state)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"liveIOControlTimerDryRunState\",\n";
    out << "  \"tickCount\": " << state.tickCount << ",\n";
    out << "  \"pumpCount\": " << state.pumpCount << ",\n";
    out << "  \"tickRateLimitedCount\": " << state.tickRateLimitedCount << ",\n";
    out << "  \"inactiveTickCount\": " << state.inactiveTickCount << ",\n";
    out << "  \"midiDryRunCount\": " << state.midiDryRunCount << ",\n";
    out << "  \"oscDryRunCount\": " << state.oscDryRunCount << ",\n";
    out << "  \"midiControlledSendCount\": " << state.midiControlledSendCount << ",\n";
    out << "  \"oscControlledSendCount\": " << state.oscControlledSendCount << ",\n";
    out << "  \"shaderSkippedCount\": " << state.shaderSkippedCount << ",\n";
    out << "  \"lastLoudness\": " << state.lastLoudness << ",\n";
    out << "  \"lastSampleCounter\": " << state.lastSampleCounter << ",\n";
    out << "  \"lastStatus\": " << jsonQuoted (state.lastStatus) << ",\n";
    out << "  \"lastMessage\": " << jsonQuoted (state.lastMessage) << ",\n";
    out << "  \"lastSendMode\": " << jsonQuoted (state.lastSendMode) << ",\n";
    out << "  \"lastPumpTimestampMs\": " << state.lastPumpTimestampMs << ",\n";
    out << "  \"errors\": ";
    appendErrorsJson (out, state.errors);
    out << "\n";
    out << "}\n";
    return out.str();
}
}
