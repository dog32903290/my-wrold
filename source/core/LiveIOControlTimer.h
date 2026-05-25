#pragma once

#include "AudioAnalyzerState.h"
#include "LiveIOControlPump.h"

#include <cstdint>
#include <string>
#include <vector>

namespace myworld
{
enum class LiveIOControlTimerSendMode
{
    dryRun,
    controlledSend
};

struct LiveIOControlTimerConfig
{
    std::vector<LiveIOBinding> bindings;
    int tickIntervalMs = 50;
    int dispatchMinIntervalMs = 0;
    bool enabled = true;
    LiveIOControlTimerSendMode sendMode = LiveIOControlTimerSendMode::dryRun;
    LiveIOMidiOutputInventory midiOutputInventory;
    std::string midiOutputIdentifier;
    LiveIOMidiOutputSender midiSender;
    LiveIOOscFloatSender oscSender;
    bool midiEnabled = true;
    bool oscEnabled = true;
};

using LiveIOControlTimerDryRunConfig = LiveIOControlTimerConfig;

struct LiveIOControlTimerState
{
    int tickCount = 0;
    int pumpCount = 0;
    int tickRateLimitedCount = 0;
    int inactiveTickCount = 0;
    int midiDryRunCount = 0;
    int oscDryRunCount = 0;
    int midiControlledSendCount = 0;
    int oscControlledSendCount = 0;
    int shaderSkippedCount = 0;
    double lastLoudness = 0.0;
    std::uint64_t lastSampleCounter = 0;
    std::string lastStatus = "idle";
    std::string lastMessage;
    std::string lastSendMode = "dry_run";
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

std::string liveIOControlTimerSendModeToString (LiveIOControlTimerSendMode mode);

LiveIOControlTimerTickResult tickLiveIOControlTimer (
    LiveIOControlTimerState& state,
    const LiveIOControlTimerConfig& config,
    std::int64_t timestampMs,
    const AudioAnalyzerSnapshot& snapshot);

LiveIOControlTimerTickResult tickLiveIOControlTimerDryRun (
    LiveIOControlTimerState& state,
    const LiveIOControlTimerConfig& config,
    std::int64_t timestampMs,
    const AudioAnalyzerSnapshot& snapshot);

std::string makeLiveIOControlTimerStateJson (const LiveIOControlTimerState& state);
}
