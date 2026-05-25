#include "LiveIOBus.h"
#include "LiveIOSendAdapter.h"

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
}

int main()
{
    myworld::LiveIOValueFrame frame;
    frame.values = {
        { "out", 0.5, "loudness_out.out" }
    };

    std::vector<myworld::LiveIOBinding> bindings;
    bindings.push_back (myworld::makeLiveIOMidiCcBinding ("midi.loudness", "out", 1, 20));
    bindings.push_back (myworld::makeLiveIOOscFloatBinding ("osc.loudness", "out", "/my-world/loudness"));
    bindings.push_back (myworld::makeLiveIOShaderUniformBinding ("uniform.loudness", "out", "u_loudness"));

    const auto busReport = myworld::evaluateLiveIOBus (frame, bindings);
    expect (busReport.ok, busReport.message);

    const auto route = myworld::makeLiveIODryRunSendRoute ("dry-run MIDI", "127.0.0.1", 9000);
    const auto sendReport = myworld::evaluateLiveIOSendBoundary (busReport, route);

    expect (sendReport.ok, sendReport.message);
    expectEqual (sendReport.status, "dry_run", "send report status");
    expectEqual (static_cast<int> (sendReport.actions.size()), 2, "send action count");
    expectEqual (static_cast<int> (sendReport.skipped.size()), 1, "skipped target count");
    expect (sendReport.errors.empty(), "dry-run send boundary should have no errors");

    const auto& midi = sendReport.actions.at (0);
    expect (midi.targetKind == myworld::LiveIOTargetKind::midiCc, "midi action target");
    expectEqual (midi.bindingId, "midi.loudness", "midi action binding id");
    expectEqual (midi.mode, "dry_run", "midi action mode");
    expect (! midi.sent, "dry-run midi action is not sent");
    expectEqual (midi.midiOutputName, "dry-run MIDI", "midi output name");
    expectEqual (midi.midiChannel, 1, "midi channel");
    expectEqual (midi.midiCc, 20, "midi cc");
    expectEqual (midi.midiValue, 64, "midi value");

    const auto& osc = sendReport.actions.at (1);
    expect (osc.targetKind == myworld::LiveIOTargetKind::oscFloat, "osc action target");
    expectEqual (osc.bindingId, "osc.loudness", "osc action binding id");
    expectEqual (osc.mode, "dry_run", "osc action mode");
    expect (! osc.sent, "dry-run osc action is not sent");
    expectEqual (osc.oscHost, "127.0.0.1", "osc host");
    expectEqual (osc.oscPort, 9000, "osc port");
    expectEqual (osc.oscAddress, "/my-world/loudness", "osc address");

    expectContains (sendReport.skipped.front(), "shader.uniform: uniform.loudness",
                    "shader uniform should stay outside device boundary");

    const auto json = myworld::makeLiveIOSendReportJson (sendReport);
    expectContains (json, "\"kind\": \"liveIOSendReport\"", "send report json");
    expectContains (json, "\"status\": \"dry_run\"", "send report json");
    expectContains (json, "\"targetKind\": \"midi.cc\"", "send report json");
    expectContains (json, "\"targetKind\": \"osc.float\"", "send report json");
    expectContains (json, "\"midiOutputName\": \"dry-run MIDI\"", "send report json");
    expectContains (json, "\"oscHost\": \"127.0.0.1\"", "send report json");
    expectContains (json, "\"oscPort\": 9000", "send report json");
    expectContains (json, "\"sent\": false", "send report json");
    expectContains (json, "\"skipped\": [\"shader.uniform: uniform.loudness\"]", "send report json");

    const auto missingMidiRoute = myworld::makeLiveIODryRunSendRoute ("", "127.0.0.1", 9000);
    const auto missingMidiReport = myworld::evaluateLiveIOSendBoundary (busReport, missingMidiRoute);
    expect (! missingMidiReport.ok, "missing midi route blocks send boundary");
    expectEqual (missingMidiReport.status, "blocked", "missing midi route status");
    expectContains (missingMidiReport.errors.front(), "midi output is required", "missing midi route error");

    const auto blockedReport = myworld::evaluateLiveIOSendBoundary (
        myworld::evaluateLiveIOBus (frame, { myworld::makeLiveIOOscFloatBinding ("osc.missing", "missing", "/missing") }),
        route);
    expect (! blockedReport.ok, "blocked bus report blocks send boundary");
    expectEqual (blockedReport.status, "blocked", "blocked bus status");
    expectContains (blockedReport.errors.front(), "live io bus is not mapped", "blocked bus error");

    std::cout << "live io send adapter ok\n";
    return 0;
}
