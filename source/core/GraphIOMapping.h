#pragma once

#include "LiveIOBus.h"

#include <string>
#include <vector>

namespace myworld
{
struct GraphIOMappingEndpoint
{
    std::string endpoint;
    std::string dataType;
    std::string streamKind;
};

struct GraphIOMappingTarget
{
    std::string id;
    std::string kind;
    std::string uniformName;
    std::string dataType;
};

struct GraphIOMapping
{
    std::string id;
    GraphIOMappingEndpoint source;
    GraphIOMappingTarget target;
    double inputMin = 0.0;
    double inputMax = 1.0;
};

struct GraphIOMappingValidation
{
    bool ok = false;
    std::string message;
    std::vector<std::string> diagnostics;
};

struct GraphIOMappingReport
{
    bool ok = false;
    std::string status;
    std::string message;
    GraphIOMapping mapping;
    std::vector<LiveIOEvent> events;
    std::vector<std::string> diagnostics;
};

GraphIOMappingValidation validateGraphIOMapping (const GraphIOMapping& mapping);
LiveIOBinding makeLiveIOBindingFromGraphIOMapping (const GraphIOMapping& mapping);
GraphIOMappingReport evaluateGraphIOMapping (const GraphIOMapping& mapping,
                                             const LiveIOValueFrame& frame);
std::string makeGraphIOMappingReportJson (const GraphIOMappingReport& report);
}
