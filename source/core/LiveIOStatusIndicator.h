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
    int midiCount = 0;
    int oscCount = 0;
    std::uint64_t sampleCounter = 0;
};

LiveIOStatusIndicatorState makeLiveIOStatusIndicatorState (
    const LiveIOControlTimerState& state,
    LiveIOControlTimerSendMode sendMode);

std::string makeLiveIOStatusIndicatorJson (const LiveIOStatusIndicatorState& state);
}
