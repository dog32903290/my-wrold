#include "TimelineState.h"

#include <cmath>

namespace myworld
{
namespace
{
bool isPositiveFinite (double value)
{
    return std::isfinite (value) && value > 0.0;
}

bool isNonNegativeFinite (double value)
{
    return std::isfinite (value) && value >= 0.0;
}

bool isValidDirection (int direction)
{
    return direction == 1 || direction == -1;
}

double secondsPerBar (const TimelineState& timeline)
{
    const auto sanitized = sanitizedTimelineState (timeline);
    return sanitized.beatsPerBar * 60.0 / sanitized.bpm;
}

double wrapIntoLoop (double positionBars, double startBars, double endBars)
{
    const auto length = endBars - startBars;
    if (! isPositiveFinite (length))
        return startBars;

    auto offset = std::fmod (positionBars - startBars, length);
    if (offset < 0.0)
        offset += length;

    return startBars + offset;
}

void setTimelinePositionWithLoop (TimelineState& timeline, double positionBars)
{
    const auto sanitized = sanitizedTimelineState (timeline);
    if (sanitized.looping)
        timeline.positionBars = wrapIntoLoop (positionBars, sanitized.loopStartBars, sanitized.loopEndBars);
    else
        timeline.positionBars = positionBars < 0.0 ? 0.0 : positionBars;
}
}

TimelineState sanitizedTimelineState (const TimelineState& timeline)
{
    TimelineState sanitized = timeline;

    if (! isPositiveFinite (sanitized.bpm))
        sanitized.bpm = 120.0;

    if (! isPositiveFinite (sanitized.framesPerSecond))
        sanitized.framesPerSecond = 60.0;

    if (! isPositiveFinite (sanitized.beatsPerBar))
        sanitized.beatsPerBar = 4.0;

    if (! isNonNegativeFinite (sanitized.positionBars))
        sanitized.positionBars = 0.0;

    if (! isNonNegativeFinite (sanitized.loopStartBars))
        sanitized.loopStartBars = 0.0;

    if (! isNonNegativeFinite (sanitized.loopEndBars) || sanitized.loopEndBars <= sanitized.loopStartBars)
        sanitized.loopEndBars = sanitized.loopStartBars + 4.0;

    if (! isPositiveFinite (sanitized.playbackRate))
        sanitized.playbackRate = 1.0;

    if (! isValidDirection (sanitized.playbackDirection))
        sanitized.playbackDirection = 1;

    return sanitized;
}

std::string transportStateToString (TimelineTransportState state)
{
    switch (state)
    {
        case TimelineTransportState::stopped: return "stopped";
        case TimelineTransportState::playing: return "playing";
        case TimelineTransportState::paused:  return "paused";
    }

    return "stopped";
}

TimelineTransportState transportStateFromString (const std::string& state)
{
    if (state == "playing")
        return TimelineTransportState::playing;

    if (state == "paused")
        return TimelineTransportState::paused;

    return TimelineTransportState::stopped;
}

double barsToSeconds (const TimelineState& timeline, double bars)
{
    if (! std::isfinite (bars))
        return 0.0;

    return bars * secondsPerBar (timeline);
}

double secondsToBars (const TimelineState& timeline, double seconds)
{
    if (! std::isfinite (seconds))
        return 0.0;

    return seconds / secondsPerBar (timeline);
}

double barsToFrames (const TimelineState& timeline, double bars)
{
    const auto sanitized = sanitizedTimelineState (timeline);
    return barsToSeconds (sanitized, bars) * sanitized.framesPerSecond;
}

double framesToBars (const TimelineState& timeline, double frames)
{
    if (! std::isfinite (frames))
        return 0.0;

    const auto sanitized = sanitizedTimelineState (timeline);
    return secondsToBars (sanitized, frames / sanitized.framesPerSecond);
}

TimelineResult playTimeline (TimelineState& timeline, int direction, double playbackRate)
{
    if (! isValidDirection (direction))
        return { false, "timeline playback direction must be 1 or -1" };

    if (! isPositiveFinite (playbackRate))
        return { false, "timeline playbackRate must be positive" };

    timeline.transportState = TimelineTransportState::playing;
    timeline.playbackDirection = direction;
    timeline.playbackRate = playbackRate;
    return { true, "transport play" };
}

TimelineResult pauseTimeline (TimelineState& timeline)
{
    timeline.transportState = TimelineTransportState::paused;
    return { true, "transport pause" };
}

TimelineResult stopTimeline (TimelineState& timeline)
{
    timeline.transportState = TimelineTransportState::stopped;
    timeline.positionBars = 0.0;
    timeline.playbackRate = 1.0;
    timeline.playbackDirection = 1;
    return { true, "transport stop" };
}

TimelineResult stepTimelineFrames (TimelineState& timeline, double frameDelta)
{
    if (! std::isfinite (frameDelta))
        return { false, "timeline frame step must be finite" };

    setTimelinePositionWithLoop (timeline, timeline.positionBars + framesToBars (timeline, frameDelta));
    return { true, "transport step" };
}

TimelineResult advanceTimelinePlayback (TimelineState& timeline, double deltaSeconds)
{
    if (! isNonNegativeFinite (deltaSeconds))
        return { false, "timeline deltaSeconds must be non-negative" };

    auto sanitized = sanitizedTimelineState (timeline);
    if (sanitized.transportState != TimelineTransportState::playing)
    {
        timeline = sanitized;
        return { true, "transport idle" };
    }

    const auto deltaBars = secondsToBars (sanitized, deltaSeconds * sanitized.playbackRate)
                         * static_cast<double> (sanitized.playbackDirection);
    timeline = sanitized;
    setTimelinePositionWithLoop (timeline, timeline.positionBars + deltaBars);
    return { true, "transport advanced" };
}

TimelineResult setTimelineTempo (TimelineState& timeline, double bpm)
{
    if (! isPositiveFinite (bpm))
        return { false, "timeline bpm must be positive" };

    timeline.bpm = bpm;
    return { true, "set timeline tempo" };
}

TimelineResult setTimelineFramesPerSecond (TimelineState& timeline, double framesPerSecond)
{
    if (! isPositiveFinite (framesPerSecond))
        return { false, "timeline framesPerSecond must be positive" };

    timeline.framesPerSecond = framesPerSecond;
    return { true, "set timeline fps" };
}

TimelineResult setTimelinePositionBars (TimelineState& timeline, double positionBars)
{
    if (! isNonNegativeFinite (positionBars))
        return { false, "timeline positionBars must be non-negative" };

    timeline.positionBars = positionBars;
    return { true, "set timeline position" };
}

TimelineResult setTimelineLoop (TimelineState& timeline, double startBars, double endBars, bool looping)
{
    if (! isNonNegativeFinite (startBars) || ! isNonNegativeFinite (endBars) || endBars <= startBars)
        return { false, "timeline loop end must be after start" };

    timeline.loopStartBars = startBars;
    timeline.loopEndBars = endBars;
    timeline.looping = looping;
    return { true, "set timeline loop" };
}
}
