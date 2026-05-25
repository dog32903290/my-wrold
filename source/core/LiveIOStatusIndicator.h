#pragma once

#include "LiveIOControlTimer.h"

#include <cstdint>
#include <string>

namespace myworld
{
struct LiveIOStatusIndicatorState
{
    std::string text;
    std::string detail;
    std::string mode;
    std::string status;
    std::string tone;
    std::string outputOperator;
    int midiCount = 0;
    int oscCount = 0;
    std::uint64_t sampleCounter = 0;
    std::string realtimeStatus;
    std::uint64_t realtimeSequence = 0;
    std::uint64_t realtimeDroppedSnapshots = 0;
    std::uint64_t realtimeSkippedSnapshots = 0;
    std::uint64_t realtimeOverwrittenSnapshots = 0;
};

struct LiveIORealtimeIndicatorTelemetry
{
    std::string status;
    std::uint64_t sequence = 0;
    std::uint64_t droppedSnapshots = 0;
    std::uint64_t skippedSnapshots = 0;
    std::uint64_t overwrittenSnapshots = 0;
};

LiveIOStatusIndicatorState makeLiveIOStatusIndicatorState (
    const LiveIOControlTimerState& state,
    LiveIOControlTimerSendMode sendMode);

LiveIOStatusIndicatorState withLiveIOOutputOperator (
    LiveIOStatusIndicatorState state,
    const std::string& outputOperator);

LiveIOStatusIndicatorState withLiveIORealtimeTelemetry (
    LiveIOStatusIndicatorState state,
    const LiveIORealtimeIndicatorTelemetry& telemetry);

std::string makeLiveIOStatusIndicatorJson (const LiveIOStatusIndicatorState& state);
}
