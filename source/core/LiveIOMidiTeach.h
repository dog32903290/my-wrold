#pragma once

#include "LiveIOBus.h"

#include <string>
#include <vector>

namespace myworld
{
enum class LiveIOMidiTeachTarget
{
    none,
    loudnessCc,
    mapCc,
    bindingMidiCc
};

struct LiveIOMidiTeachIncomingMessage
{
    bool isControlChange = false;
    int channel = 1;
    int cc = 0;
    int value = 0;
};

struct LiveIOMidiTeachState
{
    bool armed = false;
    LiveIOMidiTeachTarget target = LiveIOMidiTeachTarget::none;
    LiveIOMidiTeachTarget lastLearnedTarget = LiveIOMidiTeachTarget::none;
    std::string bindingId;
    std::string lastLearnedBindingId;
    std::string status = "idle";
    std::string message;
    int learnedChannel = 1;
    int learnedCc = 20;
    int ignoredMessageCount = 0;
    int learnedMessageCount = 0;
};

struct LiveIOMidiTeachResult
{
    bool ok = false;
    bool consumed = false;
    bool learned = false;
    LiveIOMidiTeachTarget target = LiveIOMidiTeachTarget::none;
    std::string bindingId;
    std::string status;
    std::string message;
    int channel = 1;
    int cc = 0;
};

struct LiveIOMidiTeachBindingApplyResult
{
    bool ok = false;
    std::string status;
    std::string message;
};

std::string liveIOMidiTeachTargetToString (LiveIOMidiTeachTarget target);

LiveIOMidiTeachIncomingMessage makeLiveIOMidiTeachControlChange (int channel, int cc, int value);

LiveIOMidiTeachResult armLiveIOMidiTeach (LiveIOMidiTeachState& state,
                                          LiveIOMidiTeachTarget target);

LiveIOMidiTeachResult armLiveIOMidiTeachForBinding (LiveIOMidiTeachState& state,
                                                    const std::string& bindingId);

LiveIOMidiTeachResult cancelLiveIOMidiTeach (LiveIOMidiTeachState& state);

LiveIOMidiTeachResult handleLiveIOMidiTeachMessage (
    LiveIOMidiTeachState& state,
    const LiveIOMidiTeachIncomingMessage& message);

LiveIOMidiTeachBindingApplyResult applyLiveIOMidiTeachToBindings (
    std::vector<LiveIOBinding>& bindings,
    const LiveIOMidiTeachResult& result);

std::string makeLiveIOMidiTeachStateJson (const LiveIOMidiTeachState& state);
}
