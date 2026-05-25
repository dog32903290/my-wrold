#include "WorkbenchSession.h"

#include "JsonWriter.h"
#include "OutputViewState.h"
#include "TimelineState.h"

#include <sstream>

namespace myworld
{
WorkbenchSessionSnapshot makeWorkbenchSessionSnapshot (const WorkbenchSessionRequest& request)
{
    WorkbenchSessionSnapshot snapshot;
    snapshot.workManifestPath = request.workManifestPath;
    snapshot.workSource = request.workSource;
    snapshot.graphIOMappingSourcePath = request.graphIOMappingSourcePath;
    snapshot.documentId = request.document.id;
    snapshot.documentTitle = request.document.title;
    snapshot.documentVersion = request.document.version;
    snapshot.dirty = request.dirty;
    snapshot.saveStatus = request.saveStatus;
    snapshot.proofStatus = request.proofStatus;
    snapshot.previewStatus = request.previewStatus;
    snapshot.editorNodeCount = static_cast<int> (request.document.graph.editorGraph.nodes.size());
    snapshot.editorEdgeCount = static_cast<int> (request.document.graph.editorGraph.edges.size());
    snapshot.runtimeNodeCount = static_cast<int> (request.document.graph.runtimeGraph.nodes.size());
    snapshot.runtimeEdgeCount = static_cast<int> (request.document.graph.runtimeGraph.edges.size());
    snapshot.activeOutputNodeId = activeOutputNodeId (request.document.outputView);
    snapshot.timelineTransport = transportStateToString (request.document.timeline.transportState);
    snapshot.graphIOMappingCount = static_cast<int> (request.graphIOMappings.size());

    for (const auto& mapping : request.graphIOMappings)
    {
        const auto validation = validateGraphIOMapping (mapping);
        if (validation.ok)
        {
            ++snapshot.validGraphIOMappingCount;
        }
        else
        {
            snapshot.diagnostics.insert (snapshot.diagnostics.end(),
                                         validation.diagnostics.begin(),
                                         validation.diagnostics.end());
        }
    }

    snapshot.graphIOMappingStatus = snapshot.validGraphIOMappingCount == snapshot.graphIOMappingCount
                                    ? "valid"
                                    : "invalid";

    if (snapshot.documentId.empty())
        snapshot.diagnostics.push_back ("document id is required");

    snapshot.ok = snapshot.diagnostics.empty();
    snapshot.status = snapshot.ok ? "ready" : "blocked";
    snapshot.message = snapshot.ok ? "workbench_session_ready" : snapshot.diagnostics.front();
    return snapshot;
}

std::string makeWorkbenchSessionReportJson (const WorkbenchSessionSnapshot& snapshot)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"workbenchSessionReport\",\n";
    out << "  \"ok\": " << (snapshot.ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (snapshot.status) << ",\n";
    out << "  \"message\": " << jsonQuoted (snapshot.message) << ",\n";
    out << "  \"workManifestPath\": " << jsonQuoted (snapshot.workManifestPath) << ",\n";
    out << "  \"workSource\": " << jsonQuoted (snapshot.workSource) << ",\n";
    out << "  \"graphIOMappingSourcePath\": " << jsonQuoted (snapshot.graphIOMappingSourcePath) << ",\n";
    out << "  \"documentId\": " << jsonQuoted (snapshot.documentId) << ",\n";
    out << "  \"documentTitle\": " << jsonQuoted (snapshot.documentTitle) << ",\n";
    out << "  \"documentVersion\": " << snapshot.documentVersion << ",\n";
    out << "  \"dirty\": " << (snapshot.dirty ? "true" : "false") << ",\n";
    out << "  \"saveStatus\": " << jsonQuoted (snapshot.saveStatus) << ",\n";
    out << "  \"proofStatus\": " << jsonQuoted (snapshot.proofStatus) << ",\n";
    out << "  \"previewStatus\": " << jsonQuoted (snapshot.previewStatus) << ",\n";
    out << "  \"editorNodeCount\": " << snapshot.editorNodeCount << ",\n";
    out << "  \"editorEdgeCount\": " << snapshot.editorEdgeCount << ",\n";
    out << "  \"runtimeNodeCount\": " << snapshot.runtimeNodeCount << ",\n";
    out << "  \"runtimeEdgeCount\": " << snapshot.runtimeEdgeCount << ",\n";
    out << "  \"activeOutputNodeId\": " << jsonQuoted (snapshot.activeOutputNodeId) << ",\n";
    out << "  \"timelineTransport\": " << jsonQuoted (snapshot.timelineTransport) << ",\n";
    out << "  \"graphIOMappingCount\": " << snapshot.graphIOMappingCount << ",\n";
    out << "  \"validGraphIOMappingCount\": " << snapshot.validGraphIOMappingCount << ",\n";
    out << "  \"graphIOMappingStatus\": " << jsonQuoted (snapshot.graphIOMappingStatus) << ",\n";
    out << "  \"diagnostics\": ";
    appendJsonStringArray (out, snapshot.diagnostics);
    out << "\n";
    out << "}\n";
    return out.str();
}
}
