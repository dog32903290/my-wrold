#pragma once

#include <string>
#include <vector>
#include <filesystem>

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
    std::string inputIdentifier;
    std::string inputName;
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

struct PerformancePreferencesStoreResult
{
    bool ok = false;
    std::string status;
    PerformancePreferences preferences;
};

struct PerformancePreferencesSaveResult
{
    bool ok = false;
    std::string status;
};

PerformancePreferences makeDefaultPerformancePreferences();
PerformancePreferences sanitizePerformancePreferences (PerformancePreferences preferences);
std::string liveIOSendModePreferenceToString (LiveIOSendModePreference mode);
LiveIOSendModePreference liveIOSendModePreferenceFromString (const std::string& text);
std::vector<std::string> midiTeachInputIdentifiers (const PerformancePreferences& preferences,
                                                    const std::vector<std::string>& availableIdentifiers);
PerformancePreferencesSaveResult savePerformancePreferences (const std::filesystem::path& path,
                                                             const PerformancePreferences& preferences);
PerformancePreferencesStoreResult loadPerformancePreferences (const std::filesystem::path& path);
int midiValueFromNormalized (float normalizedValue);
MidiCcFrame makeLoudnessMidiCcFrame (float loudness, const PerformancePreferences& preferences);
}
