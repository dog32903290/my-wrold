#include "AudioRealtimeDelivery.h"

#include "LiveIOControlTimer.h"

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
    snapshot.confidence = active ? 1.0f : 0.0f;
    snapshot.active = active;
    snapshot.sampleCounter = sampleCounter;
    return snapshot;
}
}

int main()
{
    myworld::AudioRealtimeDelivery delivery;
    std::uint64_t lastSeenSequence = 0;

    auto empty = delivery.consumeLatest (lastSeenSequence);
    expect (! empty.available, "empty delivery has no snapshot");
    expectEqual (static_cast<int> (lastSeenSequence), 0, "empty consume preserves sequence");

    delivery.publishFromRealtime (makeSnapshot (0.5f, true, 64));

    auto first = delivery.consumeLatest (lastSeenSequence);
    expect (first.available, "first realtime snapshot is available");
    expect (first.snapshot.active, "first snapshot active");
    expect (first.snapshot.loudness > 0.499f && first.snapshot.loudness < 0.501f,
            "first loudness delivered");
    expectEqual (static_cast<int> (first.snapshot.sampleCounter), 64, "first sample counter");
    expect (lastSeenSequence == first.sequence, "first consume advances sequence");

    auto repeated = delivery.consumeLatest (lastSeenSequence);
    expect (! repeated.available, "same sequence is not delivered twice");

    delivery.publishFromRealtime (makeSnapshot (0.75f, true, 128));

    auto second = delivery.consumeLatest (lastSeenSequence);
    expect (second.available, "second realtime snapshot is available");
    expectEqual (static_cast<int> (second.snapshot.sampleCounter), 128, "second sample counter");

    myworld::LiveIOControlTimerConfig config;
    config.bindings = {
        myworld::makeLiveIOMidiCcBinding ("midi.loudness", "out", 1, 20)
    };
    config.tickIntervalMs = 0;

    myworld::LiveIOControlTimerState state;
    const auto tick = myworld::tickLiveIOControlTimerDryRun (
        state,
        config,
        100,
        second.snapshot);

    expect (tick.ok, tick.message);
    expect (tick.pumped, "control timer pumps consumed realtime snapshot outside callback");
    expectEqual (state.midiDryRunCount, 1, "control timer dry-run MIDI count");
    expectEqual (static_cast<int> (state.lastSampleCounter), 128, "control timer sees realtime sample counter");

    std::cout << "audio realtime delivery ok\n";
    return 0;
}
