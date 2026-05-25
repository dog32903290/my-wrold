#pragma once

#include <string>

namespace myworld
{
enum class TimelineTransportState
{
    stopped,
    playing,
    paused
};

struct TimelineState
{
    double bpm = 120.0;
    double framesPerSecond = 60.0;
    double beatsPerBar = 4.0;
    double positionBars = 0.0;
    double loopStartBars = 0.0;
    double loopEndBars = 4.0;
    bool looping = false;
    TimelineTransportState transportState = TimelineTransportState::stopped;
    double playbackRate = 1.0;
    int playbackDirection = 1;
};

struct TimelineResult
{
    bool ok = false;
    std::string message;
};

TimelineState sanitizedTimelineState (const TimelineState& timeline);
std::string transportStateToString (TimelineTransportState state);
TimelineTransportState transportStateFromString (const std::string& state);

double barsToSeconds (const TimelineState& timeline, double bars);
double secondsToBars (const TimelineState& timeline, double seconds);
double barsToFrames (const TimelineState& timeline, double bars);
double framesToBars (const TimelineState& timeline, double frames);

TimelineResult playTimeline (TimelineState& timeline, int direction, double playbackRate);
TimelineResult pauseTimeline (TimelineState& timeline);
TimelineResult stopTimeline (TimelineState& timeline);
TimelineResult stepTimelineFrames (TimelineState& timeline, double frameDelta);
TimelineResult advanceTimelinePlayback (TimelineState& timeline, double deltaSeconds);
TimelineResult setTimelineTempo (TimelineState& timeline, double bpm);
TimelineResult setTimelineFramesPerSecond (TimelineState& timeline, double framesPerSecond);
TimelineResult setTimelinePositionBars (TimelineState& timeline, double positionBars);
TimelineResult setTimelineLoop (TimelineState& timeline, double startBars, double endBars, bool looping);
}
