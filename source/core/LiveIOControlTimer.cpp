#include "LiveIOControlTimer.h"

#include <iomanip>
#include <sstream>

namespace myworld
{
namespace
{
constexpr const char* dryRunMidiIdentifier = "live-io-timer-dry-run";

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

LiveIOControlTimerTickResult tickLiveIOControlTimerDryRun (
    LiveIOControlTimerState& state,
    const LiveIOControlTimerDryRunConfig& config,
    std::int64_t timestampMs,
    const AudioAnalyzerSnapshot& snapshot)
{
    ++state.tickCount;
    state.lastSampleCounter = snapshot.sampleCounter;

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
    request.midiOutputInventory = makeDryRunMidiInventory();
    request.midiOutputIdentifier = dryRunMidiIdentifier;
    request.midiSender = [] (const LiveIOMidiOutputDevice&, const LiveIOMidiCcMessage&)
    {
        return LiveIOMidiOutputDeviceSendResult { true, true, "" };
    };
    request.oscSender = [] (const LiveIOOscFloatMessage&)
    {
        return LiveIOOscFloatSendResult { true, "" };
    };

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
    state.midiDryRunCount += report.dispatch.midiSentCount;
    state.oscDryRunCount += report.dispatch.oscSentCount;
    state.shaderSkippedCount += report.dispatch.shaderSkippedCount;
    state.lastLoudness = snapshot.loudness;
    state.hasLastPumpTimestamp = true;
    state.lastPumpTimestampMs = timestampMs;
    setStateStatus (state, "pumped", "live_io_timer_dry_run_pumped");

    return makeTickResult ("pumped", state.lastMessage, true);
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
    out << "  \"shaderSkippedCount\": " << state.shaderSkippedCount << ",\n";
    out << "  \"lastLoudness\": " << state.lastLoudness << ",\n";
    out << "  \"lastSampleCounter\": " << state.lastSampleCounter << ",\n";
    out << "  \"lastStatus\": " << jsonQuoted (state.lastStatus) << ",\n";
    out << "  \"lastMessage\": " << jsonQuoted (state.lastMessage) << ",\n";
    out << "  \"lastPumpTimestampMs\": " << state.lastPumpTimestampMs << ",\n";
    out << "  \"errors\": ";
    appendErrorsJson (out, state.errors);
    out << "\n";
    out << "}\n";
    return out.str();
}
}
