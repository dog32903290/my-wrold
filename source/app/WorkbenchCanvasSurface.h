#pragma once

#include "WorkbenchSession.h"

#include <string>
#include <vector>

namespace myworld
{
struct WorkbenchCanvasSurfaceRow
{
    std::string id;
    std::string label;
    std::string value;
    std::string text;
    std::string tone;
};

struct WorkbenchCanvasNodeSurface
{
    std::string id;
    std::string type;
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
};

struct WorkbenchCanvasEdgeRoute
{
    std::string id;
    std::string from;
    std::string to;
    std::string dataType;
    std::string streamKind;
    double fromX = 0.0;
    double fromY = 0.0;
    double toX = 0.0;
    double toY = 0.0;
};

struct WorkbenchCanvasSurface
{
    bool ok = false;
    std::string headline;
    std::vector<WorkbenchCanvasSurfaceRow> rows;
    std::vector<WorkbenchCanvasNodeSurface> nodeSurfaces;
    std::vector<WorkbenchCanvasEdgeRoute> edgeRoutes;
};

WorkbenchCanvasSurface makeWorkbenchCanvasSurface (const WorkbenchSessionSnapshot& snapshot);
}
