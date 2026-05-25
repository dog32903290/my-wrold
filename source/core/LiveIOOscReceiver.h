#pragma once

#include "LiveIOBus.h"

#include <string>
#include <vector>

namespace myworld
{
struct LiveIOOscReceiverConfig
{
    std::string host = "127.0.0.1";
    int port = 0;
    std::string address = "/my-world/loudness";
    std::string valueId = "osc.loudness";
};

struct LiveIOOscFloatPacket
{
    bool ok = false;
    std::string status;
    std::string message;
    std::string address;
    float floatValue = 0.0f;
};

struct LiveIOOscReceiverOpenResult
{
    bool ok = false;
    std::string status;
    std::string message;
    int port = 0;
};

struct LiveIOOscReceiverPollResult
{
    bool ok = false;
    bool received = false;
    std::string status;
    std::string message;
    LiveIOOscFloatPacket packet;
};

std::vector<unsigned char> makeLiveIOOscFloatDatagram (const std::string& address, float value);
LiveIOOscFloatPacket decodeLiveIOOscFloatDatagram (const std::vector<unsigned char>& bytes);
LiveIOValueFrame makeLiveIOValueFrameFromOscReceive (const LiveIOOscReceiverConfig& config,
                                                     const LiveIOOscFloatPacket& packet);

class LiveIOOscReceiver
{
public:
    ~LiveIOOscReceiver();

    LiveIOOscReceiverOpenResult open (LiveIOOscReceiverConfig config);
    LiveIOOscReceiverPollResult pollOnce (int timeoutMs);
    void close();

    bool isOpen() const;
    int boundPort() const;
    const LiveIOOscReceiverConfig& config() const;

private:
    int socketFd = -1;
    int port = 0;
    LiveIOOscReceiverConfig receiverConfig;
};
}
