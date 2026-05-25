#include "AudioAnalyzerState.h"
#include "LiveIOAppController.h"

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

void expectEqual (const std::string& actual, const std::string& expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + expected + " got " + actual);
}

void expectEqual (int actual, int expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + std::to_string (expected)
                                + " got " + std::to_string (actual));
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
    myworld::LiveIOAppController controller;

    myworld::PerformancePreferences preferences = myworld::makeDefaultPerformancePreferences();
    preferences.midi.channel = 4;
    preferences.midi.loudnessCc = 74;

    myworld::LiveIOAppTimerRequest dryRequest;
    dryRequest.preferences = preferences;
    dryRequest.snapshot = makeSnapshot (0.5f, true, 64);
    dryRequest.timestampMs = 0;

    const auto dry = controller.tick (dryRequest);
    expect (dry.tick.ok, dry.tick.message);
    expectEqual (dry.indicator.text, "live io dry pumped m1 o1", "dry-run indicator text");
    expectEqual (dry.indicator.tone, "dry_run", "dry-run indicator tone");

    preferences.liveIO.sendMode = myworld::LiveIOSendModePreference::controlledSend;
    preferences.midi.outputIdentifier = "app-midi";
    preferences.midi.outputName = "App MIDI";
    controller.applyLiveIOPreferences (preferences.liveIO);

    int sentCount = 0;
    myworld::LiveIOAppTimerRequest sendRequest;
    sendRequest.preferences = preferences;
    sendRequest.snapshot = makeSnapshot (0.75f, true, 128);
    sendRequest.timestampMs = 60;
    sendRequest.midiSender = [&] (const myworld::LiveIOMidiOutputDevice& device,
                                  const myworld::LiveIOMidiCcMessage& message)
    {
        ++sentCount;
        expectEqual (device.identifier, "app-midi", "controlled midi device");
        expectEqual (message.channel, 4, "controlled midi channel");
        expectEqual (message.cc, 74, "controlled midi cc");
        expectEqual (message.value, 95, "controlled midi value");
        return myworld::LiveIOMidiOutputDeviceSendResult { true, true, "" };
    };

    const auto controlled = controller.tick (sendRequest);
    expect (controlled.tick.ok, controlled.tick.message);
    expectEqual (sentCount, 1, "controlled send count");
    expectEqual (controlled.indicator.text, "live io send controlled_sent m1 o0", "controlled indicator text");
    expectEqual (controlled.indicator.tone, "sending", "controlled indicator tone");

    preferences.liveIO.oscHost = "192.168.1.24";
    preferences.liveIO.oscPort = 9123;
    preferences.liveIO.oscLoudnessAddress = "/stage/loudness";
    controller.applyLiveIOPreferences (preferences.liveIO);

    int externalOscSendCount = 0;
    myworld::LiveIOAppTimerRequest oscRequest;
    oscRequest.preferences = preferences;
    oscRequest.snapshot = makeSnapshot (0.25f, true, 192);
    oscRequest.timestampMs = 120;
    oscRequest.midiSender = [&] (const myworld::LiveIOMidiOutputDevice&,
                                 const myworld::LiveIOMidiCcMessage&)
    {
        return myworld::LiveIOMidiOutputDeviceSendResult { true, true, "" };
    };
    oscRequest.oscSender = [&] (const myworld::LiveIOOscFloatMessage& message)
    {
        ++externalOscSendCount;
        expectEqual (message.oscHost, "192.168.1.24", "external osc host");
        expectEqual (message.oscPort, 9123, "external osc port");
        expectEqual (message.oscAddress, "/stage/loudness", "external osc address");
        expect (message.floatValue > 0.249 && message.floatValue < 0.251, "external osc value");
        return myworld::LiveIOOscFloatSendResult { true, "" };
    };

    const auto externalOsc = controller.tick (oscRequest);
    expect (externalOsc.tick.ok, externalOsc.tick.message);
    expectEqual (externalOscSendCount, 1, "controlled external osc send count");

    const auto armed = controller.armMidiTeach (myworld::LiveIOMidiTeachTarget::loudnessCc, 2);
    expect (armed.ok, armed.statusText);
    expect (armed.shouldListen, "armed teach should listen");
    expect (! armed.learned, "armed teach has not learned");
    expectEqual (armed.statusText, "teach loudness_cc waiting", "armed status text");

    myworld::LiveIOMidiTeachIncomingMessage nonCc;
    nonCc.isControlChange = false;
    const auto ignored = controller.handleMidiTeachMessage (nonCc);
    expect (ignored.ok, ignored.statusText);
    expect (ignored.shouldListen, "ignored non-cc keeps listening");
    expectEqual (ignored.statusText, "teach waiting for CC", "ignored status text");

    const auto learned = controller.handleMidiTeachMessage (
        myworld::makeLiveIOMidiTeachControlChange (9, 42, 127));
    expect (learned.ok, learned.statusText);
    expect (learned.learned, "teach learned cc");
    expect (! learned.shouldListen, "learned teach stops listening");
    expectEqual (learned.learnedChannel, 9, "learned channel");
    expectEqual (learned.learnedCc, 42, "learned cc");
    expectEqual (myworld::liveIOMidiTeachTargetToString (learned.learnedTarget),
                 "loudness_cc",
                 "learned target");
    expectEqual (learned.statusText, "learned ch9 cc42", "learned status text");

    const auto mapArmed = controller.armMidiTeach (myworld::LiveIOMidiTeachTarget::mapCc, 0);
    expect (mapArmed.shouldListen, "map teach should listen");
    expectEqual (mapArmed.statusText, "teach map_cc waiting / no inputs", "no-input status text");

    const auto cancelled = controller.cancelMidiTeach();
    expect (cancelled.ok, cancelled.statusText);
    expect (! cancelled.shouldListen, "cancel stops listening");
    expectEqual (cancelled.statusText, "teach cancelled", "cancel status text");

    std::cout << "live io app controller ok\n";
    return 0;
}
