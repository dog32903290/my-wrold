#pragma once

#include "LiveIOMidiOutputInventory.h"

#include <functional>
#include <string>
#include <vector>

namespace myworld
{
struct LiveIOMidiCcMessage
{
    std::string bindingId;
    int channel = 1;
    int cc = 0;
    int value = 0;
};

struct LiveIOMidiOutputSendRequest
{
    LiveIOMidiOutputRouteRequest route;
    LiveIOMidiCcMessage message;
};

struct LiveIOMidiOutputDeviceSendResult
{
    bool opened = false;
    bool sent = false;
    std::string error;
};

struct LiveIOMidiOutputSendReport
{
    bool ok = false;
    std::string status;
    std::string message;
    std::string selectedName;
    std::string selectedIdentifier;
    bool opened = false;
    bool sent = false;
    std::string bindingId;
    int channel = 1;
    int cc = 0;
    int value = 0;
    int statusByte = 0;
    int data1 = 0;
    int data2 = 0;
    std::vector<std::string> errors;
};

using LiveIOMidiOutputSender = std::function<LiveIOMidiOutputDeviceSendResult (
    const LiveIOMidiOutputDevice& device,
    const LiveIOMidiCcMessage& message)>;

LiveIOMidiOutputSendRequest makeLiveIOMidiOutputSendRequestByIdentifier (
    const std::string& identifier,
    const LiveIOMidiCcMessage& message);

LiveIOMidiOutputSendReport executeLiveIOMidiOutputSendProof (
    const LiveIOMidiOutputInventory& inventory,
    const LiveIOMidiOutputSendRequest& request,
    const LiveIOMidiOutputSender& sender);

std::string makeLiveIOMidiOutputSendReportJson (const LiveIOMidiOutputSendReport& report);
}
