#include "LiveIOMidiSendProof.h"

#include <algorithm>
#include <sstream>

namespace myworld
{
namespace
{
int clampInt (int value, int low, int high)
{
    return std::clamp (value, low, high);
}

std::string jsonQuoted (const std::string& text)
{
    std::ostringstream out;
    out << '"';

    for (const auto character : text)
    {
        switch (character)
        {
            case '\\': out << "\\\\"; break;
            case '"':  out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:   out << character; break;
        }
    }

    out << '"';
    return out.str();
}

void appendErrorsJson (std::ostringstream& out, const std::vector<std::string>& errors)
{
    out << "[";

    for (size_t index = 0; index < errors.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << jsonQuoted (errors[index]);
    }

    out << "]";
}

std::string midiMessageKindToString (LiveIOMidiMessageKind kind)
{
    switch (kind)
    {
        case LiveIOMidiMessageKind::controlChange: return "control_change";
        case LiveIOMidiMessageKind::noteOn:        return "note_on";
    }

    return "control_change";
}

LiveIOMidiMessage sanitizeMessage (LiveIOMidiMessage message)
{
    message.channel = clampInt (message.channel, 1, 16);
    message.cc = clampInt (message.cc, 0, 127);
    message.value = clampInt (message.value, 0, 127);
    message.note = clampInt (message.note, 0, 127);
    message.velocity = clampInt (message.velocity, 0, 127);
    return message;
}

void fillMessageFields (LiveIOMidiOutputSendReport& report, const LiveIOMidiMessage& rawMessage)
{
    const auto message = sanitizeMessage (rawMessage);
    report.bindingId = message.bindingId;
    report.messageKind = midiMessageKindToString (message.kind);
    report.channel = message.channel;
    report.cc = message.cc;
    report.value = message.value;
    report.note = message.note;
    report.velocity = message.velocity;

    if (message.kind == LiveIOMidiMessageKind::noteOn)
    {
        report.statusByte = 0x90 + (message.channel - 1);
        report.data1 = message.note;
        report.data2 = message.velocity;
        return;
    }

    report.statusByte = 0xb0 + (message.channel - 1);
    report.data1 = message.cc;
    report.data2 = message.value;
}

const LiveIOMidiOutputDevice* findDeviceByIdentifier (const LiveIOMidiOutputInventory& inventory,
                                                      const std::string& identifier)
{
    for (const auto& device : inventory.devices)
    {
        if (device.identifier == identifier)
            return &device;
    }

    return nullptr;
}
}

LiveIOMidiOutputSendRequest makeLiveIOMidiOutputSendRequestByIdentifier (
    const std::string& identifier,
    const LiveIOMidiMessage& message)
{
    LiveIOMidiOutputSendRequest request;
    request.route = makeLiveIOMidiOutputRouteByIdentifier (identifier);
    request.message = message;
    return request;
}

LiveIOMidiMessage makeLiveIOMidiNoteOnMessage (const std::string& bindingId,
                                               int channel,
                                               int note,
                                               int velocity)
{
    LiveIOMidiMessage message;
    message.bindingId = bindingId;
    message.kind = LiveIOMidiMessageKind::noteOn;
    message.channel = channel;
    message.note = note;
    message.velocity = velocity;
    return message;
}

LiveIOMidiOutputSendReport executeLiveIOMidiOutputSendProof (
    const LiveIOMidiOutputInventory& inventory,
    const LiveIOMidiOutputSendRequest& request,
    const LiveIOMidiOutputSender& sender)
{
    LiveIOMidiOutputSendReport report;
    fillMessageFields (report, request.message);

    const auto routeReport = evaluateLiveIOMidiOutputRoute (inventory, request.route);
    if (! routeReport.ok)
    {
        report.ok = false;
        report.status = routeReport.status;
        report.message = routeReport.message;
        report.errors = routeReport.errors;
        return report;
    }

    const auto* selected = findDeviceByIdentifier (inventory, routeReport.selectedIdentifier);
    if (selected == nullptr)
    {
        report.ok = false;
        report.status = "unavailable";
        report.message = "midi output is unavailable: " + routeReport.selectedIdentifier;
        report.errors.push_back (report.message);
        return report;
    }

    report.selectedName = selected->name;
    report.selectedIdentifier = selected->identifier;

    if (! sender)
    {
        report.ok = false;
        report.status = "sender_unavailable";
        report.message = "midi output sender is unavailable";
        report.errors.push_back (report.message);
        return report;
    }

    const auto sendResult = sender (*selected, sanitizeMessage (request.message));
    report.opened = sendResult.opened;
    report.sent = sendResult.sent;

    if (! sendResult.opened)
    {
        report.ok = false;
        report.status = "open_failed";
        report.message = sendResult.error.empty() ? "midi output open failed" : sendResult.error;
        report.errors.push_back (report.message);
        return report;
    }

    if (! sendResult.sent)
    {
        report.ok = false;
        report.status = "send_failed";
        report.message = sendResult.error.empty() ? "midi output send failed" : sendResult.error;
        report.errors.push_back (report.message);
        return report;
    }

    report.ok = true;
    report.status = "sent";
    report.message = report.messageKind == "note_on" ? "midi_output_note_on_sent" : "midi_output_cc_sent";
    return report;
}

std::string makeLiveIOMidiOutputSendReportJson (const LiveIOMidiOutputSendReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"liveIOMidiOutputSendProof\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (report.status) << ",\n";
    out << "  \"message\": " << jsonQuoted (report.message) << ",\n";
    out << "  \"selectedName\": " << jsonQuoted (report.selectedName) << ",\n";
    out << "  \"selectedIdentifier\": " << jsonQuoted (report.selectedIdentifier) << ",\n";
    out << "  \"opened\": " << (report.opened ? "true" : "false") << ",\n";
    out << "  \"sent\": " << (report.sent ? "true" : "false") << ",\n";
    out << "  \"bindingId\": " << jsonQuoted (report.bindingId) << ",\n";
    out << "  \"messageKind\": " << jsonQuoted (report.messageKind) << ",\n";
    out << "  \"channel\": " << report.channel << ",\n";
    out << "  \"cc\": " << report.cc << ",\n";
    out << "  \"value\": " << report.value << ",\n";
    out << "  \"note\": " << report.note << ",\n";
    out << "  \"velocity\": " << report.velocity << ",\n";
    out << "  \"statusByte\": " << report.statusByte << ",\n";
    out << "  \"data1\": " << report.data1 << ",\n";
    out << "  \"data2\": " << report.data2 << ",\n";
    out << "  \"errors\": ";
    appendErrorsJson (out, report.errors);
    out << "\n";
    out << "}\n";
    return out.str();
}
}
