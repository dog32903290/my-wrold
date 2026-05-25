#include "LiveIOControlTimer.h"
#include "LiveIOStatusIndicator.h"

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

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}
}

int main()
{
    myworld::LiveIOControlTimerState idleState;
    const auto idle = myworld::makeLiveIOStatusIndicatorState (
        idleState,
        myworld::LiveIOControlTimerSendMode::dryRun);
    expectEqual (idle.text, "live io dry idle m0 o0", "idle text");
    expectEqual (idle.mode, "dry_run", "idle mode");
    expectEqual (idle.status, "idle", "idle status");
    expectEqual (idle.tone, "idle", "idle tone");
    expectEqual (idle.midiCount, 0, "idle midi count");
    expectEqual (idle.oscCount, 0, "idle osc count");

    myworld::LiveIOControlTimerState dryRunState;
    dryRunState.lastStatus = "pumped";
    dryRunState.lastMessage = "live_io_timer_dry_run_pumped";
    dryRunState.midiDryRunCount = 3;
    dryRunState.oscDryRunCount = 2;
    dryRunState.lastSampleCounter = 256;
    const auto dryRun = myworld::makeLiveIOStatusIndicatorState (
        dryRunState,
        myworld::LiveIOControlTimerSendMode::dryRun);
    expectEqual (dryRun.text, "live io dry pumped m3 o2", "dry-run text");
    expectEqual (dryRun.detail, "live_io_timer_dry_run_pumped", "dry-run detail");
    expectEqual (dryRun.tone, "dry_run", "dry-run tone");
    expectEqual (dryRun.midiCount, 3, "dry-run midi count");
    expectEqual (dryRun.oscCount, 2, "dry-run osc count");
    expect (dryRun.sampleCounter == 256, "dry-run sample counter");

    const auto operatorState = myworld::withLiveIOOutputOperator (dryRun, "osc.float");
    expectEqual (operatorState.text, "live io dry pumped m3 o2 op osc.float", "operator text");
    expectEqual (operatorState.outputOperator, "osc.float", "operator field");

    const auto uniformState = myworld::withLiveIOShaderUniformEvidence (
        operatorState,
        "uniform.loudness",
        "u_loudness",
        0.75,
        256);
    expect (uniformState.hasShaderUniform, "indicator records uniform evidence");
    expectEqual (uniformState.shaderUniformName, "u_loudness", "indicator uniform name");

    myworld::LiveIOControlTimerState controlledState;
    controlledState.lastStatus = "controlled_sent";
    controlledState.lastMessage = "live_io_timer_controlled_sent";
    controlledState.midiControlledSendCount = 1;
    controlledState.oscControlledSendCount = 1;
    controlledState.lastSampleCounter = 512;
    const auto controlled = myworld::makeLiveIOStatusIndicatorState (
        controlledState,
        myworld::LiveIOControlTimerSendMode::controlledSend);
    expectEqual (controlled.text, "live io send controlled_sent m1 o1", "controlled text");
    expectEqual (controlled.mode, "controlled_send", "controlled mode");
    expectEqual (controlled.tone, "sending", "controlled tone");
    expectEqual (controlled.midiCount, 1, "controlled midi count");
    expectEqual (controlled.oscCount, 1, "controlled osc count");

    myworld::LiveIOControlTimerState failedState;
    failedState.lastStatus = "failed";
    failedState.lastMessage = "midi output sender is unavailable";
    failedState.errors.push_back ("midi output sender is unavailable");
    const auto failed = myworld::makeLiveIOStatusIndicatorState (
        failedState,
        myworld::LiveIOControlTimerSendMode::controlledSend);
    expectEqual (failed.text, "live io send failed m0 o0", "failed text");
    expectEqual (failed.detail, "midi output sender is unavailable", "failed detail");
    expectEqual (failed.tone, "failed", "failed tone");

    myworld::LiveIOControlTimerState inactiveState;
    inactiveState.lastStatus = "inactive";
    inactiveState.lastMessage = "live_io_timer_inactive";
    const auto inactive = myworld::makeLiveIOStatusIndicatorState (
        inactiveState,
        myworld::LiveIOControlTimerSendMode::dryRun);
    expectEqual (inactive.text, "live io dry inactive m0 o0", "inactive text");
    expectEqual (inactive.tone, "inactive", "inactive tone");

    myworld::LiveIORealtimeIndicatorTelemetry realtime;
    realtime.status = "delivered";
    realtime.sequence = 6;
    realtime.droppedSnapshots = 1;
    realtime.skippedSnapshots = 1;
    realtime.overwrittenSnapshots = 0;

    const auto realtimeState = myworld::withLiveIORealtimeTelemetry (dryRun, realtime);
    expectEqual (realtimeState.text, "live io dry pumped m3 o2 rt delivered s6 skip1 over0", "realtime text");
    expectEqual (realtimeState.realtimeStatus, "delivered", "realtime status");
    expect (realtimeState.realtimeSequence == 6, "realtime sequence");
    expect (realtimeState.realtimeDroppedSnapshots == 1, "realtime dropped snapshots");
    expect (realtimeState.realtimeSkippedSnapshots == 1, "realtime skipped snapshots");
    expect (realtimeState.realtimeOverwrittenSnapshots == 0, "realtime overwritten snapshots");

    const auto json = myworld::makeLiveIOStatusIndicatorJson (realtimeState);
    expectContains (json, "\"kind\": \"liveIOStatusIndicator\"", "indicator json kind");
    expectContains (json, "\"text\": \"live io dry pumped m3 o2 rt delivered s6 skip1 over0\"", "indicator json text");
    expectContains (json, "\"tone\": \"dry_run\"", "indicator json tone");
    expectContains (json, "\"midiCount\": 3", "indicator json midi count");
    expectContains (json, "\"oscCount\": 2", "indicator json osc count");
    expectContains (json, "\"realtimeStatus\": \"delivered\"", "indicator json realtime status");
    expectContains (json, "\"realtimeSequence\": 6", "indicator json realtime sequence");
    expectContains (json, "\"realtimeDroppedSnapshots\": 1", "indicator json realtime dropped");
    expectContains (json, "\"realtimeSkippedSnapshots\": 1", "indicator json realtime skipped");
    expectContains (json, "\"realtimeOverwrittenSnapshots\": 0", "indicator json realtime overwritten");

    const auto operatorJson = myworld::makeLiveIOStatusIndicatorJson (operatorState);
    expectContains (operatorJson, "\"outputOperator\": \"osc.float\"", "indicator json operator");

    const auto uniformJson = myworld::makeLiveIOStatusIndicatorJson (uniformState);
    expectContains (uniformJson, "\"hasShaderUniform\": true", "indicator json has uniform");
    expectContains (uniformJson, "\"shaderUniformEvidence\": {", "indicator json uniform evidence object");
    expectContains (uniformJson, "\"source\": \"LiveIOStatusIndicatorState\"", "indicator json uniform source");
    expectContains (uniformJson, "\"shaderUniformName\": \"u_loudness\"", "indicator json uniform name");
    expectContains (uniformJson, "\"shaderUniformValue\": 0.750000", "indicator json uniform value");
    expectContains (uniformJson, "\"shaderUniformSampleCounter\": 256", "indicator json uniform sample");

    std::cout << "live io status indicator ok\n";
    return 0;
}
