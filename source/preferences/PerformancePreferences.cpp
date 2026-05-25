#include "PerformancePreferences.h"

#include <algorithm>
#include <cmath>

namespace myworld
{
namespace
{
float clampFloat (float value, float low, float high)
{
    return std::clamp (value, low, high);
}

int clampInt (int value, int low, int high)
{
    return std::clamp (value, low, high);
}

LiveIOSendModePreference sanitizeLiveIOSendMode (LiveIOSendModePreference sendMode)
{
    if (sendMode == LiveIOSendModePreference::controlledSend)
        return LiveIOSendModePreference::controlledSend;

    return LiveIOSendModePreference::dryRun;
}
}

PerformancePreferences makeDefaultPerformancePreferences()
{
    return {};
}

PerformancePreferences sanitizePerformancePreferences (PerformancePreferences preferences)
{
    preferences.audio.analysisGain = clampFloat (preferences.audio.analysisGain, 0.0f, 8.0f);
    preferences.midi.channel = clampInt (preferences.midi.channel, 1, 16);
    preferences.midi.loudnessCc = clampInt (preferences.midi.loudnessCc, 0, 127);
    preferences.midi.mapCc = clampInt (preferences.midi.mapCc, 0, 127);
    preferences.liveIO.sendMode = sanitizeLiveIOSendMode (preferences.liveIO.sendMode);
    return preferences;
}

std::string liveIOSendModePreferenceToString (LiveIOSendModePreference mode)
{
    if (mode == LiveIOSendModePreference::controlledSend)
        return "controlled_send";

    return "dry_run";
}

int midiValueFromNormalized (float normalizedValue)
{
    return clampInt (static_cast<int> (std::lround (clampFloat (normalizedValue, 0.0f, 1.0f) * 127.0f)), 0, 127);
}

MidiCcFrame makeLoudnessMidiCcFrame (float loudness, const PerformancePreferences& rawPreferences)
{
    const auto preferences = sanitizePerformancePreferences (rawPreferences);

    if (preferences.midi.mapModeEnabled)
    {
        return {
            true,
            preferences.midi.channel,
            preferences.midi.mapCc,
            64
        };
    }

    if (! preferences.midi.streamEnabled)
        return {};

    return {
        true,
        preferences.midi.channel,
        preferences.midi.loudnessCc,
        midiValueFromNormalized (loudness)
    };
}
}
