#include "LiveIOMidiTeach.h"

#include <algorithm>
#include <sstream>

namespace myworld
{
namespace
{
int clampInt (int value, int low, int high)
{
    return std::clamp (value, low, high);
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

bool targetIsLearnable (LiveIOMidiTeachTarget target)
{
    return target == LiveIOMidiTeachTarget::loudnessCc
        || target == LiveIOMidiTeachTarget::mapCc;
}

LiveIOMidiTeachResult makeResult (bool ok,
                                  bool consumed,
                                  bool learned,
                                  LiveIOMidiTeachTarget target,
                                  const std::string& status,
                                  const std::string& message,
                                  int channel,
                                  int cc)
{
    return {
        ok,
        consumed,
        learned,
        target,
        status,
        message,
        channel,
        cc
    };
}
}

std::string liveIOMidiTeachTargetToString (LiveIOMidiTeachTarget target)
{
    if (target == LiveIOMidiTeachTarget::loudnessCc)
        return "loudness_cc";

    if (target == LiveIOMidiTeachTarget::mapCc)
        return "map_cc";

    return "none";
}

LiveIOMidiTeachIncomingMessage makeLiveIOMidiTeachControlChange (int channel, int cc, int value)
{
    return {
        true,
        channel,
        cc,
        value
    };
}

LiveIOMidiTeachResult armLiveIOMidiTeach (LiveIOMidiTeachState& state,
                                          LiveIOMidiTeachTarget target)
{
    if (! targetIsLearnable (target))
    {
        state.armed = false;
        state.target = LiveIOMidiTeachTarget::none;
        state.status = "failed";
        state.message = "midi teach target is required";
        return makeResult (false, false, false, target, state.status, state.message, 1, 0);
    }

    state.armed = true;
    state.target = target;
    state.status = "armed";
    state.message = "midi_teach_armed:" + liveIOMidiTeachTargetToString (target);

    return makeResult (true, false, false, target, state.status, state.message, state.learnedChannel, state.learnedCc);
}

LiveIOMidiTeachResult cancelLiveIOMidiTeach (LiveIOMidiTeachState& state)
{
    state.armed = false;
    state.target = LiveIOMidiTeachTarget::none;
    state.status = "cancelled";
    state.message = "midi_teach_cancelled";

    return makeResult (
        true,
        false,
        false,
        LiveIOMidiTeachTarget::none,
        state.status,
        state.message,
        state.learnedChannel,
        state.learnedCc);
}

LiveIOMidiTeachResult handleLiveIOMidiTeachMessage (
    LiveIOMidiTeachState& state,
    const LiveIOMidiTeachIncomingMessage& message)
{
    if (! state.armed)
    {
        state.status = "idle";
        state.message = "midi_teach_idle";
        return makeResult (
            true,
            false,
            false,
            LiveIOMidiTeachTarget::none,
            state.status,
            state.message,
            state.learnedChannel,
            state.learnedCc);
    }

    if (! message.isControlChange)
    {
        ++state.ignoredMessageCount;
        state.status = "ignored";
        state.message = "midi_teach_waiting_for_cc";
        return makeResult (
            true,
            false,
            false,
            state.target,
            state.status,
            state.message,
            state.learnedChannel,
            state.learnedCc);
    }

    const auto learnedTarget = state.target;
    const auto channel = clampInt (message.channel, 1, 16);
    const auto cc = clampInt (message.cc, 0, 127);

    state.armed = false;
    state.target = LiveIOMidiTeachTarget::none;
    state.lastLearnedTarget = learnedTarget;
    state.learnedChannel = channel;
    state.learnedCc = cc;
    ++state.learnedMessageCount;
    state.status = "learned";
    state.message = "midi_teach_learned:" + liveIOMidiTeachTargetToString (learnedTarget);

    return makeResult (
        true,
        true,
        true,
        learnedTarget,
        state.status,
        state.message,
        channel,
        cc);
}

std::string makeLiveIOMidiTeachStateJson (const LiveIOMidiTeachState& state)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"liveIOMidiTeachState\",\n";
    out << "  \"armed\": " << (state.armed ? "true" : "false") << ",\n";
    out << "  \"target\": " << jsonQuoted (liveIOMidiTeachTargetToString (state.target)) << ",\n";
    out << "  \"lastLearnedTarget\": " << jsonQuoted (liveIOMidiTeachTargetToString (state.lastLearnedTarget)) << ",\n";
    out << "  \"status\": " << jsonQuoted (state.status) << ",\n";
    out << "  \"message\": " << jsonQuoted (state.message) << ",\n";
    out << "  \"learnedChannel\": " << state.learnedChannel << ",\n";
    out << "  \"learnedCc\": " << state.learnedCc << ",\n";
    out << "  \"ignoredMessageCount\": " << state.ignoredMessageCount << ",\n";
    out << "  \"learnedMessageCount\": " << state.learnedMessageCount << "\n";
    out << "}\n";
    return out.str();
}
}
