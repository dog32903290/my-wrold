#pragma once

#include "LiveIOBus.h"

#include <string>
#include <vector>

namespace myworld
{
struct LiveIOSendRoute
{
    std::string mode = "dry_run";
    std::string midiOutputName;
    std::string oscHost;
    int oscPort = 0;
};

struct LiveIOSendAction
{
    std::string bindingId;
    std::string sourceId;
    LiveIOTargetKind targetKind = LiveIOTargetKind::oscFloat;
    std::string mode = "dry_run";
    bool sent = false;
    std::string midiOutputName;
    int midiChannel = 1;
    int midiCc = 0;
    int midiValue = 0;
    std::string oscHost;
    int oscPort = 0;
    std::string oscAddress;
    double floatValue = 0.0;
};

struct LiveIOSendReport
{
    bool ok = false;
    std::string status;
    std::string message;
    std::vector<LiveIOSendAction> actions;
    std::vector<std::string> skipped;
    std::vector<std::string> errors;
};

LiveIOSendRoute makeLiveIODryRunSendRoute (const std::string& midiOutputName,
                                           const std::string& oscHost,
                                           int oscPort);

LiveIOSendReport evaluateLiveIOSendBoundary (const LiveIOBusReport& busReport,
                                             const LiveIOSendRoute& route);

std::string makeLiveIOSendReportJson (const LiveIOSendReport& report);
}
