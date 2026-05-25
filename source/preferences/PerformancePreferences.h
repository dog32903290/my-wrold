#pragma once

#include <string>

namespace myworld
{
struct AudioPreferences
{
    float analysisGain = 1.0f;
    bool meterRowsVisible = true;
};

struct MidiPreferences
{
    bool streamEnabled = false;
    bool mapModeEnabled = false;
    int channel = 1;
    int loudnessCc = 20;
    int mapCc = 20;
    std::string outputIdentifier;
    std::string outputName;
};

enum class LiveIOSendModePreference
{
    dryRun,
    controlledSend
};

struct LiveIOPreferences
{
    LiveIOSendModePreference sendMode = LiveIOSendModePreference::dryRun;
};

struct PerformancePreferences
{
    AudioPreferences audio;
    MidiPreferences midi;
    LiveIOPreferences liveIO;
};

struct MidiCcFrame
{
    bool shouldSend = false;
    int channel = 1;
    int cc = 20;
    int value = 0;
};

PerformancePreferences makeDefaultPerformancePreferences();
PerformancePreferences sanitizePerformancePreferences (PerformancePreferences preferences);
std::string liveIOSendModePreferenceToString (LiveIOSendModePreference mode);
int midiValueFromNormalized (float normalizedValue);
MidiCcFrame makeLoudnessMidiCcFrame (float loudness, const PerformancePreferences& preferences);
}
