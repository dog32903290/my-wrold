#include "LiveIOBus.h"
#include "LiveIOControlDispatcher.h"

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

myworld::LiveIOValueFrame makeFrame (double loudness)
{
    myworld::LiveIOValueFrame frame;
    frame.values = {
        { "out", loudness, "compound.loudness.publicOutputs" }
    };
    return frame;
}
}

int main()
{
    std::vector<myworld::LiveIOBinding> bindings;
    bindings.push_back (myworld::makeLiveIOMidiCcBinding ("midi.loudness", "out", 1, 20));
    bindings.push_back (myworld::makeLiveIOOscFloatBinding ("osc.loudness", "out", "/my-world/loudness"));
    bindings.push_back (myworld::makeLiveIOShaderUniformBinding ("uniform.loudness", "out", "u_loudness"));

    myworld::LiveIOControlDispatchRequest request;
    request.bindings = bindings;
    request.frames = {
        { 0, makeFrame (0.1) },
        { 10, makeFrame (0.2) },
        { 50, makeFrame (0.5) },
        { 120, makeFrame (0.75) }
    };
    request.minIntervalMs = 50;
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

    const auto report = myworld::executeLiveIOControlDispatch (request);

    expect (report.ok, report.message);
    expectEqual (report.status, "dispatched", "dispatch status");
    expectEqual (report.frameCount, 4, "frame count");
    expectEqual (report.dispatchedFrameCount, 3, "dispatched frame count");
    expectEqual (report.rateLimitedFrameCount, 1, "rate limited frame count");
    expectEqual (report.midiSentCount, 3, "midi sent count");
    expectEqual (report.oscSentCount, 3, "osc sent count");
    expectEqual (report.shaderSkippedCount, 3, "shader skipped count");
    expectEqual (static_cast<int> (midiMessages.size()), 3, "midi sender call count");
    expectEqual (static_cast<int> (oscMessages.size()), 3, "osc sender call count");

    expectEqual (midiMessages.at (0).value, 13, "first midi value");
    expectEqual (midiMessages.at (1).value, 64, "second midi value");
    expectEqual (midiMessages.at (2).value, 95, "third midi value");
    expectEqual (oscMessages.at (0).oscAddress, "/my-world/loudness", "osc address");
    expect (oscMessages.at (1).floatValue > 0.499 && oscMessages.at (1).floatValue < 0.501,
            "second osc value");

    expectEqual (report.frames.at (0).status, "dispatched", "first frame status");
    expectEqual (report.frames.at (1).status, "rate_limited", "second frame status");
    expectEqual (report.frames.at (2).status, "dispatched", "third frame status");

    auto missingOsc = request;
    missingOsc.oscSender = {};
    const auto missingOscReport = myworld::executeLiveIOControlDispatch (missingOsc);
    expect (! missingOscReport.ok, "missing osc sender blocks dispatch");
    expectEqual (missingOscReport.status, "failed", "missing osc sender status");
    expectContains (missingOscReport.errors.front(), "osc sender is unavailable: osc.loudness",
                    "missing osc sender error");

    const auto json = myworld::makeLiveIOControlDispatchReportJson (report);
    expectContains (json, "\"kind\": \"liveIOControlDispatchProof\"", "dispatch json kind");
    expectContains (json, "\"ok\": true", "dispatch json ok");
    expectContains (json, "\"status\": \"dispatched\"", "dispatch json status");
    expectContains (json, "\"frameCount\": 4", "dispatch json frame count");
    expectContains (json, "\"dispatchedFrameCount\": 3", "dispatch json dispatched count");
    expectContains (json, "\"rateLimitedFrameCount\": 1", "dispatch json rate limit count");
    expectContains (json, "\"midiSentCount\": 3", "dispatch json midi count");
    expectContains (json, "\"oscSentCount\": 3", "dispatch json osc count");
    expectContains (json, "\"shaderSkippedCount\": 3", "dispatch json shader skipped count");
    expectContains (json, "\"status\": \"rate_limited\"", "dispatch json frame status");
    expectContains (json, "\"errors\": []", "dispatch json no errors");

    std::cout << "live io control dispatcher ok\n";
    return 0;
}
