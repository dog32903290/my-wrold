#include "LiveIOSendAdapter.h"

#include <sstream>

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

std::string skippedTargetLabel (const LiveIOEvent& event)
{
    return liveIOTargetKindToString (event.targetKind) + ": " + event.bindingId;
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
