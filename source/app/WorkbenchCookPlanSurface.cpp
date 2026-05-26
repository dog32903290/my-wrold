#include "WorkbenchCookPlanSurface.h"

#include <algorithm>
#include <string>

namespace myworld
{
namespace
{
std::string fallback (const std::string& value, const std::string& fallbackValue)
{
    return value.empty() ? fallbackValue : value;
}

std::string countValue (int nodes, int edges)
{
    return std::to_string (nodes)
           + " nodes/"
           + std::to_string (edges)
           + " edges";
}

std::string endpointNodeId (const std::string& endpoint)
{
    const auto dot = endpoint.find ('.');
    return dot == std::string::npos ? endpoint : endpoint.substr (0, dot);
}

void appendUnique (std::vector<std::string>& values, const std::string& value)
{
    if (value.empty())
        return;

    if (std::find (values.begin(), values.end(), value) == values.end())
        values.push_back (value);
}

std::vector<std::string> cookOrderFor (const WorkbenchSessionSnapshot& snapshot)
{
    std::vector<std::string> order;
    order.reserve (snapshot.runtimeNodes.size());

    for (const auto& edge : snapshot.runtimeEdges)
    {
        appendUnique (order, endpointNodeId (edge.from));
        appendUnique (order, endpointNodeId (edge.to));
    }

    for (const auto& node : snapshot.runtimeNodes)
        appendUnique (order, node.id);

    return order;
}

std::string joinOrder (const std::vector<std::string>& order)
{
    if (order.empty())
        return "none";

    std::string joined;
    for (const auto& id : order)
    {
        if (! joined.empty())
            joined += " -> ";

        joined += id;
    }

    return joined;
}

std::string readinessValue (const WorkbenchSessionSnapshot& snapshot)
{
    if (! snapshot.ok)
        return "blocked";

    if (snapshot.runtimeNodeCount <= 0 || snapshot.runtimeNodes.empty())
        return "no-runtime";

    if (snapshot.activeOutputNodeId.empty())
        return "no-output";

    return "ready";
}

std::string headlineStatus (const std::string& readiness)
{
    return readiness == "ready" ? "ready" : "idle";
}

std::string readinessTone (const std::string& readiness)
{
    if (readiness == "blocked")
        return "blocked";

    if (readiness == "ready")
        return "ready";

    return "idle";
}

WorkbenchCookPlanSurfaceRow makeRow (std::string id,
                                     std::string label,
                                     std::string value,
                                     std::string tone)
{
    WorkbenchCookPlanSurfaceRow row;
    row.id = std::move (id);
    row.label = std::move (label);
    row.value = std::move (value);
    row.text = row.label + " " + row.value;
    row.tone = std::move (tone);
    return row;
}
}

WorkbenchCookPlanSurface makeWorkbenchCookPlanSurface (const WorkbenchSessionSnapshot& snapshot)
{
    WorkbenchCookPlanSurface surface;
    surface.ok = snapshot.ok;
    surface.cookOrder = cookOrderFor (snapshot);

    const auto documentId = fallback (snapshot.documentId,
                                      snapshot.ok ? "untitled cook" : "no workbench session");
    const auto orderText = joinOrder (surface.cookOrder);
    const auto readiness = readinessValue (snapshot);
    const auto target = fallback (snapshot.activeOutputNodeId, "none");

    surface.headline = snapshot.ok
                           ? "cook "
                                 + headlineStatus (readiness)
                                 + " "
                                 + documentId
                                 + " order "
                                 + orderText
                           : "cook blocked: " + fallback (snapshot.message, "no current workbench session");

    surface.rows = {
        makeRow ("cook",
                 "cook",
                 documentId,
                 snapshot.ok ? readinessTone (readiness) : "blocked"),
        makeRow ("order",
                 "order",
                 orderText,
                 surface.cookOrder.empty() ? "idle" : "ready"),
        makeRow ("target",
                 "target",
                 target,
                 target == "none" ? "idle" : "ready"),
        makeRow ("graph",
                 "graph",
                 countValue (snapshot.runtimeNodeCount, snapshot.runtimeEdgeCount),
                 snapshot.runtimeNodeCount > 0 ? "ready" : "idle"),
        makeRow ("readiness",
                 "readiness",
                 readiness,
                 readinessTone (readiness)),
        makeRow ("execution",
                 "execution",
                 "parked",
                 "idle")
    };

    return surface;
}
}
