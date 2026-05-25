#include "AudioAnalyzerState.h"
#include "LiveIOBus.h"
#include "LiveIOControlTimer.h"

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
    myworld::LiveIOControlTimerDryRunConfig config;
    config.bindings = {
        myworld::makeLiveIOMidiCcBinding ("midi.loudness", "out", 1, 20),
        myworld::makeLiveIOOscFloatBinding ("osc.loudness", "out", "/my-world/loudness"),
        myworld::makeLiveIOShaderUniformBinding ("uniform.loudness", "out", "u_loudness")
    };
    config.tickIntervalMs = 50;
    config.dispatchMinIntervalMs = 0;

    myworld::LiveIOControlTimerState state;

    const auto first = myworld::tickLiveIOControlTimerDryRun (
        state,
        config,
        0,
        makeSnapshot (0.1f, true, 64));
    expect (first.ok, first.message);
    expectEqual (first.status, "pumped", "first tick status");
    expect (first.pumped, "first tick pumped");

    const auto fast = myworld::tickLiveIOControlTimerDryRun (
        state,
        config,
        20,
        makeSnapshot (0.2f, true, 128));
    expect (fast.ok, fast.message);
    expectEqual (fast.status, "tick_rate_limited", "fast tick status");
    expect (! fast.pumped, "fast tick is not pumped");

    const auto inactive = myworld::tickLiveIOControlTimerDryRun (
        state,
        config,
        70,
        makeSnapshot (0.0f, false, 192));
    expect (inactive.ok, inactive.message);
    expectEqual (inactive.status, "inactive", "inactive tick status");
    expect (! inactive.pumped, "inactive tick is not pumped");

    const auto second = myworld::tickLiveIOControlTimerDryRun (
        state,
        config,
        120,
        makeSnapshot (0.75f, true, 256));
    expect (second.ok, second.message);
    expectEqual (second.status, "pumped", "second pumped tick status");
    expect (second.pumped, "second tick pumped");

    expectEqual (state.tickCount, 4, "state tick count");
    expectEqual (state.pumpCount, 2, "state pump count");
    expectEqual (state.tickRateLimitedCount, 1, "state tick rate limit count");
    expectEqual (state.inactiveTickCount, 1, "state inactive count");
    expectEqual (state.midiDryRunCount, 2, "state midi dry-run count");
    expectEqual (state.oscDryRunCount, 2, "state osc dry-run count");
    expectEqual (state.shaderSkippedCount, 2, "state shader skipped count");
    expectEqual (state.lastStatus, "pumped", "state last status");
    expect (state.lastLoudness > 0.749 && state.lastLoudness < 0.751, "state last loudness");
    expectEqual (state.lastSampleCounter, 256, "state last sample counter");

    auto disabledConfig = config;
    disabledConfig.enabled = false;
    const auto disabled = myworld::tickLiveIOControlTimerDryRun (
        state,
        disabledConfig,
        180,
        makeSnapshot (0.9f, true, 320));
    expect (disabled.ok, disabled.message);
    expectEqual (disabled.status, "disabled", "disabled tick status");
    expect (! disabled.pumped, "disabled tick is not pumped");
    expectEqual (state.tickCount, 5, "disabled still records timer tick");
    expectEqual (state.pumpCount, 2, "disabled does not pump");

    const auto json = myworld::makeLiveIOControlTimerStateJson (state);
    expectContains (json, "\"kind\": \"liveIOControlTimerDryRunState\"", "timer json kind");
    expectContains (json, "\"tickCount\": 5", "timer json tick count");
    expectContains (json, "\"pumpCount\": 2", "timer json pump count");
    expectContains (json, "\"tickRateLimitedCount\": 1", "timer json tick rate limit count");
    expectContains (json, "\"inactiveTickCount\": 1", "timer json inactive count");
    expectContains (json, "\"midiDryRunCount\": 2", "timer json midi count");
    expectContains (json, "\"oscDryRunCount\": 2", "timer json osc count");
    expectContains (json, "\"midiControlledSendCount\": 0", "timer json controlled midi count");
    expectContains (json, "\"oscControlledSendCount\": 0", "timer json controlled osc count");
    expectContains (json, "\"shaderSkippedCount\": 2", "timer json shader count");
    expectContains (json, "\"lastStatus\": \"disabled\"", "timer json last status");
    expectContains (json, "\"lastSendMode\": \"dry_run\"", "timer json send mode");
    expectContains (json, "\"lastSampleCounter\": 320", "timer json sample counter");

    myworld::LiveIOControlTimerConfig gatedConfig = config;
    gatedConfig.sendMode = myworld::LiveIOControlTimerSendMode::dryRun;
    gatedConfig.midiOutputInventory.devices = {
        { "App Timer MIDI", "app-midi" }
    };
    gatedConfig.midiOutputIdentifier = "app-midi";

    int gatedMidiSendCount = 0;
    int gatedOscSendCount = 0;
    gatedConfig.midiSender = [&] (const myworld::LiveIOMidiOutputDevice&,
                                  const myworld::LiveIOMidiCcMessage&)
    {
        ++gatedMidiSendCount;
        return myworld::LiveIOMidiOutputDeviceSendResult { true, true, "" };
    };
    gatedConfig.oscSender = [&] (const myworld::LiveIOOscFloatMessage&)
    {
        ++gatedOscSendCount;
        return myworld::LiveIOOscFloatSendResult { true, "" };
    };

    myworld::LiveIOControlTimerState gatedState;
    const auto gated = myworld::tickLiveIOControlTimer (
        gatedState,
        gatedConfig,
        0,
        makeSnapshot (0.25f, true, 384));
    expect (gated.ok, gated.message);
    expectEqual (gated.status, "pumped", "dry-run gated status");
    expectEqual (gatedMidiSendCount, 0, "dry-run does not call provided midi sender");
    expectEqual (gatedOscSendCount, 0, "dry-run does not call provided osc sender");
    expectEqual (gatedState.midiDryRunCount, 1, "dry-run gated midi count");
    expectEqual (gatedState.oscDryRunCount, 1, "dry-run gated osc count");
    expectEqual (gatedState.midiControlledSendCount, 0, "dry-run controlled midi count");
    expectEqual (gatedState.oscControlledSendCount, 0, "dry-run controlled osc count");

    auto controlledConfig = gatedConfig;
    controlledConfig.sendMode = myworld::LiveIOControlTimerSendMode::controlledSend;
    int controlledMidiSendCount = 0;
    int controlledOscSendCount = 0;
    controlledConfig.midiSender = [&] (const myworld::LiveIOMidiOutputDevice&,
                                       const myworld::LiveIOMidiCcMessage& message)
    {
        ++controlledMidiSendCount;
        expectEqual (message.value, 95, "controlled midi value");
        return myworld::LiveIOMidiOutputDeviceSendResult { true, true, "" };
    };
    controlledConfig.oscSender = [&] (const myworld::LiveIOOscFloatMessage& message)
    {
        ++controlledOscSendCount;
        expect (message.floatValue > 0.749 && message.floatValue < 0.751, "controlled osc float");
        return myworld::LiveIOOscFloatSendResult { true, "" };
    };

    myworld::LiveIOControlTimerState controlledState;
    const auto controlled = myworld::tickLiveIOControlTimer (
        controlledState,
        controlledConfig,
        0,
        makeSnapshot (0.75f, true, 448));
    expect (controlled.ok, controlled.message);
    expectEqual (controlled.status, "controlled_sent", "controlled send status");
    expectEqual (controlledMidiSendCount, 1, "controlled calls midi sender");
    expectEqual (controlledOscSendCount, 1, "controlled calls osc sender");
    expectEqual (controlledState.midiDryRunCount, 0, "controlled dry-run midi count");
    expectEqual (controlledState.oscDryRunCount, 0, "controlled dry-run osc count");
    expectEqual (controlledState.midiControlledSendCount, 1, "controlled midi count");
    expectEqual (controlledState.oscControlledSendCount, 1, "controlled osc count");
    expectEqual (controlledState.lastSendMode, "controlled_send", "controlled state mode");

    auto missingSenderConfig = controlledConfig;
    missingSenderConfig.midiSender = {};
    missingSenderConfig.oscSender = [] (const myworld::LiveIOOscFloatMessage&)
    {
        return myworld::LiveIOOscFloatSendResult { true, "" };
    };
    myworld::LiveIOControlTimerState missingSenderState;
    const auto missingSender = myworld::tickLiveIOControlTimer (
        missingSenderState,
        missingSenderConfig,
        0,
        makeSnapshot (0.4f, true, 512));
    expect (! missingSender.ok, "controlled send requires midi sender");
    expectEqual (missingSender.status, "failed", "controlled missing sender status");
    expectContains (missingSenderState.errors.front(), "midi output sender is unavailable",
                    "controlled missing sender error");

    std::cout << "live io control timer ok\n";
    return 0;
}
