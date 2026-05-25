#pragma once

#include "GraphContract.h"
#include "NodeSpec.h"

#include <string>
#include <vector>

namespace myworld
{
enum class ParameterRowKind
{
    parameter,
    input
};

enum class ParameterRowValueState
{
    defaultValue,
    manual,
    connected,
    animated,
    missing
};

struct ParameterRowState
{
    ParameterRowKind kind = ParameterRowKind::parameter;
    ParameterRowValueState valueState = ParameterRowValueState::missing;
    std::string id;
    std::string label;
    std::string value;
    std::string stateLabel;
    std::string dataType;
};

std::string parameterRowStateLabel (ParameterRowValueState state);
ParameterRowState parameterRowForParam (const GraphNode& node, const ParamSpec& param);
ParameterRowState parameterRowForInput (const GraphContract& graph, const GraphNode& node, const PortSpec& input);
std::vector<ParameterRowState> parameterRowsForNode (const GraphContract& graph,
                                                     const GraphNode& node,
                                                     const NodeSpec& spec);
}
