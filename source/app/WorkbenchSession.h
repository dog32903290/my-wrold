#pragma once

#include "GraphIOMapping.h"
#include "StorageContract.h"

#include <string>
#include <vector>

namespace myworld
{
struct WorkbenchSessionRequest
{
    std::string workManifestPath;
    std::string workSource = "unknown";
    std::string workSourceStatus = "unknown";
    std::string activeWorkManifestPath;
    std::string graphIOMappingSourcePath;
    PatchDocument document;
    std::vector<GraphIOMapping> graphIOMappings;
    bool dirty = false;
    std::string saveStatus = "unknown";
    std::string proofStatus = "not-run";
    std::string previewStatus = "not-ready";
};

struct WorkbenchSessionSnapshot
{
    bool ok = false;
    std::string status;
    std::string message;
    std::string workManifestPath;
    std::string workSource;
    std::string workSourceStatus;
    std::string activeWorkManifestPath;
    std::string graphIOMappingSourcePath;
    std::string documentId;
    std::string documentTitle;
    int documentVersion = 0;
    bool dirty = false;
    std::string saveStatus;
    std::string proofStatus;
    std::string previewStatus;
    int editorNodeCount = 0;
    int editorEdgeCount = 0;
    int runtimeNodeCount = 0;
    int runtimeEdgeCount = 0;
    std::string activeOutputNodeId;
    std::string timelineTransport;
    int graphIOMappingCount = 0;
    int validGraphIOMappingCount = 0;
    std::string graphIOMappingStatus;
    std::vector<std::string> diagnostics;
};

WorkbenchSessionSnapshot makeWorkbenchSessionSnapshot (const WorkbenchSessionRequest& request);
std::string makeWorkbenchSessionReportJson (const WorkbenchSessionSnapshot& snapshot);
}
