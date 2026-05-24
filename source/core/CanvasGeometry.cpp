#include "CanvasGeometry.h"

#include "NodeSpec.h"

#include <algorithm>

namespace myworld
{
namespace
{
std::string labelForPort (const PortSpec& port)
{
    return port.label.empty() ? port.id : port.label;
}

double estimatedTextWidth (const std::string& text, const CanvasGeometryContract& geometry)
{
    return static_cast<double> (text.size()) * geometry.textCharWidth;
}

double maxPortLabelWidth (const std::vector<PortSpec>& ports, const CanvasGeometryContract& geometry)
{
    double width = 0.0;

    for (const auto& port : ports)
        width = std::max (width, estimatedTextWidth (labelForPort (port), geometry));

    return width;
}

double nodeWidthForSpec (const GraphNode& node, const NodeSpec* spec, const CanvasGeometryContract& geometry)
{
    auto required = geometry.nodeWidth;
    required = std::max (required, estimatedTextWidth (node.id, geometry) + geometry.titlePaddingX * 2.0);

    if (spec == nullptr)
        return std::clamp (required, geometry.nodeWidth, geometry.maxNodeWidth);

    const auto inputWidth = maxPortLabelWidth (spec->inputs, geometry);
    const auto outputWidth = maxPortLabelWidth (spec->outputs, geometry);
    const auto hasInputs = ! spec->inputs.empty();
    const auto hasOutputs = ! spec->outputs.empty();

    if (hasInputs && hasOutputs)
    {
        required = std::max (required,
                             geometry.portStripWidth * 2.0
                                 + geometry.portLabelPaddingX * 2.0
                                 + geometry.portColumnGutter
                                 + inputWidth
                                 + outputWidth);
    }
    else
    {
        required = std::max (required,
                             geometry.portStripWidth * 2.0
                                 + geometry.portLabelPaddingX * 2.0
                                 + std::max (inputWidth, outputWidth));
    }

    return std::clamp (required, geometry.nodeWidth, geometry.maxNodeWidth);
}

double nodeHeightForSpec (const NodeSpec* spec, const CanvasGeometryContract& geometry)
{
    if (spec == nullptr)
        return geometry.nodeHeight;

    const auto rowCount = std::max (spec->inputs.size(), spec->outputs.size());
    if (rowCount == 0)
        return geometry.nodeHeight;

    const auto lastCenterY = geometry.firstPortOffsetY + static_cast<double> (rowCount - 1) * geometry.portSpacing;
    const auto required = lastCenterY + geometry.portRowHalfHeight + geometry.bottomPadding;
    return std::max (geometry.nodeHeight, required);
}

CanvasNodeBounds labelBoundsForDirection (const CanvasNodeBounds& bounds,
                                          const CanvasGeometryContract& geometry,
                                          const std::string& direction,
                                          bool splitColumns,
                                          double rowTop,
                                          double rowHeight)
{
    if (splitColumns)
    {
        const auto width = std::max (0.0,
                                     (bounds.width
                                      - geometry.portStripWidth * 2.0
                                      - geometry.portLabelPaddingX * 2.0
                                      - geometry.portColumnGutter)
                                         / 2.0);
        const auto inputX = bounds.x + geometry.portStripWidth + geometry.portLabelPaddingX;
        const auto outputX = bounds.x + bounds.width - geometry.portStripWidth - geometry.portLabelPaddingX - width;
        return { direction == "out" ? outputX : inputX, rowTop, width, rowHeight };
    }

    const auto width = std::max (0.0,
                                 bounds.width
                                     - geometry.portStripWidth * 2.0
                                     - geometry.portLabelPaddingX * 2.0);

    if (direction == "out")
        return { bounds.x + bounds.width - geometry.portStripWidth - geometry.portLabelPaddingX - width,
                 rowTop,
                 width,
                 rowHeight };

    return { bounds.x + geometry.portStripWidth + geometry.portLabelPaddingX,
             rowTop,
             width,
             rowHeight };
}

CanvasPortRowGeometry makePortRowGeometry (const CanvasNodeBounds& bounds,
                                           const CanvasGeometryContract& geometry,
                                           const PortSpec& port,
                                           const std::string& direction,
                                           size_t index,
                                           bool splitColumns)
{
    const auto centerY = bounds.y + geometry.firstPortOffsetY + static_cast<double> (index) * geometry.portSpacing;
    const auto rowTop = centerY - geometry.portRowHalfHeight;
    const auto rowHeight = geometry.portRowHalfHeight * 2.0;
    const auto stripX = direction == "out" ? bounds.x + bounds.width - geometry.portStripWidth : bounds.x;

    return {
        port.id,
        labelForPort (port),
        { bounds.x, rowTop, bounds.width, rowHeight },
        { stripX, rowTop, geometry.portStripWidth, rowHeight },
        labelBoundsForDirection (bounds, geometry, direction, splitColumns, rowTop, rowHeight),
        { direction == "out" ? bounds.x + bounds.width : bounds.x, centerY }
    };
}

bool pointInBounds (GraphPoint point, CanvasNodeBounds bounds)
{
    return point.x >= bounds.x
           && point.x <= bounds.x + bounds.width
           && point.y >= bounds.y
           && point.y <= bounds.y + bounds.height;
}
}

const CanvasGeometryContract& defaultCanvasGeometry()
{
    static const CanvasGeometryContract geometry;
    return geometry;
}

CanvasNodeSurfaceGeometry canvasNodeSurfaceGeometry (const GraphNode& node,
                                                     const NodeSpec* spec,
                                                     const CanvasGeometryContract& geometry)
{
    CanvasNodeSurfaceGeometry surface;
    surface.bounds = {
        node.position.x,
        node.position.y,
        nodeWidthForSpec (node, spec, geometry),
        nodeHeightForSpec (spec, geometry)
    };
    surface.titleBounds = {
        surface.bounds.x + geometry.titlePaddingX,
        surface.bounds.y + geometry.titleOffsetY,
        std::max (0.0, surface.bounds.width - geometry.titlePaddingX * 2.0),
        geometry.titleHeight
    };

    if (spec == nullptr)
        return surface;

    const auto splitColumns = ! spec->inputs.empty() && ! spec->outputs.empty();

    surface.inputRows.reserve (spec->inputs.size());
    for (size_t index = 0; index < spec->inputs.size(); ++index)
        surface.inputRows.push_back (makePortRowGeometry (surface.bounds,
                                                          geometry,
                                                          spec->inputs[index],
                                                          "in",
                                                          index,
                                                          splitColumns));

    surface.outputRows.reserve (spec->outputs.size());
    for (size_t index = 0; index < spec->outputs.size(); ++index)
        surface.outputRows.push_back (makePortRowGeometry (surface.bounds,
                                                           geometry,
                                                           spec->outputs[index],
                                                           "out",
                                                           index,
                                                           splitColumns));

    return surface;
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

CanvasNodeBounds canvasNodeBounds (const GraphNode& node, const NodeSpec* spec, const CanvasGeometryContract& geometry)
{
    return canvasNodeSurfaceGeometry (node, spec, geometry).bounds;
}

bool canvasPointInNodeBody (GraphPoint point, const GraphNode& node, const CanvasGeometryContract& geometry)
{
    return pointInBounds (point, canvasNodeBounds (node, geometry));
}

bool canvasPointInNodeBody (GraphPoint point,
                            const GraphNode& node,
                            const NodeSpec* spec,
                            const CanvasGeometryContract& geometry)
{
    return pointInBounds (point, canvasNodeBounds (node, spec, geometry));
}

GraphPoint canvasPortCenterForIndex (const GraphNode& node,
                                      const std::string& direction,
                                      size_t index,
                                      const CanvasGeometryContract& geometry)
{
    const auto x = direction == "out" ? node.position.x + geometry.nodeWidth : node.position.x;
    return { x, node.position.y + geometry.firstPortOffsetY + static_cast<double> (index) * geometry.portSpacing };
}

GraphPoint canvasPortCenterForIndex (const GraphNode& node,
                                      const NodeSpec* spec,
                                      const std::string& direction,
                                      size_t index,
                                      const CanvasGeometryContract& geometry)
{
    const auto surface = canvasNodeSurfaceGeometry (node, spec, geometry);
    const auto& rows = direction == "out" ? surface.outputRows : surface.inputRows;

    if (index >= rows.size())
        return canvasPortCenterForIndex (node, direction, index, geometry);

    return rows[index].center;
}
}
