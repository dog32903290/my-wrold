#include "LiveIOBus.h"
#include "LiveIOSendAdapter.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string>
#include <unistd.h>
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

int openLoopbackReceiver()
{
    const auto fd = ::socket (AF_INET, SOCK_DGRAM, 0);
    expect (fd >= 0, "udp receiver socket");

    sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
    address.sin_port = 0;

    expect (::bind (fd, reinterpret_cast<sockaddr*> (&address), sizeof (address)) == 0,
            "bind udp receiver");
    return fd;
}

int boundPort (int fd)
{
    sockaddr_in address {};
    socklen_t size = sizeof (address);
    expect (::getsockname (fd, reinterpret_cast<sockaddr*> (&address), &size) == 0,
            "read udp receiver port");
    return ntohs (address.sin_port);
}

std::vector<unsigned char> receiveDatagram (int fd)
{
    fd_set readSet;
    FD_ZERO (&readSet);
    FD_SET (fd, &readSet);

    timeval timeout {};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    const auto ready = ::select (fd + 1, &readSet, nullptr, nullptr, &timeout);
    expect (ready > 0, "udp receiver should get one datagram");

    std::vector<unsigned char> buffer (256);
    const auto bytes = ::recv (fd, buffer.data(), buffer.size(), 0);
    expect (bytes > 0, "udp receiver bytes");
    buffer.resize (static_cast<size_t> (bytes));
    return buffer;
}

size_t paddedOscStringSize (const char* text)
{
    const auto lengthWithNull = std::strlen (text) + 1;
    return ((lengthWithNull + 3) / 4) * 4;
}

float readOscFloat (const std::vector<unsigned char>& datagram, size_t offset)
{
    expect (offset + 4 <= datagram.size(), "osc float bytes");
    const uint32_t bits = (static_cast<uint32_t> (datagram[offset]) << 24)
        | (static_cast<uint32_t> (datagram[offset + 1]) << 16)
        | (static_cast<uint32_t> (datagram[offset + 2]) << 8)
        | static_cast<uint32_t> (datagram[offset + 3]);

    float value = 0.0f;
    std::memcpy (&value, &bits, sizeof (value));
    return value;
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

    const auto receiver = openLoopbackReceiver();
    const auto receiverPort = boundPort (receiver);
    const auto loopbackRoute = myworld::makeLiveIOControlledOscLoopbackRoute ("127.0.0.1", receiverPort);
    const auto loopbackReport = myworld::executeLiveIOSendBoundary (busReport, loopbackRoute);

    expect (loopbackReport.ok, loopbackReport.message);
    expectEqual (loopbackReport.status, "controlled_send", "controlled loopback status");
    expectEqual (static_cast<int> (loopbackReport.actions.size()), 1, "controlled loopback action count");
    expectEqual (static_cast<int> (loopbackReport.skipped.size()), 2, "controlled loopback skipped count");

    const auto datagram = receiveDatagram (receiver);
    ::close (receiver);

    const auto& loopbackAction = loopbackReport.actions.front();
    expect (loopbackAction.targetKind == myworld::LiveIOTargetKind::oscFloat, "loopback action target");
    expect (loopbackAction.sent, "controlled loopback action should be sent");
    expectEqual (loopbackAction.mode, "controlled_send", "controlled loopback action mode");
    expectEqual (loopbackAction.oscHost, "127.0.0.1", "controlled loopback host");
    expectEqual (loopbackAction.oscPort, receiverPort, "controlled loopback port");
    expectEqual (loopbackAction.oscAddress, "/my-world/loudness", "controlled loopback address");

    expect (std::string (reinterpret_cast<const char*> (datagram.data())) == "/my-world/loudness",
            "osc datagram address");
    const auto typeOffset = paddedOscStringSize ("/my-world/loudness");
    expect (std::string (reinterpret_cast<const char*> (datagram.data() + typeOffset)) == ",f",
            "osc datagram type tag");
    const auto valueOffset = typeOffset + paddedOscStringSize (",f");
    const auto receivedValue = readOscFloat (datagram, valueOffset);
    expect (receivedValue > 0.499f && receivedValue < 0.501f, "osc datagram float value");

    const auto loopbackJson = myworld::makeLiveIOSendReportJson (loopbackReport);
    expectContains (loopbackJson, "\"status\": \"controlled_send\"", "loopback json");
    expectContains (loopbackJson, "\"sent\": true", "loopback json");
    expectContains (loopbackJson, "\"skipped\": [\"midi.disabled: midi.loudness\", \"shader.uniform: uniform.loudness\"]",
                    "loopback json skipped targets");

    std::cout << "live io send adapter ok\n";
    return 0;
}
