#pragma once

#include "LiveIOMidiOutputInventory.h"

#include <functional>
#include <string>
#include <vector>

namespace myworld
{
enum class LiveIOMidiMessageKind
{
    controlChange,
    noteOn
};

struct LiveIOMidiMessage
{
    std::string bindingId;
    LiveIOMidiMessageKind kind = LiveIOMidiMessageKind::controlChange;
    int channel = 1;
    int cc = 0;
    int value = 0;
    int note = 60;
    int velocity = 0;
};

using LiveIOMidiCcMessage = LiveIOMidiMessage;

struct LiveIOMidiOutputSendRequest
{
    LiveIOMidiOutputRouteRequest route;
    LiveIOMidiMessage message;
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
    std::string messageKind = "control_change";
    int channel = 1;
    int cc = 0;
    int value = 0;
    int note = 60;
    int velocity = 0;
    int statusByte = 0;
    int data1 = 0;
    int data2 = 0;
    std::vector<std::string> errors;
};

using LiveIOMidiOutputSender = std::function<LiveIOMidiOutputDeviceSendResult (
    const LiveIOMidiOutputDevice& device,
    const LiveIOMidiMessage& message)>;

LiveIOMidiOutputSendRequest makeLiveIOMidiOutputSendRequestByIdentifier (
    const std::string& identifier,
    const LiveIOMidiMessage& message);

LiveIOMidiMessage makeLiveIOMidiNoteOnMessage (const std::string& bindingId,
                                               int channel,
                                               int note,
                                               int velocity);

LiveIOMidiOutputSendReport executeLiveIOMidiOutputSendProof (
    const LiveIOMidiOutputInventory& inventory,
    const LiveIOMidiOutputSendRequest& request,
    const LiveIOMidiOutputSender& sender);

std::string makeLiveIOMidiOutputSendReportJson (const LiveIOMidiOutputSendReport& report);
}
