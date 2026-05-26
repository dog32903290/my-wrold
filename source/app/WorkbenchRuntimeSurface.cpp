#include "WorkbenchRuntimeSurface.h"

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

std::string mappingValue (const WorkbenchSessionSnapshot& snapshot)
{
    return fallback (snapshot.graphIOMappingStatus, "unknown")
           + " "
           + std::to_string (snapshot.validGraphIOMappingCount)
           + "/"
           + std::to_string (snapshot.graphIOMappingCount);
}

std::string readinessValue (const WorkbenchSessionSnapshot& snapshot)
{
    if (! snapshot.ok)
        return "blocked";

    if (snapshot.runtimeNodeCount <= 0)
        return "no-runtime";

    return "ready";
}

std::string headlineStatus (const WorkbenchSessionSnapshot& snapshot)
{
    if (! snapshot.ok)
        return "blocked";

    return snapshot.runtimeNodeCount > 0 ? "ready" : "idle";
}

std::string readinessTone (const std::string& readiness)
{
    if (readiness == "blocked")
        return "blocked";

    if (readiness == "ready")
        return "ready";

    return "idle";
}

WorkbenchRuntimeSurfaceRow makeRow (std::string id,
                                    std::string label,
                                    std::string value,
                                    std::string tone)
{
    WorkbenchRuntimeSurfaceRow row;
    row.id = std::move (id);
    row.label = std::move (label);
    row.value = std::move (value);
    row.text = row.label + " " + row.value;
    row.tone = std::move (tone);
    return row;
}
}

WorkbenchRuntimeSurface makeWorkbenchRuntimeSurface (const WorkbenchSessionSnapshot& snapshot)
{
    WorkbenchRuntimeSurface surface;
    surface.ok = snapshot.ok;

    const auto documentId = fallback (snapshot.documentId,
                                      snapshot.ok ? "untitled runtime" : "no workbench session");
    const auto readiness = readinessValue (snapshot);
    const auto status = headlineStatus (snapshot);

    surface.headline = snapshot.ok
                           ? "runtime "
                                 + status
                                 + " "
                                 + documentId
                                 + " nodes "
                                 + std::to_string (snapshot.runtimeNodeCount)
                                 + " edges "
                                 + std::to_string (snapshot.runtimeEdgeCount)
                           : "runtime blocked: " + fallback (snapshot.message, "no current workbench session");

    surface.rows = {
        makeRow ("runtime",
                 "runtime",
                 documentId,
                 snapshot.ok ? readinessTone (readiness) : "blocked"),
        makeRow ("graph",
                 "graph",
                 countValue (snapshot.runtimeNodeCount, snapshot.runtimeEdgeCount),
                 snapshot.runtimeNodeCount > 0 ? "ready" : "idle"),
        makeRow ("output",
                 "output",
                 fallback (snapshot.activeOutputNodeId, "none"),
                 snapshot.activeOutputNodeId.empty() ? "idle" : "ready"),
        makeRow ("mapping",
                 "mapping",
                 mappingValue (snapshot),
                 snapshot.graphIOMappingStatus == "valid" ? "ready" : "idle"),
        makeRow ("readiness",
                 "readiness",
                 readiness,
                 readinessTone (readiness)),
        makeRow ("cook",
                 "cook",
                 "parked",
                 "idle")
    };

    return surface;
}
}
