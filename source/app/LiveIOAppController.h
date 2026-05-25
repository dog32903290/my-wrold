#pragma once

#include "AudioAnalyzerState.h"
#include "LiveIOControlTimer.h"
#include "LiveIOMidiTeach.h"
#include "LiveIOStatusIndicator.h"
#include "PerformancePreferences.h"

#include <cstdint>
#include <string>

namespace myworld
{
struct LiveIOAppTimerRequest
{
    AudioAnalyzerSnapshot snapshot;
    PerformancePreferences preferences;
    std::int64_t timestampMs = 0;
    LiveIOMidiOutputSender midiSender;
    LiveIOOscFloatSender oscSender;
};

struct LiveIOAppTimerResult
{
    LiveIOControlTimerTickResult tick;
    LiveIOStatusIndicatorState indicator;
};

struct LiveIOAppMidiTeachView
{
    bool ok = false;
    bool shouldListen = false;
    bool learned = false;
    LiveIOMidiTeachTarget learnedTarget = LiveIOMidiTeachTarget::none;
    int learnedChannel = 1;
    int learnedCc = 20;
    std::string statusText;
};

class LiveIOAppController
{
public:
    void applyLiveIOPreferences (LiveIOPreferences preferences);
    LiveIOControlTimerSendMode getSendMode() const;

    LiveIOAppTimerResult tick (const LiveIOAppTimerRequest& request);

    LiveIOAppMidiTeachView armMidiTeach (LiveIOMidiTeachTarget target, int inputCount);
    LiveIOAppMidiTeachView cancelMidiTeach();
    LiveIOAppMidiTeachView handleMidiTeachMessage (const LiveIOMidiTeachIncomingMessage& message);

private:
    LiveIOControlTimerSendMode liveIOSendMode = LiveIOControlTimerSendMode::dryRun;
    LiveIOControlTimerState timerState;
    LiveIOMidiTeachState midiTeachState;
    int midiTeachInputCount = 0;
};
}
