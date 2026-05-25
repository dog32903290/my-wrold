#include "LiveIOOscReceiver.h"

#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

namespace myworld
{
namespace
{
constexpr size_t maxOscDatagramBytes = 65536;

void appendPaddedOscString (std::vector<unsigned char>& bytes, const std::string& text)
{
    for (const auto character : text)
        bytes.push_back (static_cast<unsigned char> (character));

    bytes.push_back (0);

    while (bytes.size() % 4 != 0)
        bytes.push_back (0);
}

void appendOscFloat (std::vector<unsigned char>& bytes, float value)
{
    uint32_t bits = 0;
    std::memcpy (&bits, &value, sizeof (bits));

    bytes.push_back (static_cast<unsigned char> ((bits >> 24) & 0xff));
    bytes.push_back (static_cast<unsigned char> ((bits >> 16) & 0xff));
    bytes.push_back (static_cast<unsigned char> ((bits >> 8) & 0xff));
    bytes.push_back (static_cast<unsigned char> (bits & 0xff));
}

bool readPaddedOscString (const std::vector<unsigned char>& bytes,
                          size_t& offset,
                          std::string& text)
{
    if (offset >= bytes.size())
        return false;

    const auto start = offset;

    while (offset < bytes.size() && bytes[offset] != 0)
        ++offset;

    if (offset >= bytes.size())
        return false;

    text.assign (reinterpret_cast<const char*> (bytes.data() + start), offset - start);
    ++offset;

    while (offset % 4 != 0)
        ++offset;

    return offset <= bytes.size();
}

bool readOscFloat (const std::vector<unsigned char>& bytes, size_t offset, float& value)
{
    if (offset + 4 > bytes.size())
        return false;

    const uint32_t bits = (static_cast<uint32_t> (bytes[offset]) << 24)
        | (static_cast<uint32_t> (bytes[offset + 1]) << 16)
        | (static_cast<uint32_t> (bytes[offset + 2]) << 8)
        | static_cast<uint32_t> (bytes[offset + 3]);

    std::memcpy (&value, &bits, sizeof (value));
    return true;
}

std::string normalizedHost (const std::string& host)
{
    if (host.empty() || host == "localhost")
        return "127.0.0.1";

    return host;
}
}

std::vector<unsigned char> makeLiveIOOscFloatDatagram (const std::string& address, float value)
{
    std::vector<unsigned char> bytes;
    appendPaddedOscString (bytes, address);
    appendPaddedOscString (bytes, ",f");
    appendOscFloat (bytes, value);
    return bytes;
}

LiveIOOscFloatPacket decodeLiveIOOscFloatDatagram (const std::vector<unsigned char>& bytes)
{
    LiveIOOscFloatPacket packet;
    size_t offset = 0;
    std::string typeTag;

    if (! readPaddedOscString (bytes, offset, packet.address) || packet.address.empty())
    {
        packet.status = "failed";
        packet.message = "osc address is required";
        return packet;
    }

    if (! readPaddedOscString (bytes, offset, typeTag) || typeTag != ",f")
    {
        packet.status = "failed";
        packet.message = "osc float typetag is required";
        return packet;
    }

    if (! readOscFloat (bytes, offset, packet.floatValue))
    {
        packet.status = "failed";
        packet.message = "osc float value is required";
        return packet;
    }

    packet.ok = true;
    packet.status = "decoded";
    packet.message = "osc_float_decoded";
    return packet;
}

LiveIOValueFrame makeLiveIOValueFrameFromOscReceive (const LiveIOOscReceiverConfig& config,
                                                     const LiveIOOscFloatPacket& packet)
{
    LiveIOValueFrame frame;

    if (! packet.ok || packet.address != config.address)
        return frame;

    frame.values.push_back ({ config.valueId, packet.floatValue, "osc:" + packet.address });
    return frame;
}

LiveIOOscReceiver::~LiveIOOscReceiver()
{
    close();
}

LiveIOOscReceiverOpenResult LiveIOOscReceiver::open (LiveIOOscReceiverConfig config)
{
    close();
    receiverConfig = std::move (config);

    socketFd = ::socket (AF_INET, SOCK_DGRAM, 0);
    if (socketFd < 0)
        return { false, "failed", "could not open osc receiver socket", 0 };

    sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_port = htons (static_cast<uint16_t> (receiverConfig.port));

    const auto host = normalizedHost (receiverConfig.host);
    if (::inet_pton (AF_INET, host.c_str(), &address.sin_addr) != 1)
    {
        close();
        return { false, "failed", "could not resolve osc receiver host", 0 };
    }

    if (::bind (socketFd, reinterpret_cast<sockaddr*> (&address), sizeof (address)) != 0)
    {
        close();
        return { false, "failed", "could not bind osc receiver", 0 };
    }

    sockaddr_in boundAddress {};
    socklen_t size = sizeof (boundAddress);
    if (::getsockname (socketFd, reinterpret_cast<sockaddr*> (&boundAddress), &size) != 0)
    {
        close();
        return { false, "failed", "could not read osc receiver port", 0 };
    }

    port = ntohs (boundAddress.sin_port);
    return { true, "open", "osc_receiver_open", port };
}

LiveIOOscReceiverPollResult LiveIOOscReceiver::pollOnce (int timeoutMs)
{
    if (socketFd < 0)
        return { false, false, "failed", "osc receiver is not open", {} };

    fd_set readSet;
    FD_ZERO (&readSet);
    FD_SET (socketFd, &readSet);

    timeval timeout {};
    timeout.tv_sec = timeoutMs / 1000;
    timeout.tv_usec = (timeoutMs % 1000) * 1000;

    const auto ready = ::select (socketFd + 1, &readSet, nullptr, nullptr, &timeout);
    if (ready == 0)
        return { true, false, "timeout", "osc_receiver_timeout", {} };

    if (ready < 0)
        return { false, false, "failed", "osc receiver poll failed", {} };

    std::vector<unsigned char> buffer (maxOscDatagramBytes);
    const auto bytes = ::recv (socketFd, buffer.data(), buffer.size(), 0);
    if (bytes <= 0)
        return { false, false, "failed", "osc receiver read failed", {} };

    buffer.resize (static_cast<size_t> (bytes));
    auto packet = decodeLiveIOOscFloatDatagram (buffer);
    return { packet.ok, packet.ok, packet.status, packet.message, packet };
}

void LiveIOOscReceiver::close()
{
    if (socketFd >= 0)
        ::close (socketFd);

    socketFd = -1;
    port = 0;
}

bool LiveIOOscReceiver::isOpen() const
{
    return socketFd >= 0;
}

int LiveIOOscReceiver::boundPort() const
{
    return port;
}

const LiveIOOscReceiverConfig& LiveIOOscReceiver::config() const
{
    return receiverConfig;
}
}
