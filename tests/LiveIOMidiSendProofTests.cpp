#include "LiveIOMidiSendProof.h"

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
    myworld::LiveIOMidiOutputInventory inventory;
    inventory.devices = {
        { "IAC Driver Bus 1", "iac-1" }
    };

    myworld::LiveIOMidiCcMessage message;
    message.bindingId = "midi.loudness";
    message.channel = 1;
    message.cc = 20;
    message.value = 64;

    std::string openedIdentifier;
    myworld::LiveIOMidiCcMessage sentMessage;

    const auto sent = myworld::executeLiveIOMidiOutputSendProof (
        inventory,
        myworld::makeLiveIOMidiOutputSendRequestByIdentifier ("iac-1", message),
        [&] (const myworld::LiveIOMidiOutputDevice& device, const myworld::LiveIOMidiCcMessage& outgoing)
        {
            openedIdentifier = device.identifier;
            sentMessage = outgoing;
            return myworld::LiveIOMidiOutputDeviceSendResult { true, true, "" };
        });

    expect (sent.ok, sent.message);
    expectEqual (sent.status, "sent", "send proof status");
    expect (sent.opened, "send proof opened device");
    expect (sent.sent, "send proof sent message");
    expectEqual (sent.selectedName, "IAC Driver Bus 1", "send proof selected name");
    expectEqual (sent.selectedIdentifier, "iac-1", "send proof selected identifier");
    expectEqual (sent.channel, 1, "send proof channel");
    expectEqual (sent.cc, 20, "send proof cc");
    expectEqual (sent.value, 64, "send proof value");
    expectEqual (sent.statusByte, 176, "send proof status byte");
    expectEqual (sent.data1, 20, "send proof data1");
    expectEqual (sent.data2, 64, "send proof data2");
    expectEqual (openedIdentifier, "iac-1", "fake sender opened identifier");
    expectEqual (sentMessage.bindingId, "midi.loudness", "fake sender binding id");

    const auto noteMessage = myworld::makeLiveIOMidiNoteOnMessage ("midi.note", 2, 60, 96);
    myworld::LiveIOMidiMessage sentNoteMessage;

    const auto noteSent = myworld::executeLiveIOMidiOutputSendProof (
        inventory,
        myworld::makeLiveIOMidiOutputSendRequestByIdentifier ("iac-1", noteMessage),
        [&] (const myworld::LiveIOMidiOutputDevice&, const myworld::LiveIOMidiMessage& outgoing)
        {
            sentNoteMessage = outgoing;
            return myworld::LiveIOMidiOutputDeviceSendResult { true, true, "" };
        });

    expect (noteSent.ok, noteSent.message);
    expectEqual (noteSent.message, "midi_output_note_on_sent", "note send message");
    expectEqual (noteSent.channel, 2, "note send channel");
    expectEqual (noteSent.note, 60, "note send note");
    expectEqual (noteSent.velocity, 96, "note send velocity");
    expectEqual (noteSent.statusByte, 145, "note send status byte");
    expectEqual (noteSent.data1, 60, "note send data1");
    expectEqual (noteSent.data2, 96, "note send data2");
    expectEqual (sentNoteMessage.bindingId, "midi.note", "fake sender note binding id");
    expect (sentNoteMessage.kind == myworld::LiveIOMidiMessageKind::noteOn, "fake sender note kind");

    const auto missing = myworld::executeLiveIOMidiOutputSendProof (
        inventory,
        myworld::makeLiveIOMidiOutputSendRequestByIdentifier ("missing", message),
        [] (const myworld::LiveIOMidiOutputDevice&, const myworld::LiveIOMidiCcMessage&)
        {
            return myworld::LiveIOMidiOutputDeviceSendResult { true, true, "" };
        });
    expect (! missing.ok, "missing route blocks send proof");
    expectEqual (missing.status, "unavailable", "missing route status");
    expect (! missing.opened, "missing route does not open device");
    expect (! missing.sent, "missing route does not send message");
    expectContains (missing.errors.front(), "midi output is unavailable: missing", "missing route error");

    const auto openFailed = myworld::executeLiveIOMidiOutputSendProof (
        inventory,
        myworld::makeLiveIOMidiOutputSendRequestByIdentifier ("iac-1", message),
        [] (const myworld::LiveIOMidiOutputDevice&, const myworld::LiveIOMidiCcMessage&)
        {
            return myworld::LiveIOMidiOutputDeviceSendResult { false, false, "open failed" };
        });
    expect (! openFailed.ok, "open failure blocks send proof");
    expectEqual (openFailed.status, "open_failed", "open failure status");
    expect (! openFailed.opened, "open failure opened flag");
    expect (! openFailed.sent, "open failure sent flag");
    expectContains (openFailed.errors.front(), "open failed", "open failure error");

    const auto sendFailed = myworld::executeLiveIOMidiOutputSendProof (
        inventory,
        myworld::makeLiveIOMidiOutputSendRequestByIdentifier ("iac-1", message),
        [] (const myworld::LiveIOMidiOutputDevice&, const myworld::LiveIOMidiCcMessage&)
        {
            return myworld::LiveIOMidiOutputDeviceSendResult { true, false, "send failed" };
        });
    expect (! sendFailed.ok, "send failure blocks send proof");
    expectEqual (sendFailed.status, "send_failed", "send failure status");
    expect (sendFailed.opened, "send failure opened flag");
    expect (! sendFailed.sent, "send failure sent flag");
    expectContains (sendFailed.errors.front(), "send failed", "send failure error");

    const auto missingSender = myworld::executeLiveIOMidiOutputSendProof (
        inventory,
        myworld::makeLiveIOMidiOutputSendRequestByIdentifier ("iac-1", message),
        myworld::LiveIOMidiOutputSender {});
    expect (! missingSender.ok, "missing sender blocks send proof");
    expectEqual (missingSender.status, "sender_unavailable", "missing sender status");
    expect (! missingSender.opened, "missing sender opened flag");
    expect (! missingSender.sent, "missing sender sent flag");
    expectContains (missingSender.errors.front(), "midi output sender is unavailable",
                    "missing sender error");

    const auto json = myworld::makeLiveIOMidiOutputSendReportJson (sent);
    expectContains (json, "\"kind\": \"liveIOMidiOutputSendProof\"", "send json");
    expectContains (json, "\"ok\": true", "send json");
    expectContains (json, "\"status\": \"sent\"", "send json");
    expectContains (json, "\"opened\": true", "send json");
    expectContains (json, "\"sent\": true", "send json");
    expectContains (json, "\"selectedIdentifier\": \"iac-1\"", "send json");
    expectContains (json, "\"channel\": 1", "send json");
    expectContains (json, "\"cc\": 20", "send json");
    expectContains (json, "\"value\": 64", "send json");
    expectContains (json, "\"statusByte\": 176", "send json");
    expectContains (json, "\"data1\": 20", "send json");
    expectContains (json, "\"data2\": 64", "send json");
    expectContains (json, "\"errors\": []", "send json");

    const auto noteJson = myworld::makeLiveIOMidiOutputSendReportJson (noteSent);
    expectContains (noteJson, "\"messageKind\": \"note_on\"", "note send json kind");
    expectContains (noteJson, "\"note\": 60", "note send json note");
    expectContains (noteJson, "\"velocity\": 96", "note send json velocity");

    std::cout << "live io midi send proof ok\n";
    return 0;
}
