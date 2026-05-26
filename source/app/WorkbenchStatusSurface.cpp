#include "WorkbenchStatusSurface.h"

#include <string>

namespace myworld
{
namespace
{
std::string fallback (const std::string& value, const std::string& fallbackValue)
{
    return value.empty() ? fallbackValue : value;
}

std::string mappingValue (const WorkbenchAppStatusSnapshot& snapshot)
{
    return fallback (snapshot.graphIOMappingStatus, "unknown")
           + " "
           + std::to_string (snapshot.validGraphIOMappingCount)
           + "/"
           + std::to_string (snapshot.graphIOMappingCount);
}

WorkbenchStatusSurfaceRow makeRow (std::string id,
                                   std::string label,
                                   std::string value,
                                   std::string tone)
{
    WorkbenchStatusSurfaceRow row;
    row.id = std::move (id);
    row.label = std::move (label);
    row.value = std::move (value);
    row.text = row.label + " " + row.value;
    row.tone = std::move (tone);
    return row;
}
}

WorkbenchStatusSurface makeWorkbenchStatusSurface (const WorkbenchAppStatusSnapshot& snapshot)
{
    WorkbenchStatusSurface surface;
    surface.ok = snapshot.ok;
    surface.headline = fallback (snapshot.statusText,
                                 snapshot.ok ? "workbench ready" : "workbench blocked");
    surface.rows = {
        makeRow ("work",
                 "work",
                 fallback (snapshot.documentId,
                           snapshot.ok ? "untitled work" : "no workbench session"),
                 snapshot.ok ? "ready" : "blocked"),
        makeRow ("source",
                 "source",
                 fallback (snapshot.workSourceStatus, "unknown"),
                 snapshot.ok ? "ready" : "blocked"),
        makeRow ("save",
                 "save",
                 fallback (snapshot.saveStatus, "unknown"),
                 snapshot.dirty ? "dirty" : "ready"),
        makeRow ("mapping",
                 "mapping",
                 mappingValue (snapshot),
                 snapshot.graphIOMappingStatus == "valid" ? "ready" : "blocked"),
        makeRow ("proof",
                 "proof",
                 fallback (snapshot.proofStatus, "unknown"),
                 "idle"),
        makeRow ("preview",
                 "preview",
                 fallback (snapshot.previewStatus, "unknown"),
                 "idle")
    };
    return surface;
}
}
