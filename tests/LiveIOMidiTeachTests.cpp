#include "LiveIOMidiTeach.h"
#include "LiveIOBus.h"

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
    myworld::LiveIOMidiTeachState state;
    expect (! state.armed, "teach starts idle");
    expectEqual (myworld::liveIOMidiTeachTargetToString (state.target), "none", "default teach target");

    const auto invalidArm = myworld::armLiveIOMidiTeach (state, myworld::LiveIOMidiTeachTarget::none);
    expect (! invalidArm.ok, "cannot arm without target");
    expect (! state.armed, "invalid arm stays idle");
    expectEqual (invalidArm.status, "failed", "invalid arm status");

    const auto armed = myworld::armLiveIOMidiTeach (state, myworld::LiveIOMidiTeachTarget::loudnessCc);
    expect (armed.ok, armed.message);
    expect (state.armed, "teach is armed");
    expectEqual (armed.status, "armed", "armed status");
    expectEqual (myworld::liveIOMidiTeachTargetToString (state.target), "loudness_cc", "armed target");

    myworld::LiveIOMidiTeachIncomingMessage nonCc;
    nonCc.isControlChange = false;
    const auto ignored = myworld::handleLiveIOMidiTeachMessage (state, nonCc);
    expect (ignored.ok, ignored.message);
    expect (! ignored.learned, "non-cc does not learn");
    expect (! ignored.consumed, "non-cc is not consumed");
    expect (state.armed, "non-cc keeps teach armed");
    expectEqual (state.ignoredMessageCount, 1, "ignored count");

    const auto learned = myworld::handleLiveIOMidiTeachMessage (
        state,
        myworld::makeLiveIOMidiTeachControlChange (14, 99, 127));
    expect (learned.ok, learned.message);
    expect (learned.learned, "cc learns");
    expect (learned.consumed, "cc is consumed");
    expect (! state.armed, "learn disarms teach");
    expectEqual (learned.channel, 14, "learned channel");
    expectEqual (learned.cc, 99, "learned cc");
    expectEqual (state.learnedChannel, 14, "state learned channel");
    expectEqual (state.learnedCc, 99, "state learned cc");
    expectEqual (state.learnedMessageCount, 1, "learned count");
    expectEqual (myworld::liveIOMidiTeachTargetToString (state.lastLearnedTarget),
                 "loudness_cc",
                 "last learned target");

    const auto unarmed = myworld::handleLiveIOMidiTeachMessage (
        state,
        myworld::makeLiveIOMidiTeachControlChange (2, 10, 64));
    expect (unarmed.ok, unarmed.message);
    expect (! unarmed.learned, "unarmed teach ignores cc");
    expectEqual (unarmed.status, "idle", "unarmed status");

    const auto mapArm = myworld::armLiveIOMidiTeach (state, myworld::LiveIOMidiTeachTarget::mapCc);
    expect (mapArm.ok, mapArm.message);
    const auto clamped = myworld::handleLiveIOMidiTeachMessage (
        state,
        myworld::makeLiveIOMidiTeachControlChange (99, -4, 32));
    expect (clamped.learned, "clamped cc learns");
    expectEqual (clamped.channel, 16, "learned channel clamps high");
    expectEqual (clamped.cc, 0, "learned cc clamps low");
    expectEqual (myworld::liveIOMidiTeachTargetToString (clamped.target),
                 "map_cc",
                 "learned map target");

    myworld::armLiveIOMidiTeach (state, myworld::LiveIOMidiTeachTarget::loudnessCc);
    const auto cancelled = myworld::cancelLiveIOMidiTeach (state);
    expect (cancelled.ok, cancelled.message);
    expect (! state.armed, "cancel disarms");
    expectEqual (cancelled.status, "cancelled", "cancel status");

    const auto json = myworld::makeLiveIOMidiTeachStateJson (state);
    expectContains (json, "\"kind\": \"liveIOMidiTeachState\"", "teach json kind");
    expectContains (json, "\"armed\": false", "teach json armed");
    expectContains (json, "\"status\": \"cancelled\"", "teach json status");
    expectContains (json, "\"lastLearnedTarget\": \"map_cc\"", "teach json target");
    expectContains (json, "\"learnedMessageCount\": 2", "teach json learned count");

    std::vector<myworld::LiveIOBinding> bindings {
        myworld::makeLiveIOMidiCcBinding ("midi.loudness", "out", 1, 20),
        myworld::makeLiveIOMidiCcBinding ("midi.attack", "attack", 2, 21),
        myworld::makeLiveIOOscFloatBinding ("osc.loudness", "out", "/my-world/loudness")
    };
    const auto bindingArm = myworld::armLiveIOMidiTeachForBinding (state, "midi.attack");
    expect (bindingArm.ok, bindingArm.message);
    expectEqual (bindingArm.bindingId, "midi.attack", "armed binding id");
    const auto bindingLearned = myworld::handleLiveIOMidiTeachMessage (
        state,
        myworld::makeLiveIOMidiTeachControlChange (6, 88, 127));
    expect (bindingLearned.learned, "binding teach learns");
    expectEqual (bindingLearned.bindingId, "midi.attack", "learned binding id");

    const auto applyResult = myworld::applyLiveIOMidiTeachToBindings (bindings, bindingLearned);
    expect (applyResult.ok, applyResult.message);
    expectEqual (bindings[0].midiChannel, 1, "unselected midi binding channel unchanged");
    expectEqual (bindings[0].midiCc, 20, "unselected midi binding cc unchanged");
    expectEqual (bindings[1].midiChannel, 6, "selected binding channel updated");
    expectEqual (bindings[1].midiCc, 88, "selected binding cc updated");
    expectEqual (bindings[2].oscAddress, "/my-world/loudness", "non-midi binding unchanged");

    std::cout << "live io midi teach ok\n";
    return 0;
}
