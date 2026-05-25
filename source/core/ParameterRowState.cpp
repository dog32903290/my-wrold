#include "ParameterRowState.h"

#include <algorithm>

namespace myworld
{
namespace
{
const GraphNode::ParamValue* findParamValue (const GraphNode& node, const std::string& paramId)
{
    const auto found = std::find_if (node.params.begin(), node.params.end(), [&paramId] (const auto& param) {
        return param.id == paramId;
    });

    return found == node.params.end() ? nullptr : &*found;
}

const ParamSpec* findParamSpec (const std::vector<ParamSpec>& params, const std::string& paramId)
{
    const auto found = std::find_if (params.begin(), params.end(), [&paramId] (const auto& param) {
        return param.id == paramId;
    });

    return found == params.end() ? nullptr : &*found;
}

const GraphNode::PortBindingValue* findPortBindingValue (const GraphNode& node, const std::string& portId)
{
    const auto found = std::find_if (node.portBindings.begin(), node.portBindings.end(), [&portId] (const auto& binding) {
        return binding.portId == portId;
    });

    return found == node.portBindings.end() ? nullptr : &*found;
}

const GraphEdge* findIncomingEdge (const GraphContract& graph, const std::string& endpoint)
{
    const auto found = std::find_if (graph.editorGraph.edges.begin(), graph.editorGraph.edges.end(), [&endpoint] (const auto& edge) {
        return edge.to == endpoint;
    });

    return found == graph.editorGraph.edges.end() ? nullptr : &*found;
}

ParameterRowValueState stateForBindingMode (const std::string& bindingMode)
{
    if (bindingMode == "connected")
        return ParameterRowValueState::connected;

    if (bindingMode == "animated")
        return ParameterRowValueState::animated;

    if (bindingMode == "manual")
        return ParameterRowValueState::manual;

    return ParameterRowValueState::defaultValue;
}

std::string paramValueOrDefault (const GraphNode& node,
                                 const std::vector<ParamSpec>& params,
                                 const std::string& paramId)
{
    if (const auto* stored = findParamValue (node, paramId))
        return stored->value;

    if (const auto* spec = findParamSpec (params, paramId))
        return spec->defaultValue;

    return {};
}

bool paramIsVisibleForNode (const GraphNode& node,
                            const std::vector<ParamSpec>& params,
                            const ParamSpec& param)
{
    if (param.visibleWhenParamId.empty())
        return true;

    const auto controller = paramValueOrDefault (node, params, param.visibleWhenParamId);
    if (controller.empty())
        return true;

    return controller == param.visibleWhenValue;
}
}

std::string parameterRowStateLabel (ParameterRowValueState state)
{
    switch (state)
    {
        case ParameterRowValueState::defaultValue: return "default";
        case ParameterRowValueState::manual:       return "manual";
        case ParameterRowValueState::connected:    return "connected";
        case ParameterRowValueState::animated:     return "animated";
        case ParameterRowValueState::missing:      return "missing";
    }

    return "missing";
}

ParameterRowState parameterRowForParam (const GraphNode& node, const ParamSpec& param)
{
    const auto* stored = findParamValue (node, param.id);
    const auto valueState = stored == nullptr ? ParameterRowValueState::defaultValue
                                              : ParameterRowValueState::manual;

    return {
        ParameterRowKind::parameter,
        valueState,
        "param." + param.id,
        param.label.empty() ? param.id : param.label,
        stored == nullptr ? param.defaultValue : stored->value,
        parameterRowStateLabel (valueState),
        param.dataType,
        param.group,
        param.description,
        param.excludeFromPresets
    };
}

ParameterRowState parameterRowForInput (const GraphContract& graph, const GraphNode& node, const PortSpec& input)
{
    const auto endpoint = node.id + "." + input.id;
    if (const auto* edge = findIncomingEdge (graph, endpoint))
    {
        return {
            ParameterRowKind::input,
            ParameterRowValueState::connected,
            "input." + input.id,
            input.label.empty() ? input.id : input.label,
            edge->from,
            parameterRowStateLabel (ParameterRowValueState::connected),
            input.dataType
        };
    }

    const auto* binding = findPortBindingValue (node, input.id);
    const auto valueState = binding == nullptr ? ParameterRowValueState::defaultValue
                                               : stateForBindingMode (binding->bindingMode);
    const auto value = binding == nullptr || valueState == ParameterRowValueState::defaultValue
                           ? input.dataType
                           : binding->value;

    return {
        ParameterRowKind::input,
        valueState,
        "input." + input.id,
        input.label.empty() ? input.id : input.label,
        value,
        parameterRowStateLabel (valueState),
        input.dataType
    };
}

std::vector<ParameterRowState> parameterRowsForNode (const GraphContract& graph,
                                                     const GraphNode& node,
                                                     const NodeSpec& spec)
{
    std::vector<ParameterRowState> rows;
    rows.reserve (spec.params.size() + spec.inputs.size());

    for (const auto& param : spec.params)
        if (paramIsVisibleForNode (node, spec.params, param))
            rows.push_back (parameterRowForParam (node, param));

    for (const auto& input : spec.inputs)
        rows.push_back (parameterRowForInput (graph, node, input));

    return rows;
}
}
