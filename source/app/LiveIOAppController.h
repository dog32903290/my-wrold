#pragma once

#include "AudioAnalyzerState.h"
#include "LiveIOControlTimer.h"
#include "LiveIOMidiTeach.h"
#include "LiveIOOscReceiver.h"
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
    bool oscReceiverOpen = false;
    bool oscReceived = false;
    int oscReceiverPort = 0;
    std::string oscReceiverStatus;
    std::string oscReceiverMessage;
    LiveIOValueFrame oscFrame;
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
    LiveIOOscReceiverConfig makeOscReceiverConfig (const LiveIOPreferences& preferences) const;
    LiveIOOscReceiverOpenResult ensureOscReceiverOpen (const LiveIOOscReceiverConfig& config);

    LiveIOControlTimerSendMode liveIOSendMode = LiveIOControlTimerSendMode::dryRun;
    LiveIOControlTimerState timerState;
    LiveIOMidiTeachState midiTeachState;
    LiveIOOscReceiver oscReceiver;
    int midiTeachInputCount = 0;
};
}
