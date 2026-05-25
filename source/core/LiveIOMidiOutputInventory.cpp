#include "LiveIOMidiOutputInventory.h"

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

std::string routeLabel (const LiveIOMidiOutputRouteRequest& request)
{
    if (! request.preferredIdentifier.empty())
        return request.preferredIdentifier;

    if (! request.preferredName.empty())
        return request.preferredName;

    return "<empty>";
}

const LiveIOMidiOutputDevice* findDevice (const LiveIOMidiOutputInventory& inventory,
                                          const LiveIOMidiOutputRouteRequest& request)
{
    for (const auto& device : inventory.devices)
    {
        if (! request.preferredIdentifier.empty()
            && device.identifier == request.preferredIdentifier)
            return &device;

        if (request.preferredIdentifier.empty()
            && ! request.preferredName.empty()
            && device.name == request.preferredName)
            return &device;
    }

    return nullptr;
}

void appendRouteReportJson (std::ostringstream& out, const LiveIOMidiOutputRouteReport& report)
{
    out << "{\n";
    out << "    \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "    \"status\": " << jsonQuoted (report.status) << ",\n";
    out << "    \"message\": " << jsonQuoted (report.message) << ",\n";
    out << "    \"preferredName\": " << jsonQuoted (report.preferredName) << ",\n";
    out << "    \"preferredIdentifier\": " << jsonQuoted (report.preferredIdentifier) << ",\n";
    out << "    \"selectedName\": " << jsonQuoted (report.selectedName) << ",\n";
    out << "    \"selectedIdentifier\": " << jsonQuoted (report.selectedIdentifier) << ",\n";
    out << "    \"errors\": ";
    appendErrorsJson (out, report.errors);
    out << "\n";
    out << "  }";
}

void appendDevicesJson (std::ostringstream& out, const std::vector<LiveIOMidiOutputDevice>& devices)
{
    out << "[\n";

    for (size_t index = 0; index < devices.size(); ++index)
    {
        const auto& device = devices[index];
        out << "    {\n";
        out << "      \"name\": " << jsonQuoted (device.name) << ",\n";
        out << "      \"identifier\": " << jsonQuoted (device.identifier) << "\n";
        out << "    }";

        if (index + 1 < devices.size())
            out << ",";

        out << "\n";
    }

    out << "  ]";
}
}

LiveIOMidiOutputRouteRequest makeLiveIOMidiOutputRouteByName (const std::string& name)
{
    LiveIOMidiOutputRouteRequest request;
    request.preferredName = name;
    return request;
}

LiveIOMidiOutputRouteRequest makeLiveIOMidiOutputRouteByIdentifier (const std::string& identifier)
{
    LiveIOMidiOutputRouteRequest request;
    request.preferredIdentifier = identifier;
    return request;
}

LiveIOMidiOutputRouteReport evaluateLiveIOMidiOutputRoute (
    const LiveIOMidiOutputInventory& inventory,
    const LiveIOMidiOutputRouteRequest& request)
{
    LiveIOMidiOutputRouteReport report;
    report.preferredName = request.preferredName;
    report.preferredIdentifier = request.preferredIdentifier;

    const auto* selected = findDevice (inventory, request);
    if (selected != nullptr)
    {
        report.ok = true;
        report.status = "selected";
        report.message = "midi_output_selected";
        report.selectedName = selected->name;
        report.selectedIdentifier = selected->identifier;
        return report;
    }

    report.ok = false;
    report.status = "unavailable";
    report.message = "midi output is unavailable: " + routeLabel (request);
    report.errors.push_back (report.message);
    return report;
}

std::string makeLiveIOMidiOutputInventoryReportJson (
    const LiveIOMidiOutputInventory& inventory,
    const LiveIOMidiOutputRouteReport& selectedRoute,
    const LiveIOMidiOutputRouteReport& unavailableRoute)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"liveIOMidiOutputInventoryProof\",\n";
    out << "  \"ok\": true,\n";
    out << "  \"status\": \"inventoried\",\n";
    out << "  \"deviceCount\": " << inventory.devices.size() << ",\n";
    out << "  \"devices\": ";
    appendDevicesJson (out, inventory.devices);
    out << ",\n";
    out << "  \"selectedRoute\": ";
    appendRouteReportJson (out, selectedRoute);
    out << ",\n";
    out << "  \"unavailableRoute\": ";
    appendRouteReportJson (out, unavailableRoute);
    out << "\n";
    out << "}\n";
    return out.str();
}
}
