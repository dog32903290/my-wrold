#include "WorkbenchGraphSurface.h"

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

std::string firstNodeValue (const WorkbenchSessionSnapshot& snapshot)
{
    if (snapshot.editorNodes.empty())
        return "none";

    const auto& node = snapshot.editorNodes.front();
    return fallback (node.id, "unknown") + " " + fallback (node.type, "unknown");
}

std::string firstEdgeValue (const WorkbenchSessionSnapshot& snapshot)
{
    if (snapshot.editorEdges.empty())
        return "none";

    const auto& edge = snapshot.editorEdges.front();
    return fallback (edge.from, "unknown") + " -> " + fallback (edge.to, "unknown");
}

WorkbenchGraphSurfaceRow makeRow (std::string id,
                                  std::string label,
                                  std::string value,
                                  std::string tone)
{
    WorkbenchGraphSurfaceRow row;
    row.id = std::move (id);
    row.label = std::move (label);
    row.value = std::move (value);
    row.text = row.label + " " + row.value;
    row.tone = std::move (tone);
    return row;
}
}

WorkbenchGraphSurface makeWorkbenchGraphSurface (const WorkbenchSessionSnapshot& snapshot)
{
    WorkbenchGraphSurface surface;
    surface.ok = snapshot.ok;

    const auto documentId = fallback (snapshot.documentId,
                                      snapshot.ok ? "untitled graph" : "no workbench session");
    surface.headline = snapshot.ok
                           ? "graph ready "
                                 + documentId
                                 + " editor "
                                 + std::to_string (snapshot.editorNodeCount)
                                 + "/"
                                 + std::to_string (snapshot.editorEdgeCount)
                                 + " runtime "
                                 + std::to_string (snapshot.runtimeNodeCount)
                                 + "/"
                                 + std::to_string (snapshot.runtimeEdgeCount)
                           : "graph blocked: " + fallback (snapshot.message, "no current workbench session");

    surface.rows = {
        makeRow ("graph",
                 "graph",
                 documentId,
                 snapshot.ok ? "ready" : "blocked"),
        makeRow ("editor",
                 "editor",
                 countValue (snapshot.editorNodeCount, snapshot.editorEdgeCount),
                 snapshot.ok ? "ready" : "blocked"),
        makeRow ("runtime",
                 "runtime",
                 countValue (snapshot.runtimeNodeCount, snapshot.runtimeEdgeCount),
                 snapshot.ok ? "ready" : "blocked"),
        makeRow ("output",
                 "output",
                 fallback (snapshot.activeOutputNodeId, "none"),
                 snapshot.activeOutputNodeId.empty() ? "idle" : "ready"),
        makeRow ("node",
                 "node",
                 firstNodeValue (snapshot),
                 snapshot.editorNodes.empty() ? "idle" : "ready"),
        makeRow ("edge",
                 "edge",
                 firstEdgeValue (snapshot),
                 snapshot.editorEdges.empty() ? "idle" : "ready")
    };

    return surface;
}
}
