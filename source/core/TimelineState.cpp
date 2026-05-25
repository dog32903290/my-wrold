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

double secondsPerBar (const TimelineState& timeline)
{
    const auto sanitized = sanitizedTimelineState (timeline);
    return sanitized.beatsPerBar * 60.0 / sanitized.bpm;
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

    return sanitized;
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
