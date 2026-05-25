#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct LiveIOMidiOutputDevice
{
    std::string name;
    std::string identifier;
};

struct LiveIOMidiOutputInventory
{
    std::vector<LiveIOMidiOutputDevice> devices;
};

struct LiveIOMidiOutputRouteRequest
{
    std::string preferredName;
    std::string preferredIdentifier;
};

struct LiveIOMidiOutputRouteReport
{
    bool ok = false;
    std::string status;
    std::string message;
    std::string preferredName;
    std::string preferredIdentifier;
    std::string selectedName;
    std::string selectedIdentifier;
    std::vector<std::string> errors;
};

LiveIOMidiOutputRouteRequest makeLiveIOMidiOutputRouteByName (const std::string& name);
LiveIOMidiOutputRouteRequest makeLiveIOMidiOutputRouteByIdentifier (const std::string& identifier);

LiveIOMidiOutputRouteReport evaluateLiveIOMidiOutputRoute (
    const LiveIOMidiOutputInventory& inventory,
    const LiveIOMidiOutputRouteRequest& request);

std::string makeLiveIOMidiOutputInventoryReportJson (
    const LiveIOMidiOutputInventory& inventory,
    const LiveIOMidiOutputRouteReport& selectedRoute,
    const LiveIOMidiOutputRouteReport& unavailableRoute);
}
