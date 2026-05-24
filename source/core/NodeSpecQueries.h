#pragma once

#include "GraphContract.h"
#include "NodeSpec.h"

#include <string>
#include <vector>

namespace myworld
{
std::string outputDataTypeForEndpoint (const GraphContract& graph,
                                       const std::vector<NodeSpec>& specs,
                                       const std::string& endpoint);
bool canCreateFromEndpoint (const NodeSpec& spec, const std::string& sourceDataType);
bool nodeSpecMatchesFilter (const NodeSpec& spec, const std::string& filter);
std::string makeNodeIdStem (const std::string& nodeType);
std::string makeUniqueNodeId (const GraphContract& graph, const std::string& nodeType);
std::string primaryDataTypeForSpec (const NodeSpec* spec);
}
