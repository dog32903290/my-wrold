#include "LiveIOBus.h"

#include <cmath>
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

void expectNear (double actual, double expected, double tolerance, const std::string& message)
{
    expect (std::abs (actual - expected) <= tolerance,
            message + " expected near " + std::to_string (expected)
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
        { "out", 0.5, "loudness_out.out" },
        { "gate", 1.0, "pre_gate.gate" }
    };

    std::vector<myworld::LiveIOBinding> bindings;
    bindings.push_back (myworld::makeLiveIOMidiCcBinding ("midi.loudness", "out", 3, 42));
    bindings.push_back (myworld::makeLiveIOMidiNoteOnBinding ("midi.note", "out", 2, 60));
    bindings.push_back (myworld::makeLiveIOOscFloatBinding ("osc.loudness", "out", "/my-world/loudness"));
    bindings.push_back (myworld::makeLiveIOShaderUniformBinding ("uniform.loudness", "out", "u_loudness"));

    const auto report = myworld::evaluateLiveIOBus (frame, bindings);

    expect (report.ok, report.message);
    expectEqual (report.status, "mapped", "live io report status");
    expectEqual (static_cast<int> (report.events.size()), 4, "mapped event count");
    expect (report.errors.empty(), "no mapping errors");

    const auto& midi = report.events.at (0);
    expect (midi.targetKind == myworld::LiveIOTargetKind::midiCc, "midi target kind");
    expectEqual (midi.bindingId, "midi.loudness", "midi binding id");
    expectEqual (midi.sourceId, "out", "midi source id");
    expectEqual (midi.source, "loudness_out.out", "midi source path");
    expectNear (midi.normalizedValue, 0.5, 0.000001, "midi normalized value");
    expectEqual (midi.midiChannel, 3, "midi channel");
    expectEqual (midi.midiCc, 42, "midi cc");
    expectEqual (midi.midiValue, 64, "midi value rounds normalized value");

    const auto& note = report.events.at (1);
    expect (note.targetKind == myworld::LiveIOTargetKind::midiNoteOn, "midi note target kind");
    expectEqual (note.bindingId, "midi.note", "midi note binding id");
    expectEqual (note.midiChannel, 2, "midi note channel");
    expectEqual (note.midiNote, 60, "midi note number");
    expectEqual (note.midiVelocity, 64, "midi note velocity follows normalized value");

    const auto& osc = report.events.at (2);
    expect (osc.targetKind == myworld::LiveIOTargetKind::oscFloat, "osc target kind");
    expectEqual (osc.oscAddress, "/my-world/loudness", "osc address");
    expectNear (osc.floatValue, 0.5, 0.000001, "osc float value");

    const auto& uniform = report.events.at (3);
    expect (uniform.targetKind == myworld::LiveIOTargetKind::shaderUniform, "shader uniform target kind");
    expectEqual (uniform.uniformName, "u_loudness", "uniform name");
    expectNear (uniform.floatValue, 0.5, 0.000001, "uniform float value");

    auto clampedBinding = myworld::makeLiveIOMidiCcBinding ("midi.clamped", "gate", 99, -3);
    const auto clampedReport = myworld::evaluateLiveIOBus (frame, { clampedBinding });
    expect (clampedReport.ok, clampedReport.message);
    expectEqual (clampedReport.events.front().midiChannel, 16, "midi channel clamps high");
    expectEqual (clampedReport.events.front().midiCc, 0, "midi cc clamps low");
    expectEqual (clampedReport.events.front().midiValue, 127, "midi value clamps high");

    const auto missingReport = myworld::evaluateLiveIOBus (
        frame,
        { myworld::makeLiveIOOscFloatBinding ("osc.missing", "missing", "/missing") });
    expect (! missingReport.ok, "missing source fails report");
    expectEqual (missingReport.status, "blocked", "missing source status");
    expectEqual (static_cast<int> (missingReport.events.size()), 0, "missing source emits no events");
    expectEqual (static_cast<int> (missingReport.errors.size()), 1, "missing source error count");
    expectContains (missingReport.errors.front(), "missing source: missing", "missing source error");

    const auto json = myworld::makeLiveIOBusReportJson (report);
    expectContains (json, "\"kind\": \"liveIOBusReport\"", "live io json");
    expectContains (json, "\"status\": \"mapped\"", "live io json");
    expectContains (json, "\"targetKind\": \"midi.cc\"", "live io json");
    expectContains (json, "\"targetKind\": \"midi.note_on\"", "live io json");
    expectContains (json, "\"targetKind\": \"osc.float\"", "live io json");
    expectContains (json, "\"targetKind\": \"shader.uniform\"", "live io json");
    expectContains (json, "\"source\": \"loudness_out.out\"", "live io json");
    expectContains (json, "\"midiValue\": 64", "live io json");
    expectContains (json, "\"midiNote\": 60", "live io json");
    expectContains (json, "\"midiVelocity\": 64", "live io json");
    expectContains (json, "\"oscAddress\": \"/my-world/loudness\"", "live io json");
    expectContains (json, "\"uniformName\": \"u_loudness\"", "live io json");

    std::cout << "live io bus ok\n";
    return 0;
}
