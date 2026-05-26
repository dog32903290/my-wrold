#include "WorkbenchCanvasSurface.h"

#include "CanvasGeometry.h"

#include <cmath>
#include <sstream>
#include <string>

namespace myworld
{
namespace
{
std::string fallback (const std::string& value, const std::string& fallbackValue)
{
    return value.empty() ? fallbackValue : value;
}

std::string endpointNodeId (const std::string& endpoint)
{
    const auto dot = endpoint.find ('.');
    return dot == std::string::npos ? endpoint : endpoint.substr (0, dot);
}

std::string numberText (double value)
{
    const auto rounded = std::round (value);
    if (std::abs (value - rounded) < 0.0001)
        return std::to_string (static_cast<int> (rounded));

    std::ostringstream out;
    out << value;
    return out.str();
}

std::string boundsValue (const WorkbenchCanvasNodeSurface& node)
{
    if (node.id.empty())
        return "none";

    return node.id
           + " "
           + numberText (node.x)
           + ","
           + numberText (node.y)
           + " "
           + numberText (node.width)
           + "x"
           + numberText (node.height);
}

std::string routeValue (const WorkbenchCanvasEdgeRoute& route)
{
    if (route.id.empty())
        return "none";

    return route.from + " -> " + route.to;
}

std::string pointsValue (const WorkbenchCanvasEdgeRoute& route)
{
    if (route.id.empty())
        return "none";

    return numberText (route.fromX)
           + ","
           + numberText (route.fromY)
           + " -> "
           + numberText (route.toX)
           + ","
           + numberText (route.toY);
}

GraphNode graphNodeForSummary (const WorkbenchSessionSnapshot::GraphNodeSummary& summary)
{
    GraphNode node;
    node.id = summary.id;
    node.type = summary.type;
    node.position = { summary.x, summary.y };
    node.collapsed = summary.collapsed;
    return node;
}

WorkbenchCanvasNodeSurface makeNodeSurface (const WorkbenchSessionSnapshot::GraphNodeSummary& summary)
{
    const auto bounds = canvasNodeBounds (graphNodeForSummary (summary));
    return {
        summary.id,
        summary.type,
        bounds.x,
        bounds.y,
        bounds.width,
        bounds.height
    };
}

const WorkbenchCanvasNodeSurface* findNodeSurface (const std::vector<WorkbenchCanvasNodeSurface>& nodes,
                                                  const std::string& id)
{
    for (const auto& node : nodes)
    {
        if (node.id == id)
            return &node;
    }

    return nullptr;
}

WorkbenchCanvasEdgeRoute makeEdgeRoute (const WorkbenchSessionSnapshot::GraphEdgeSummary& edge,
                                        const std::vector<WorkbenchCanvasNodeSurface>& nodes)
{
    const auto* fromNode = findNodeSurface (nodes, endpointNodeId (edge.from));
    const auto* toNode = findNodeSurface (nodes, endpointNodeId (edge.to));

    WorkbenchCanvasEdgeRoute route;
    route.id = edge.id;
    route.from = edge.from;
    route.to = edge.to;
    route.dataType = edge.dataType;
    route.streamKind = edge.streamKind;

    if (fromNode != nullptr)
    {
        route.fromX = fromNode->x + fromNode->width;
        route.fromY = fromNode->y + fromNode->height / 2.0;
    }

    if (toNode != nullptr)
    {
        route.toX = toNode->x;
        route.toY = toNode->y + toNode->height / 2.0;
    }

    return route;
}

WorkbenchCanvasSurfaceRow makeRow (std::string id,
                                   std::string label,
                                   std::string value,
                                   std::string tone)
{
    WorkbenchCanvasSurfaceRow row;
    row.id = std::move (id);
    row.label = std::move (label);
    row.value = std::move (value);
    row.text = row.label + " " + row.value;
    row.tone = std::move (tone);
    return row;
}
}

WorkbenchCanvasSurface makeWorkbenchCanvasSurface (const WorkbenchSessionSnapshot& snapshot)
{
    WorkbenchCanvasSurface surface;
    surface.ok = snapshot.ok;

    for (const auto& node : snapshot.editorNodes)
        surface.nodeSurfaces.push_back (makeNodeSurface (node));

    for (const auto& edge : snapshot.editorEdges)
        surface.edgeRoutes.push_back (makeEdgeRoute (edge, surface.nodeSurfaces));

    const auto documentId = fallback (snapshot.documentId,
                                      snapshot.ok ? "untitled canvas" : "no workbench session");
    surface.headline = snapshot.ok
                           ? "canvas ready "
                                 + documentId
                                 + " nodes "
                                 + std::to_string (surface.nodeSurfaces.size())
                                 + " routes "
                                 + std::to_string (surface.edgeRoutes.size())
                           : "canvas blocked: " + fallback (snapshot.message, "no current workbench session");

    const auto emptyNode = WorkbenchCanvasNodeSurface {};
    const auto emptyRoute = WorkbenchCanvasEdgeRoute {};
    const auto& firstNode = surface.nodeSurfaces.empty() ? emptyNode : surface.nodeSurfaces.front();
    const auto& firstRoute = surface.edgeRoutes.empty() ? emptyRoute : surface.edgeRoutes.front();

    surface.rows = {
        makeRow ("canvas",
                 "canvas",
                 documentId,
                 snapshot.ok ? "ready" : "blocked"),
        makeRow ("nodes",
                 "nodes",
                 std::to_string (surface.nodeSurfaces.size()),
                 surface.nodeSurfaces.empty() ? "idle" : "ready"),
        makeRow ("routes",
                 "routes",
                 std::to_string (surface.edgeRoutes.size()),
                 surface.edgeRoutes.empty() ? "idle" : "ready"),
        makeRow ("bounds",
                 "bounds",
                 boundsValue (firstNode),
                 surface.nodeSurfaces.empty() ? "idle" : "ready"),
        makeRow ("route",
                 "route",
                 routeValue (firstRoute),
                 surface.edgeRoutes.empty() ? "idle" : "ready"),
        makeRow ("points",
                 "points",
                 pointsValue (firstRoute),
                 surface.edgeRoutes.empty() ? "idle" : "ready")
    };

    return surface;
}
}
