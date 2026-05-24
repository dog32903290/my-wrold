#include "NodeSpecQueries.h"

#include "GraphEndpoint.h"

#include <cctype>

namespace myworld
{
std::string outputDataTypeForEndpoint (const GraphContract& graph,
                                       const std::vector<NodeSpec>& specs,
                                       const std::string& endpoint)
{
    const auto* spec = specForNode (graph, specs, nodeIdFromEndpoint (endpoint));

    if (spec == nullptr)
        return {};

    const auto portId = portIdFromEndpoint (endpoint);
    for (const auto& port : spec->outputs)
        if (port.id == portId)
            return port.dataType;

    return {};
}

bool canCreateFromEndpoint (const NodeSpec& spec, const std::string& sourceDataType)
{
    return ! spec.inputs.empty() && spec.inputs.front().dataType == sourceDataType;
}

bool nodeSpecMatchesFilter (const NodeSpec& spec, const std::string& filter)
{
    if (filter.empty())
        return true;

    const auto needle = makeNodeIdStem (filter);
    const auto haystack = makeNodeIdStem (spec.type + " " + spec.displayName + " " + spec.category + " " + spec.subcategory);
    return haystack.find (needle) != std::string::npos;
}

std::string makeNodeIdStem (const std::string& nodeType)
{
    std::string stem;

    for (const auto c : nodeType)
    {
        if (std::isalnum (static_cast<unsigned char> (c)))
            stem.push_back (static_cast<char> (std::tolower (static_cast<unsigned char> (c))));
        else if (! stem.empty() && stem.back() != '_')
            stem.push_back ('_');
    }

    while (! stem.empty() && stem.back() == '_')
        stem.pop_back();

    return stem.empty() ? "node" : stem;
}

std::string makeUniqueNodeId (const GraphContract& graph, const std::string& nodeType)
{
    const auto stem = makeNodeIdStem (nodeType);

    for (int index = 1; index < 1000; ++index)
    {
        const auto candidate = stem + std::to_string (index);
        if (findEditorNode (graph, candidate) == nullptr)
            return candidate;
    }

    return stem + "_overflow";
}

std::string primaryDataTypeForSpec (const NodeSpec* spec)
{
    if (spec == nullptr)
        return {};

    if (! spec->outputs.empty())
        return spec->outputs.front().dataType;

    if (! spec->inputs.empty())
        return spec->inputs.front().dataType;

    return {};
}
}
