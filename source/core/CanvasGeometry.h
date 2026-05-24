#pragma once

#include "GraphContract.h"

#include <cstddef>
#include <string>
#include <vector>

namespace myworld
{
struct NodeSpec;

struct CanvasGeometryContract
{
    double nodeWidth = 140.0;
    double nodeHeight = 60.0;
    double firstPortOffsetY = 30.0;
    double portSpacing = 18.0;
    double maxNodeWidth = 260.0;
    double portRowHalfHeight = 7.0;
    double portStripWidth = 5.0;
    double portLabelPaddingX = 10.0;
    double portColumnGutter = 12.0;
    double titlePaddingX = 12.0;
    double titleOffsetY = 10.0;
    double titleHeight = 16.0;
    double bottomPadding = 8.0;
    double textCharWidth = 7.0;
};

struct CanvasNodeBounds
{
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
};

struct CanvasPortRowGeometry
{
    std::string portId;
    std::string label;
    CanvasNodeBounds rowBounds;
    CanvasNodeBounds stripBounds;
    CanvasNodeBounds labelBounds;
    GraphPoint center;
};

struct CanvasNodeSurfaceGeometry
{
    CanvasNodeBounds bounds;
    CanvasNodeBounds titleBounds;
    std::vector<CanvasPortRowGeometry> inputRows;
    std::vector<CanvasPortRowGeometry> outputRows;
};

const CanvasGeometryContract& defaultCanvasGeometry();
CanvasNodeSurfaceGeometry canvasNodeSurfaceGeometry (const GraphNode& node,
                                                     const NodeSpec* spec,
                                                     const CanvasGeometryContract& geometry = defaultCanvasGeometry());
CanvasNodeBounds canvasNodeBounds (const GraphNode& node,
                                   const CanvasGeometryContract& geometry = defaultCanvasGeometry());
CanvasNodeBounds canvasNodeBounds (const GraphNode& node,
                                   const NodeSpec* spec,
                                   const CanvasGeometryContract& geometry = defaultCanvasGeometry());
bool canvasPointInNodeBody (GraphPoint point,
                            const GraphNode& node,
                            const CanvasGeometryContract& geometry = defaultCanvasGeometry());
bool canvasPointInNodeBody (GraphPoint point,
                            const GraphNode& node,
                            const NodeSpec* spec,
                            const CanvasGeometryContract& geometry = defaultCanvasGeometry());
GraphPoint canvasPortCenterForIndex (const GraphNode& node,
                                      const std::string& direction,
                                      size_t index,
                                      const CanvasGeometryContract& geometry = defaultCanvasGeometry());
GraphPoint canvasPortCenterForIndex (const GraphNode& node,
                                      const NodeSpec* spec,
                                      const std::string& direction,
                                      size_t index,
                                      const CanvasGeometryContract& geometry = defaultCanvasGeometry());
}
