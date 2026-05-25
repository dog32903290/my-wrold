#include "AudioAnalyzerState.h"
#include "LiveIOBus.h"
#include "LiveIOControlPump.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

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

void expectEqual (const std::string& actual, const std::string& expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + expected + " got " + actual);
}

void expectEqual (int actual, int expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + std::to_string (expected)
                                + " got " + std::to_string (actual));
}

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}

myworld::AudioAnalyzerSnapshot makeSnapshot (float loudness, bool active, std::uint64_t sampleCounter)
{
    myworld::AudioAnalyzerSnapshot snapshot;
    snapshot.rms = loudness;
    snapshot.peak = loudness;
    snapshot.loudness = loudness;
    snapshot.gate = active ? 1.0f : 0.0f;
    snapshot.confidence = snapshot.gate;
    snapshot.active = active;
    snapshot.sampleCounter = sampleCounter;
    return snapshot;
}
}

int main()
{
    std::vector<myworld::LiveIOBinding> bindings;
    bindings.push_back (myworld::makeLiveIOMidiCcBinding ("midi.loudness", "out", 1, 20));
    bindings.push_back (myworld::makeLiveIOOscFloatBinding ("osc.loudness", "out", "/my-world/loudness"));
    bindings.push_back (myworld::makeLiveIOShaderUniformBinding ("uniform.loudness", "out", "u_loudness"));

    myworld::LiveIOControlPumpRequest request;
    request.ticks = {
        { 0, makeSnapshot (0.1f, true, 64) },
        { 20, makeSnapshot (0.2f, true, 128) },
        { 50, makeSnapshot (0.5f, true, 192) },
        { 120, makeSnapshot (0.75f, true, 256) },
        { 170, makeSnapshot (0.0f, false, 320) }
    };
    request.bindings = bindings;
    request.tickIntervalMs = 50;
    request.dispatchMinIntervalMs = 50;
    request.midiOutputInventory.devices = {
        { "IAC Driver Bus 1", "iac-1" }
    };
    request.midiOutputIdentifier = "iac-1";

    std::vector<myworld::LiveIOMidiCcMessage> midiMessages;
    std::vector<myworld::LiveIOOscFloatMessage> oscMessages;

    request.midiSender = [&] (const myworld::LiveIOMidiOutputDevice&,
                              const myworld::LiveIOMidiCcMessage& message)
    {
        midiMessages.push_back (message);
        return myworld::LiveIOMidiOutputDeviceSendResult { true, true, "" };
    };
    request.oscSender = [&] (const myworld::LiveIOOscFloatMessage& message)
    {
        oscMessages.push_back (message);
        return myworld::LiveIOOscFloatSendResult { true, "" };
    };

    const auto report = myworld::executeLiveIOControlPump (request);

    expect (report.ok, report.message);
    expectEqual (report.status, "pumped", "pump status");
    expectEqual (report.tickCount, 5, "tick count");
    expectEqual (report.frameCount, 3, "frame count");
    expectEqual (report.inactiveTickCount, 1, "inactive tick count");
    expectEqual (report.tickRateLimitedCount, 1, "tick rate-limited count");
    expectEqual (report.dispatch.dispatchedFrameCount, 3, "dispatch count");
    expectEqual (report.dispatch.rateLimitedFrameCount, 0, "dispatch rate-limited count");
    expectEqual (report.dispatch.midiSentCount, 3, "midi sent count");
    expectEqual (report.dispatch.oscSentCount, 3, "osc sent count");
    expectEqual (report.lastSampleCounter, 320, "last sample counter");
    expect (report.lastLoudness > 0.749 && report.lastLoudness < 0.751, "last active loudness");
    expectEqual (static_cast<int> (midiMessages.size()), 3, "midi sender count");
    expectEqual (static_cast<int> (oscMessages.size()), 3, "osc sender count");
    expectEqual (midiMessages.at (0).value, 13, "first midi value");
    expectEqual (midiMessages.at (1).value, 64, "second midi value");
    expectEqual (midiMessages.at (2).value, 95, "third midi value");

    expectEqual (report.ticks.at (0).status, "framed", "first tick status");
    expectEqual (report.ticks.at (1).status, "tick_rate_limited", "second tick status");
    expectEqual (report.ticks.at (4).status, "inactive", "inactive tick status");

    auto missingMidi = request;
    missingMidi.midiSender = {};
    const auto missingMidiReport = myworld::executeLiveIOControlPump (missingMidi);
    expect (! missingMidiReport.ok, "missing midi sender blocks pump");
    expectEqual (missingMidiReport.status, "failed", "missing midi status");
    expectContains (missingMidiReport.errors.front(), "midi output sender is unavailable",
                    "missing midi error");

    const auto json = myworld::makeLiveIOControlPumpReportJson (report);
    expectContains (json, "\"kind\": \"liveIOControlPumpProof\"", "pump json kind");
    expectContains (json, "\"ok\": true", "pump json ok");
    expectContains (json, "\"status\": \"pumped\"", "pump json status");
    expectContains (json, "\"tickCount\": 5", "pump json tick count");
    expectContains (json, "\"frameCount\": 3", "pump json frame count");
    expectContains (json, "\"inactiveTickCount\": 1", "pump json inactive count");
    expectContains (json, "\"tickRateLimitedCount\": 1", "pump json tick rate limit count");
    expectContains (json, "\"lastSampleCounter\": 320", "pump json sample counter");
    expectContains (json, "\"dispatch\": {", "pump json dispatch");
    expectContains (json, "\"midiSentCount\": 3", "pump json midi count");
    expectContains (json, "\"oscSentCount\": 3", "pump json osc count");
    expectContains (json, "\"status\": \"tick_rate_limited\"", "pump json tick rate limited");
    expectContains (json, "\"status\": \"inactive\"", "pump json inactive");
    expectContains (json, "\"errors\": []", "pump json no errors");

    std::cout << "live io control pump ok\n";
    return 0;
}
