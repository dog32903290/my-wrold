#include "PerformancePreferences.h"

#include <cstdlib>
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

    preferences.audio.analysisGain = -4.0f;
    preferences.midi.channel = 99;
    preferences.midi.loudnessCc = -9;
    preferences.midi.mapCc = 300;
    preferences = myworld::sanitizePerformancePreferences (preferences);

    expect (preferences.audio.analysisGain == 0.0f, "analysis gain clamps low");
    expectEqual (preferences.midi.channel, 16, "midi channel clamps high");
    expectEqual (preferences.midi.loudnessCc, 0, "loudness cc clamps low");
    expectEqual (preferences.midi.mapCc, 127, "map cc clamps high");

    preferences = myworld::makeDefaultPerformancePreferences();
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

    std::cout << "performance preferences ok\n";
    return 0;
}
