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
    bool hasShaderUniform = false;
    std::string shaderUniformBindingId;
    std::string shaderUniformName;
    double shaderUniformValue = 0.0;
    std::uint64_t shaderUniformSampleCounter = 0;
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

LiveIOStatusIndicatorState withLiveIOShaderUniformEvidence (
    LiveIOStatusIndicatorState state,
    const std::string& bindingId,
    const std::string& uniformName,
    double value,
    std::uint64_t sampleCounter);

LiveIOStatusIndicatorState withLiveIORealtimeTelemetry (
    LiveIOStatusIndicatorState state,
    const LiveIORealtimeIndicatorTelemetry& telemetry);

std::string makeLiveIOStatusIndicatorJson (const LiveIOStatusIndicatorState& state);
}
