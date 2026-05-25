#include "LiveIOSendAdapter.h"

#include <arpa/inet.h>
#include <cmath>
#include <cstring>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

namespace myworld
{
namespace
{
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

void appendStringArrayJson (std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";

    for (size_t index = 0; index < values.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << jsonQuoted (values[index]);
    }

    out << "]";
}

void appendActionsJson (std::ostringstream& out, const std::vector<LiveIOSendAction>& actions)
{
    out << "[\n";

    for (size_t index = 0; index < actions.size(); ++index)
    {
        const auto& action = actions[index];
        out << "    {\n";
        out << "      \"bindingId\": " << jsonQuoted (action.bindingId) << ",\n";
        out << "      \"sourceId\": " << jsonQuoted (action.sourceId) << ",\n";
        out << "      \"targetKind\": " << jsonQuoted (liveIOTargetKindToString (action.targetKind)) << ",\n";
        out << "      \"mode\": " << jsonQuoted (action.mode) << ",\n";
        out << "      \"sent\": " << (action.sent ? "true" : "false") << ",\n";
        out << "      \"midiOutputName\": " << jsonQuoted (action.midiOutputName) << ",\n";
        out << "      \"channel\": " << action.midiChannel << ",\n";
        out << "      \"cc\": " << action.midiCc << ",\n";
        out << "      \"midiValue\": " << action.midiValue << ",\n";
        out << "      \"oscHost\": " << jsonQuoted (action.oscHost) << ",\n";
        out << "      \"oscPort\": " << action.oscPort << ",\n";
        out << "      \"oscAddress\": " << jsonQuoted (action.oscAddress) << ",\n";
        out << "      \"floatValue\": " << action.floatValue << "\n";
        out << "    }";

        if (index + 1 < actions.size())
            out << ",";

        out << "\n";
    }

    out << "  ]";
}

bool oscEndpointIsValid (const LiveIOSendRoute& route)
{
    return ! route.oscHost.empty() && route.oscPort > 0 && route.oscPort <= 65535;
}

bool oscHostIsLoopback (const std::string& host)
{
    return host == "127.0.0.1" || host == "localhost";
}

std::string skippedTargetLabel (const LiveIOEvent& event)
{
    return liveIOTargetKindToString (event.targetKind) + ": " + event.bindingId;
}

std::string disabledTargetLabel (const char* prefix, const LiveIOEvent& event)
{
    return std::string (prefix) + ": " + event.bindingId;
}

LiveIOSendAction makeMidiAction (const LiveIOEvent& event, const LiveIOSendRoute& route)
{
    LiveIOSendAction action;
    action.bindingId = event.bindingId;
    action.sourceId = event.sourceId;
    action.targetKind = event.targetKind;
    action.mode = route.mode;
    action.sent = false;
    action.midiOutputName = route.midiOutputName;
    action.midiChannel = event.midiChannel;
    action.midiCc = event.midiCc;
    action.midiValue = event.midiValue;
    return action;
}

LiveIOSendAction makeOscAction (const LiveIOEvent& event, const LiveIOSendRoute& route)
{
    LiveIOSendAction action;
    action.bindingId = event.bindingId;
    action.sourceId = event.sourceId;
    action.targetKind = event.targetKind;
    action.mode = route.mode;
    action.sent = false;
    action.oscHost = route.oscHost;
    action.oscPort = route.oscPort;
    action.oscAddress = event.oscAddress;
    action.floatValue = event.floatValue;
    return action;
}

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

std::vector<unsigned char> makeOscFloatMessage (const std::string& address, double value)
{
    std::vector<unsigned char> bytes;
    appendPaddedOscString (bytes, address);
    appendPaddedOscString (bytes, ",f");
    appendOscFloat (bytes, static_cast<float> (value));
    return bytes;
}

std::string sendOscFloatDatagram (const LiveIOSendAction& action)
{
    if (action.oscAddress.empty())
        return "osc address is required: " + action.bindingId;

    if (! std::isfinite (action.floatValue))
        return "osc float is not finite: " + action.bindingId;

    const auto socketFd = ::socket (AF_INET, SOCK_DGRAM, 0);
    if (socketFd < 0)
        return "could not open osc udp socket: " + action.bindingId;

    sockaddr_in destination {};
    destination.sin_family = AF_INET;
    destination.sin_port = htons (static_cast<uint16_t> (action.oscPort));

    const auto host = action.oscHost == "localhost" ? std::string ("127.0.0.1") : action.oscHost;
    if (::inet_pton (AF_INET, host.c_str(), &destination.sin_addr) != 1)
    {
        ::close (socketFd);
        return "could not resolve osc host: " + action.bindingId;
    }

    const auto message = makeOscFloatMessage (action.oscAddress, action.floatValue);
    const auto sent = ::sendto (socketFd,
                                message.data(),
                                message.size(),
                                0,
                                reinterpret_cast<const sockaddr*> (&destination),
                                sizeof (destination));
    ::close (socketFd);

    if (sent != static_cast<ssize_t> (message.size()))
        return "could not send osc packet: " + action.bindingId;

    return {};
}
}

LiveIOSendRoute makeLiveIODryRunSendRoute (const std::string& midiOutputName,
                                           const std::string& oscHost,
                                           int oscPort)
{
    LiveIOSendRoute route;
    route.mode = "dry_run";
    route.midiOutputName = midiOutputName;
    route.oscHost = oscHost;
    route.oscPort = oscPort;
    return route;
}

LiveIOSendRoute makeLiveIOControlledOscLoopbackRoute (const std::string& oscHost,
                                                      int oscPort)
{
    LiveIOSendRoute route;
    route.mode = "controlled_send";
    route.oscHost = oscHost;
    route.oscPort = oscPort;
    route.midiEnabled = false;
    route.oscEnabled = true;
    return route;
}

LiveIOSendReport evaluateLiveIOSendBoundary (const LiveIOBusReport& busReport,
                                             const LiveIOSendRoute& route)
{
    LiveIOSendReport report;

    if (! busReport.ok)
    {
        report.status = "blocked";
        report.message = "live io bus is not mapped: " + busReport.message;
        report.errors.push_back (report.message);
        return report;
    }

    for (const auto& event : busReport.events)
    {
        if (event.targetKind == LiveIOTargetKind::shaderUniform)
        {
            report.skipped.push_back (skippedTargetLabel (event));
            continue;
        }

        if (event.targetKind == LiveIOTargetKind::midiCc)
        {
            if (! route.midiEnabled)
            {
                report.skipped.push_back (disabledTargetLabel ("midi.disabled", event));
                continue;
            }

            if (route.midiOutputName.empty())
            {
                report.errors.push_back ("midi output is required: " + event.bindingId);
                continue;
            }

            report.actions.push_back (makeMidiAction (event, route));
            continue;
        }

        if (event.targetKind == LiveIOTargetKind::oscFloat)
        {
            if (! route.oscEnabled)
            {
                report.skipped.push_back (disabledTargetLabel ("osc.disabled", event));
                continue;
            }

            if (! oscEndpointIsValid (route))
            {
                report.errors.push_back ("osc endpoint is required: " + event.bindingId);
                continue;
            }

            report.actions.push_back (makeOscAction (event, route));
        }
    }

    report.ok = report.errors.empty();
    report.status = report.ok ? route.mode : "blocked";
    report.message = report.ok ? "live_io_send_boundary_dry_run" : report.errors.front();
    return report;
}

LiveIOSendReport executeLiveIOSendBoundary (const LiveIOBusReport& busReport,
                                            const LiveIOSendRoute& route)
{
    auto report = evaluateLiveIOSendBoundary (busReport, route);
    if (! report.ok)
        return report;

    if (route.mode != "controlled_send")
        return report;

    if (! oscHostIsLoopback (route.oscHost))
    {
        report.ok = false;
        report.status = "blocked";
        report.message = "controlled osc send requires loopback host";
        report.errors.push_back (report.message);
        return report;
    }

    for (auto& action : report.actions)
    {
        if (action.targetKind != LiveIOTargetKind::oscFloat)
            continue;

        if (const auto error = sendOscFloatDatagram (action); ! error.empty())
        {
            report.ok = false;
            report.status = "blocked";
            report.message = error;
            report.errors.push_back (error);
            return report;
        }

        action.sent = true;
    }

    report.ok = report.errors.empty();
    report.status = report.ok ? route.mode : "blocked";
    report.message = report.ok ? "live_io_osc_loopback_sent" : report.errors.front();
    return report;
}

std::string makeLiveIOSendReportJson (const LiveIOSendReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"liveIOSendReport\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (report.status) << ",\n";
    out << "  \"message\": " << jsonQuoted (report.message) << ",\n";
    out << "  \"actions\": ";
    appendActionsJson (out, report.actions);
    out << ",\n";
    out << "  \"skipped\": ";
    appendStringArrayJson (out, report.skipped);
    out << ",\n";
    out << "  \"errors\": ";
    appendStringArrayJson (out, report.errors);
    out << "\n";
    out << "}\n";
    return out.str();
}
}
