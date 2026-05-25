#pragma once

#include "GraphIOMapping.h"

#include <string>
#include <vector>

namespace myworld
{
struct GraphIOMappingLoadResult
{
    bool ok = false;
    std::vector<GraphIOMapping> mappings;
    std::string error;
};

GraphIOMappingLoadResult loadGraphIOMappingsFromJsonText (const std::string& text);
GraphIOMappingLoadResult loadGraphIOMappingsFromFile (const std::string& path);
}
