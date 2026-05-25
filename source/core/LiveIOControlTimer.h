#pragma once

#include "AudioAnalyzerState.h"
#include "LiveIOControlPump.h"

#include <cstdint>
#include <string>
#include <vector>

namespace myworld
{
struct LiveIOControlTimerDryRunConfig
{
    std::vector<LiveIOBinding> bindings;
    int tickIntervalMs = 50;
    int dispatchMinIntervalMs = 0;
    bool enabled = true;
};

struct LiveIOControlTimerState
{
    int tickCount = 0;
    int pumpCount = 0;
    int tickRateLimitedCount = 0;
    int inactiveTickCount = 0;
    int midiDryRunCount = 0;
    int oscDryRunCount = 0;
    int shaderSkippedCount = 0;
    double lastLoudness = 0.0;
    std::uint64_t lastSampleCounter = 0;
    std::string lastStatus = "idle";
    std::string lastMessage;
    bool hasLastPumpTimestamp = false;
    std::int64_t lastPumpTimestampMs = 0;
    LiveIOControlPumpReport lastPumpReport;
    std::vector<std::string> errors;
};

struct LiveIOControlTimerTickResult
{
    bool ok = false;
    std::string status;
    std::string message;
    bool pumped = false;
};

LiveIOControlTimerTickResult tickLiveIOControlTimerDryRun (
    LiveIOControlTimerState& state,
    const LiveIOControlTimerDryRunConfig& config,
    std::int64_t timestampMs,
    const AudioAnalyzerSnapshot& snapshot);

std::string makeLiveIOControlTimerStateJson (const LiveIOControlTimerState& state);
}
