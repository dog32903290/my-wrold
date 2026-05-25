#include "LiveIOOscReceiver.h"

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

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

void sendUdp (int port, const std::vector<unsigned char>& bytes)
{
    const auto fd = ::socket (AF_INET, SOCK_DGRAM, 0);
    expect (fd >= 0, "udp sender socket");

    sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
    address.sin_port = htons (static_cast<uint16_t> (port));

    const auto sent = ::sendto (fd,
                                bytes.data(),
                                bytes.size(),
                                0,
                                reinterpret_cast<sockaddr*> (&address),
                                sizeof (address));
    ::close (fd);
    expect (sent == static_cast<ssize_t> (bytes.size()), "udp sender bytes");
}
}

int main()
{
    const auto packet = myworld::makeLiveIOOscFloatDatagram ("/stage/loudness", 0.75f);
    const auto decoded = myworld::decodeLiveIOOscFloatDatagram (packet);
    expect (decoded.ok, decoded.message);
    expectEqual (decoded.address, "/stage/loudness", "decoded address");
    expect (decoded.floatValue > 0.749f && decoded.floatValue < 0.751f, "decoded float");

    const std::vector<unsigned char> missingNullAddress { '/', 'x' };
    const auto missingNullDecoded = myworld::decodeLiveIOOscFloatDatagram (missingNullAddress);
    expect (! missingNullDecoded.ok, "missing address null should fail");
    expectEqual (missingNullDecoded.message, "osc address is required", "missing address null message");

    const std::vector<unsigned char> truncatedTypeTag {
        '/', 'x', 0, 0,
        ',', 'f'
    };
    const auto truncatedTypeDecoded = myworld::decodeLiveIOOscFloatDatagram (truncatedTypeTag);
    expect (! truncatedTypeDecoded.ok, "truncated typetag should fail");
    expectEqual (truncatedTypeDecoded.message, "osc float typetag is required", "truncated typetag message");

    auto truncatedFloat = myworld::makeLiveIOOscFloatDatagram ("/stage/loudness", 0.75f);
    truncatedFloat.pop_back();
    const auto truncatedFloatDecoded = myworld::decodeLiveIOOscFloatDatagram (truncatedFloat);
    expect (! truncatedFloatDecoded.ok, "truncated float should fail");
    expectEqual (truncatedFloatDecoded.message, "osc float value is required", "truncated float message");

    myworld::LiveIOOscReceiver receiver;
    const auto opened = receiver.open ({ "127.0.0.1", 0, "/stage/loudness", "osc.loudness" });
    expect (opened.ok, opened.message);
    expect (receiver.boundPort() > 0, "receiver bound port");

    sendUdp (receiver.boundPort(), packet);

    const auto polled = receiver.pollOnce (1000);
    expect (polled.ok, polled.message);
    expect (polled.received, "receiver gets datagram");
    expectEqual (polled.packet.address, "/stage/loudness", "received address");

    const auto frame = myworld::makeLiveIOValueFrameFromOscReceive (receiver.config(), polled.packet);
    expectEqual (static_cast<int> (frame.values.size()), 1, "osc receive frame value count");
    expectEqual (frame.values[0].id, "osc.loudness", "osc receive value id");
    expect (frame.values[0].value > 0.749 && frame.values[0].value < 0.751, "osc receive value");
    expectEqual (frame.values[0].source, "osc:/stage/loudness", "osc receive source");

    receiver.close();
    expect (! receiver.isOpen(), "receiver closes");

    std::cout << "live io osc receiver ok\n";
    return 0;
}
