#pragma once

#include "AudioAnalyzerState.h"
#include "LiveIOControlDispatcher.h"

#include <cstdint>
#include <string>
#include <vector>

namespace myworld
{
struct LiveIOControlPumpTick
{
    std::int64_t timestampMs = 0;
    AudioAnalyzerSnapshot snapshot;
};

struct LiveIOControlPumpRequest
{
    std::vector<LiveIOControlPumpTick> ticks;
    std::vector<LiveIOBinding> bindings;
    int tickIntervalMs = 0;
    int dispatchMinIntervalMs = 0;
    LiveIOMidiOutputInventory midiOutputInventory;
    std::string midiOutputIdentifier;
    LiveIOMidiOutputSender midiSender;
    LiveIOOscFloatSender oscSender;
    std::string oscHost;
    int oscPort = 0;
    bool midiEnabled = true;
    bool oscEnabled = true;
};

struct LiveIOControlPumpTickReport
{
    std::int64_t timestampMs = 0;
    std::uint64_t sampleCounter = 0;
    double loudness = 0.0;
    bool active = false;
    bool framed = false;
    std::string status;
};

struct LiveIOControlPumpReport
{
    bool ok = false;
    std::string status;
    std::string message;
    int tickCount = 0;
    int frameCount = 0;
    int inactiveTickCount = 0;
    int tickRateLimitedCount = 0;
    double lastLoudness = 0.0;
    std::uint64_t lastSampleCounter = 0;
    std::vector<LiveIOControlPumpTickReport> ticks;
    LiveIOControlDispatchReport dispatch;
    std::vector<std::string> errors;
};

LiveIOValueFrame makeLiveIOValueFrameFromAnalyzerSnapshot (const AudioAnalyzerSnapshot& snapshot);
LiveIOControlPumpReport executeLiveIOControlPump (const LiveIOControlPumpRequest& request);
std::string makeLiveIOControlPumpReportJson (const LiveIOControlPumpReport& report);
}
