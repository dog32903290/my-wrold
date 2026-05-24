#pragma once

#include "GraphContract.h"

#include <cstddef>
#include <string>

namespace myworld
{
struct CanvasGeometryContract
{
    double nodeWidth = 140.0;
    double nodeHeight = 60.0;
    double firstPortOffsetY = 30.0;
    double portSpacing = 18.0;
};

struct CanvasNodeBounds
{
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
};

const CanvasGeometryContract& defaultCanvasGeometry();
CanvasNodeBounds canvasNodeBounds (const GraphNode& node,
                                   const CanvasGeometryContract& geometry = defaultCanvasGeometry());
bool canvasPointInNodeBody (GraphPoint point,
                            const GraphNode& node,
                            const CanvasGeometryContract& geometry = defaultCanvasGeometry());
GraphPoint canvasPortCenterForIndex (const GraphNode& node,
                                      const std::string& direction,
                                      size_t index,
                                      const CanvasGeometryContract& geometry = defaultCanvasGeometry());
}
