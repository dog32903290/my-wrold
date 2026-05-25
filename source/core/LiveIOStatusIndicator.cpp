#include "LiveIOStatusIndicator.h"

#include <sstream>

namespace myworld
{
namespace
{
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

std::string modeLabel (LiveIOControlTimerSendMode sendMode)
{
    if (sendMode == LiveIOControlTimerSendMode::controlledSend)
        return "send";

    return "dry";
}

std::string toneForStatus (const std::string& status, LiveIOControlTimerSendMode sendMode)
{
    if (status == "failed")
        return "failed";

    if (status == "inactive")
        return "inactive";

    if (status == "disabled")
        return "disabled";

    if (status == "tick_rate_limited")
        return "limited";

    if (sendMode == LiveIOControlTimerSendMode::controlledSend && status == "controlled_sent")
        return "sending";

    if (sendMode == LiveIOControlTimerSendMode::dryRun && status == "pumped")
        return "dry_run";

    return "idle";
}

int midiCountForMode (const LiveIOControlTimerState& state, LiveIOControlTimerSendMode sendMode)
{
    if (sendMode == LiveIOControlTimerSendMode::controlledSend)
        return state.midiControlledSendCount;

    return state.midiDryRunCount;
}

int oscCountForMode (const LiveIOControlTimerState& state, LiveIOControlTimerSendMode sendMode)
{
    if (sendMode == LiveIOControlTimerSendMode::controlledSend)
        return state.oscControlledSendCount;

    return state.oscDryRunCount;
}
}

LiveIOStatusIndicatorState makeLiveIOStatusIndicatorState (
    const LiveIOControlTimerState& state,
    LiveIOControlTimerSendMode sendMode)
{
    LiveIOStatusIndicatorState indicator;
    indicator.mode = liveIOControlTimerSendModeToString (sendMode);
    indicator.status = state.lastStatus.empty() ? "idle" : state.lastStatus;
    indicator.detail = state.lastMessage;
    indicator.tone = toneForStatus (indicator.status, sendMode);
    indicator.midiCount = midiCountForMode (state, sendMode);
    indicator.oscCount = oscCountForMode (state, sendMode);
    indicator.sampleCounter = state.lastSampleCounter;

    std::ostringstream text;
    text << "live io " << modeLabel (sendMode)
         << " " << indicator.status
         << " m" << indicator.midiCount
         << " o" << indicator.oscCount;
    indicator.text = text.str();

    return indicator;
}

LiveIOStatusIndicatorState withLiveIOOutputOperator (
    LiveIOStatusIndicatorState state,
    const std::string& outputOperator)
{
    state.outputOperator = outputOperator;

    if (! state.outputOperator.empty())
    {
        std::ostringstream text;
        text << state.text << " op " << state.outputOperator;
        state.text = text.str();
    }

    return state;
}

LiveIOStatusIndicatorState withLiveIORealtimeTelemetry (
    LiveIOStatusIndicatorState state,
    const LiveIORealtimeIndicatorTelemetry& telemetry)
{
    state.realtimeStatus = telemetry.status;
    state.realtimeSequence = telemetry.sequence;
    state.realtimeDroppedSnapshots = telemetry.droppedSnapshots;
    state.realtimeSkippedSnapshots = telemetry.skippedSnapshots;
    state.realtimeOverwrittenSnapshots = telemetry.overwrittenSnapshots;

    if (! state.realtimeStatus.empty())
    {
        std::ostringstream text;
        text << state.text
             << " rt " << state.realtimeStatus
             << " s" << state.realtimeSequence
             << " skip" << state.realtimeSkippedSnapshots
             << " over" << state.realtimeOverwrittenSnapshots;
        state.text = text.str();
    }

    return state;
}

std::string makeLiveIOStatusIndicatorJson (const LiveIOStatusIndicatorState& state)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"liveIOStatusIndicator\",\n";
    out << "  \"text\": " << jsonQuoted (state.text) << ",\n";
    out << "  \"detail\": " << jsonQuoted (state.detail) << ",\n";
    out << "  \"mode\": " << jsonQuoted (state.mode) << ",\n";
    out << "  \"status\": " << jsonQuoted (state.status) << ",\n";
    out << "  \"tone\": " << jsonQuoted (state.tone) << ",\n";
    out << "  \"outputOperator\": " << jsonQuoted (state.outputOperator) << ",\n";
    out << "  \"midiCount\": " << state.midiCount << ",\n";
    out << "  \"oscCount\": " << state.oscCount << ",\n";
    out << "  \"sampleCounter\": " << state.sampleCounter << ",\n";
    out << "  \"realtimeStatus\": " << jsonQuoted (state.realtimeStatus) << ",\n";
    out << "  \"realtimeSequence\": " << state.realtimeSequence << ",\n";
    out << "  \"realtimeDroppedSnapshots\": " << state.realtimeDroppedSnapshots << ",\n";
    out << "  \"realtimeSkippedSnapshots\": " << state.realtimeSkippedSnapshots << ",\n";
    out << "  \"realtimeOverwrittenSnapshots\": " << state.realtimeOverwrittenSnapshots << "\n";
    out << "}\n";
    return out.str();
}
}
