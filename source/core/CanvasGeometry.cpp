#include "CanvasGeometry.h"

namespace myworld
{
const CanvasGeometryContract& defaultCanvasGeometry()
{
    static const CanvasGeometryContract geometry;
    return geometry;
}

CanvasNodeBounds canvasNodeBounds (const GraphNode& node, const CanvasGeometryContract& geometry)
{
    return {
        node.position.x,
        node.position.y,
        geometry.nodeWidth,
        geometry.nodeHeight
    };
}

bool canvasPointInNodeBody (GraphPoint point, const GraphNode& node, const CanvasGeometryContract& geometry)
{
    const auto bounds = canvasNodeBounds (node, geometry);
    return point.x >= bounds.x
           && point.x <= bounds.x + bounds.width
           && point.y >= bounds.y
           && point.y <= bounds.y + bounds.height;
}

GraphPoint canvasPortCenterForIndex (const GraphNode& node,
                                      const std::string& direction,
                                      size_t index,
                                      const CanvasGeometryContract& geometry)
{
    const auto x = direction == "out" ? node.position.x + geometry.nodeWidth : node.position.x;
    return { x, node.position.y + geometry.firstPortOffsetY + static_cast<double> (index) * geometry.portSpacing };
}
}
