#include "PerformancePreferences.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <unordered_map>

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

std::string sanitizeOscHost (const std::string& host)
{
    return host.empty() ? "127.0.0.1" : host;
}

std::string sanitizeOscAddress (const std::string& address)
{
    if (! address.empty() && address.front() == '/')
        return address;

    return "/my-world/loudness";
}

std::string boolToText (bool value)
{
    return value ? "true" : "false";
}

bool boolFromText (const std::string& text)
{
    return text == "true" || text == "1";
}

int intFromText (const std::string& text, int fallback)
{
    try
    {
        return std::stoi (text);
    }
    catch (...)
    {
        return fallback;
    }
}

float floatFromText (const std::string& text, float fallback)
{
    try
    {
        return std::stof (text);
    }
    catch (...)
    {
        return fallback;
    }
}

std::unordered_map<std::string, std::string> readKeyValueFile (std::istream& input)
{
    std::unordered_map<std::string, std::string> values;
    std::string line;

    while (std::getline (input, line))
    {
        const auto separator = line.find ('=');

        if (separator == std::string::npos)
            continue;

        values[line.substr (0, separator)] = line.substr (separator + 1);
    }

    return values;
}

std::string valueOr (const std::unordered_map<std::string, std::string>& values,
                     const std::string& key,
                     const std::string& fallback)
{
    const auto found = values.find (key);
    return found == values.end() ? fallback : found->second;
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
    preferences.liveIO.oscHost = sanitizeOscHost (preferences.liveIO.oscHost);
    preferences.liveIO.oscPort = clampInt (preferences.liveIO.oscPort, 1, 65535);
    preferences.liveIO.oscLoudnessAddress = sanitizeOscAddress (preferences.liveIO.oscLoudnessAddress);
    return preferences;
}

std::string liveIOSendModePreferenceToString (LiveIOSendModePreference mode)
{
    if (mode == LiveIOSendModePreference::controlledSend)
        return "controlled_send";

    return "dry_run";
}

LiveIOSendModePreference liveIOSendModePreferenceFromString (const std::string& text)
{
    if (text == "controlled_send")
        return LiveIOSendModePreference::controlledSend;

    return LiveIOSendModePreference::dryRun;
}

std::vector<std::string> midiTeachInputIdentifiers (const PerformancePreferences& rawPreferences,
                                                    const std::vector<std::string>& availableIdentifiers)
{
    const auto preferences = sanitizePerformancePreferences (rawPreferences);

    if (preferences.midi.inputIdentifier.empty())
        return availableIdentifiers;

    if (std::find (availableIdentifiers.begin(),
                   availableIdentifiers.end(),
                   preferences.midi.inputIdentifier) != availableIdentifiers.end())
        return { preferences.midi.inputIdentifier };

    return {};
}

PerformancePreferencesSaveResult savePerformancePreferences (const std::filesystem::path& path,
                                                             const PerformancePreferences& rawPreferences)
{
    std::error_code error;
    std::filesystem::create_directories (path.parent_path(), error);

    if (error)
        return { false, "create_directory_failed: " + error.message() };

    const auto preferences = sanitizePerformancePreferences (rawPreferences);
    std::ofstream output (path);

    if (! output)
        return { false, "open_failed" };

    output << "audio.analysisGain=" << preferences.audio.analysisGain << "\n";
    output << "audio.meterRowsVisible=" << boolToText (preferences.audio.meterRowsVisible) << "\n";
    output << "midi.streamEnabled=" << boolToText (preferences.midi.streamEnabled) << "\n";
    output << "midi.mapModeEnabled=" << boolToText (preferences.midi.mapModeEnabled) << "\n";
    output << "midi.channel=" << preferences.midi.channel << "\n";
    output << "midi.loudnessCc=" << preferences.midi.loudnessCc << "\n";
    output << "midi.mapCc=" << preferences.midi.mapCc << "\n";
    output << "midi.inputIdentifier=" << preferences.midi.inputIdentifier << "\n";
    output << "midi.inputName=" << preferences.midi.inputName << "\n";
    output << "midi.outputIdentifier=" << preferences.midi.outputIdentifier << "\n";
    output << "midi.outputName=" << preferences.midi.outputName << "\n";
    output << "liveIO.sendMode=" << liveIOSendModePreferenceToString (preferences.liveIO.sendMode) << "\n";
    output << "liveIO.oscHost=" << preferences.liveIO.oscHost << "\n";
    output << "liveIO.oscPort=" << preferences.liveIO.oscPort << "\n";
    output << "liveIO.oscLoudnessAddress=" << preferences.liveIO.oscLoudnessAddress << "\n";

    if (! output)
        return { false, "write_failed" };

    return { true, "saved" };
}

PerformancePreferencesStoreResult loadPerformancePreferences (const std::filesystem::path& path)
{
    PerformancePreferencesStoreResult result;
    result.preferences = makeDefaultPerformancePreferences();

    if (! std::filesystem::exists (path))
    {
        result.ok = true;
        result.status = "default";
        return result;
    }

    std::ifstream input (path);

    if (! input)
    {
        result.status = "open_failed";
        return result;
    }

    const auto values = readKeyValueFile (input);
    auto preferences = makeDefaultPerformancePreferences();

    preferences.audio.analysisGain = floatFromText (
        valueOr (values, "audio.analysisGain", std::to_string (preferences.audio.analysisGain)),
        preferences.audio.analysisGain);
    preferences.audio.meterRowsVisible = boolFromText (
        valueOr (values, "audio.meterRowsVisible", boolToText (preferences.audio.meterRowsVisible)));
    preferences.midi.streamEnabled = boolFromText (
        valueOr (values, "midi.streamEnabled", boolToText (preferences.midi.streamEnabled)));
    preferences.midi.mapModeEnabled = boolFromText (
        valueOr (values, "midi.mapModeEnabled", boolToText (preferences.midi.mapModeEnabled)));
    preferences.midi.channel = intFromText (
        valueOr (values, "midi.channel", std::to_string (preferences.midi.channel)),
        preferences.midi.channel);
    preferences.midi.loudnessCc = intFromText (
        valueOr (values, "midi.loudnessCc", std::to_string (preferences.midi.loudnessCc)),
        preferences.midi.loudnessCc);
    preferences.midi.mapCc = intFromText (
        valueOr (values, "midi.mapCc", std::to_string (preferences.midi.mapCc)),
        preferences.midi.mapCc);
    preferences.midi.inputIdentifier = valueOr (values, "midi.inputIdentifier", preferences.midi.inputIdentifier);
    preferences.midi.inputName = valueOr (values, "midi.inputName", preferences.midi.inputName);
    preferences.midi.outputIdentifier = valueOr (values, "midi.outputIdentifier", preferences.midi.outputIdentifier);
    preferences.midi.outputName = valueOr (values, "midi.outputName", preferences.midi.outputName);
    preferences.liveIO.sendMode = liveIOSendModePreferenceFromString (
        valueOr (values, "liveIO.sendMode", liveIOSendModePreferenceToString (preferences.liveIO.sendMode)));
    preferences.liveIO.oscHost = valueOr (values, "liveIO.oscHost", preferences.liveIO.oscHost);
    preferences.liveIO.oscPort = intFromText (
        valueOr (values, "liveIO.oscPort", std::to_string (preferences.liveIO.oscPort)),
        preferences.liveIO.oscPort);
    preferences.liveIO.oscLoudnessAddress = valueOr (
        values,
        "liveIO.oscLoudnessAddress",
        preferences.liveIO.oscLoudnessAddress);

    result.ok = true;
    result.status = "loaded";
    result.preferences = sanitizePerformancePreferences (preferences);
    return result;
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
