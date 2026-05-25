#pragma once

#include <string>

namespace myworld
{
struct TimelineState
{
    double bpm = 120.0;
    double framesPerSecond = 60.0;
    double beatsPerBar = 4.0;
    double positionBars = 0.0;
    double loopStartBars = 0.0;
    double loopEndBars = 4.0;
    bool looping = false;
};

struct TimelineResult
{
    bool ok = false;
    std::string message;
};

TimelineState sanitizedTimelineState (const TimelineState& timeline);

double barsToSeconds (const TimelineState& timeline, double bars);
double secondsToBars (const TimelineState& timeline, double seconds);
double barsToFrames (const TimelineState& timeline, double bars);
double framesToBars (const TimelineState& timeline, double frames);

TimelineResult setTimelineTempo (TimelineState& timeline, double bpm);
TimelineResult setTimelineFramesPerSecond (TimelineState& timeline, double framesPerSecond);
TimelineResult setTimelinePositionBars (TimelineState& timeline, double positionBars);
TimelineResult setTimelineLoop (TimelineState& timeline, double startBars, double endBars, bool looping);
}
