#include "LiveIOMidiOutputInventory.h"

#include <cstdlib>
#include <iostream>
#include <string>

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

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}
}

int main()
{
    myworld::LiveIOMidiOutputInventory inventory;
    inventory.devices = {
        { "IAC Driver Bus 1", "iac-1" },
        { "Hardware Synth", "hw-synth" }
    };

    const auto byIdentifier = myworld::evaluateLiveIOMidiOutputRoute (
        inventory,
        myworld::makeLiveIOMidiOutputRouteByIdentifier ("iac-1"));
    expect (byIdentifier.ok, byIdentifier.message);
    expectEqual (byIdentifier.status, "selected", "identifier route status");
    expectEqual (byIdentifier.selectedName, "IAC Driver Bus 1", "identifier route name");
    expectEqual (byIdentifier.selectedIdentifier, "iac-1", "identifier route id");

    const auto byName = myworld::evaluateLiveIOMidiOutputRoute (
        inventory,
        myworld::makeLiveIOMidiOutputRouteByName ("Hardware Synth"));
    expect (byName.ok, byName.message);
    expectEqual (byName.status, "selected", "name route status");
    expectEqual (byName.selectedName, "Hardware Synth", "name route name");
    expectEqual (byName.selectedIdentifier, "hw-synth", "name route id");

    const auto missing = myworld::evaluateLiveIOMidiOutputRoute (
        inventory,
        myworld::makeLiveIOMidiOutputRouteByIdentifier ("missing-output"));
    expect (! missing.ok, "missing route should fail");
    expectEqual (missing.status, "unavailable", "missing route status");
    expectEqual (static_cast<int> (missing.errors.size()), 1, "missing route error count");
    expectContains (missing.errors.front(), "midi output is unavailable: missing-output",
                    "missing route error");

    myworld::LiveIOMidiOutputInventory emptyInventory;
    const auto empty = myworld::evaluateLiveIOMidiOutputRoute (
        emptyInventory,
        myworld::makeLiveIOMidiOutputRouteByName ("IAC Driver Bus 1"));
    expect (! empty.ok, "empty inventory route should fail");
    expectEqual (empty.status, "unavailable", "empty inventory route status");

    const auto json = myworld::makeLiveIOMidiOutputInventoryReportJson (
        inventory,
        byIdentifier,
        missing);
    expectContains (json, "\"kind\": \"liveIOMidiOutputInventoryProof\"", "inventory json");
    expectContains (json, "\"ok\": true", "inventory json");
    expectContains (json, "\"deviceCount\": 2", "inventory json");
    expectContains (json, "\"name\": \"IAC Driver Bus 1\"", "inventory json");
    expectContains (json, "\"identifier\": \"iac-1\"", "inventory json");
    expectContains (json, "\"selectedRoute\": {", "inventory json");
    expectContains (json, "\"status\": \"selected\"", "inventory json");
    expectContains (json, "\"unavailableRoute\": {", "inventory json");
    expectContains (json, "\"status\": \"unavailable\"", "inventory json");
    expectContains (json, "\"errors\": [\"midi output is unavailable: missing-output\"]",
                    "inventory json");

    std::cout << "live io midi output inventory ok\n";
    return 0;
}
