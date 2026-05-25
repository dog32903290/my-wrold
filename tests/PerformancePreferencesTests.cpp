#include "PerformancePreferences.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}

void expectEqual (int actual, int expected, const std::string& message)
{
    expect (actual == expected,
            message + " expected " + std::to_string (expected) + " got " + std::to_string (actual));
}

void expectEqual (const std::string& actual, const std::string& expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + expected + " got " + actual);
}
}

int main()
{
    auto preferences = myworld::makeDefaultPerformancePreferences();

    expect (preferences.audio.analysisGain == 1.0f, "default analysis gain");
    expect (preferences.audio.meterRowsVisible, "meter rows visible by default");
    expect (! preferences.midi.streamEnabled, "midi stream is opt-in");
    expect (! preferences.midi.mapModeEnabled, "map mode is opt-in");
    expectEqual (preferences.midi.channel, 1, "default midi channel");
    expectEqual (preferences.midi.loudnessCc, 20, "default loudness cc follows analyzer vocabulary");
    expectEqual (preferences.midi.mapCc, 20, "default map cc");
    expectEqual (preferences.midi.inputIdentifier, "", "default midi input identifier");
    expectEqual (preferences.midi.inputName, "", "default midi input name");
    expect (preferences.liveIO.sendMode == myworld::LiveIOSendModePreference::dryRun,
            "live IO send mode defaults to dry-run");
    expectEqual (preferences.liveIO.oscHost, "127.0.0.1", "default osc host");
    expectEqual (preferences.liveIO.oscPort, 9000, "default osc port");
    expectEqual (preferences.liveIO.oscLoudnessAddress, "/my-world/loudness", "default osc loudness address");
    expectEqual (myworld::liveIOSendModePreferenceToString (preferences.liveIO.sendMode),
                 "dry_run",
                 "default live IO send mode string");

    preferences.audio.analysisGain = -4.0f;
    preferences.midi.channel = 99;
    preferences.midi.loudnessCc = -9;
    preferences.midi.mapCc = 300;
    preferences.liveIO.sendMode = static_cast<myworld::LiveIOSendModePreference> (-20);
    preferences.liveIO.oscHost = "";
    preferences.liveIO.oscPort = 70000;
    preferences.liveIO.oscLoudnessAddress = "bad-address";
    preferences = myworld::sanitizePerformancePreferences (preferences);

    expect (preferences.audio.analysisGain == 0.0f, "analysis gain clamps low");
    expectEqual (preferences.midi.channel, 16, "midi channel clamps high");
    expectEqual (preferences.midi.loudnessCc, 0, "loudness cc clamps low");
    expectEqual (preferences.midi.mapCc, 127, "map cc clamps high");
    expect (preferences.liveIO.sendMode == myworld::LiveIOSendModePreference::dryRun,
            "invalid live IO send mode sanitizes to dry-run");
    expectEqual (preferences.liveIO.oscHost, "127.0.0.1", "empty osc host sanitizes to loopback");
    expectEqual (preferences.liveIO.oscPort, 65535, "osc port clamps high");
    expectEqual (preferences.liveIO.oscLoudnessAddress, "/my-world/loudness", "invalid osc address sanitizes to default");

    preferences = myworld::makeDefaultPerformancePreferences();
    preferences.liveIO.sendMode = myworld::LiveIOSendModePreference::controlledSend;
    preferences = myworld::sanitizePerformancePreferences (preferences);
    expect (preferences.liveIO.sendMode == myworld::LiveIOSendModePreference::controlledSend,
            "controlled send survives sanitization");
    preferences.midi.inputIdentifier = "midi-in-1";
    preferences.midi.inputName = "Keyboard In";
    preferences = myworld::sanitizePerformancePreferences (preferences);
    expectEqual (preferences.midi.inputIdentifier, "midi-in-1", "midi input identifier survives sanitization");
    expectEqual (preferences.midi.inputName, "Keyboard In", "midi input name survives sanitization");
    expectEqual (myworld::liveIOSendModePreferenceToString (preferences.liveIO.sendMode),
                 "controlled_send",
                 "controlled live IO send mode string");

    std::vector<std::string> availableInputs { "midi-in-1", "midi-in-2" };
    auto teachInputs = myworld::midiTeachInputIdentifiers (preferences, availableInputs);
    expectEqual (static_cast<int> (teachInputs.size()), 1, "selected midi input narrows teach listener count");
    expectEqual (teachInputs[0], "midi-in-1", "selected midi input is used for teach");

    preferences.midi.inputIdentifier.clear();
    teachInputs = myworld::midiTeachInputIdentifiers (preferences, availableInputs);
    expectEqual (static_cast<int> (teachInputs.size()), 2, "empty midi input selection listens to all inputs");
    expectEqual (teachInputs[0], "midi-in-1", "all-input fallback keeps first input");
    expectEqual (teachInputs[1], "midi-in-2", "all-input fallback keeps second input");

    preferences.midi.inputIdentifier = "missing";
    teachInputs = myworld::midiTeachInputIdentifiers (preferences, availableInputs);
    expectEqual (static_cast<int> (teachInputs.size()), 0, "missing selected midi input listens to no inputs");

    auto frame = myworld::makeLoudnessMidiCcFrame (0.5f, preferences);
    expect (! frame.shouldSend, "default preferences should not send midi");

    preferences.midi.streamEnabled = true;
    preferences.midi.channel = 3;
    frame = myworld::makeLoudnessMidiCcFrame (0.5f, preferences);
    expect (frame.shouldSend, "stream enabled sends loudness cc");
    expectEqual (frame.channel, 3, "stream midi channel");
    expectEqual (frame.cc, 20, "stream loudness cc");
    expectEqual (frame.value, 64, "stream maps 0.5 to center cc value");

    preferences.midi.mapModeEnabled = true;
    preferences.midi.mapCc = 42;
    frame = myworld::makeLoudnessMidiCcFrame (0.9f, preferences);
    expect (frame.shouldSend, "map mode sends even when loudness changes");
    expectEqual (frame.cc, 42, "map mode cc");
    expectEqual (frame.value, 64, "map mode sends stable center value");

    auto persisted = myworld::makeDefaultPerformancePreferences();
    persisted.audio.analysisGain = 2.5f;
    persisted.midi.streamEnabled = true;
    persisted.midi.mapModeEnabled = true;
    persisted.midi.channel = 9;
    persisted.midi.loudnessCc = 74;
    persisted.midi.mapCc = 18;
    persisted.midi.inputIdentifier = "input-device";
    persisted.midi.inputName = "Input Device";
    persisted.midi.outputIdentifier = "output-device";
    persisted.midi.outputName = "Output Device";
    persisted.liveIO.sendMode = myworld::LiveIOSendModePreference::controlledSend;
    persisted.liveIO.oscHost = "192.168.1.24";
    persisted.liveIO.oscPort = 9123;
    persisted.liveIO.oscLoudnessAddress = "/stage/loudness";

    const auto tempPath = std::filesystem::temp_directory_path() / "my-world-performance-preferences-test.properties";
    std::filesystem::remove (tempPath);

    expect (myworld::savePerformancePreferences (tempPath, persisted).ok, "save performance preferences");
    const auto loaded = myworld::loadPerformancePreferences (tempPath);
    expect (loaded.ok, "load performance preferences");
    expect (loaded.preferences.audio.analysisGain == 2.5f, "persisted analysis gain");
    expect (loaded.preferences.midi.streamEnabled, "persisted midi stream");
    expect (loaded.preferences.midi.mapModeEnabled, "persisted map mode");
    expectEqual (loaded.preferences.midi.channel, 9, "persisted midi channel");
    expectEqual (loaded.preferences.midi.loudnessCc, 74, "persisted loudness cc");
    expectEqual (loaded.preferences.midi.mapCc, 18, "persisted map cc");
    expectEqual (loaded.preferences.midi.inputIdentifier, "input-device", "persisted input identifier");
    expectEqual (loaded.preferences.midi.inputName, "Input Device", "persisted input name");
    expectEqual (loaded.preferences.midi.outputIdentifier, "output-device", "persisted output identifier");
    expectEqual (loaded.preferences.midi.outputName, "Output Device", "persisted output name");
    expect (loaded.preferences.liveIO.sendMode == myworld::LiveIOSendModePreference::controlledSend,
            "persisted live IO send mode");
    expectEqual (loaded.preferences.liveIO.oscHost, "192.168.1.24", "persisted osc host");
    expectEqual (loaded.preferences.liveIO.oscPort, 9123, "persisted osc port");
    expectEqual (loaded.preferences.liveIO.oscLoudnessAddress, "/stage/loudness", "persisted osc address");

    const auto missing = myworld::loadPerformancePreferences (tempPath.parent_path() / "missing-preferences.properties");
    expect (missing.ok, "missing performance preferences fall back to defaults");
    expectEqual (missing.status, "default", "missing preferences status");
    expectEqual (missing.preferences.midi.channel, 1, "missing preferences default channel");

    std::filesystem::remove (tempPath);

    std::cout << "performance preferences ok\n";
    return 0;
}
