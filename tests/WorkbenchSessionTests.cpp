#include "WorkbenchSession.h"

#include "GraphIOMappingStorage.h"
#include "StorageContract.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (condition)
        return;

    std::cerr << "FAIL: " << message << '\n';
    std::exit (1);
}

void expectEqual (const std::string& actual, const std::string& expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + expected + " got " + actual);
}

void expectEqual (int actual, int expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + std::to_string (expected)
                                + " got " + std::to_string (actual));
}

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}
}

int main()
{
    const std::string workManifestPath = "fixtures/storage/c2-compound-work/myworld.work.json";
    auto loadedPatch = myworld::loadMainPatchDocumentForWork (workManifestPath);
    expect (loadedPatch.ok, loadedPatch.error);
    loadedPatch.document.outputView.followedNodeId = "out1";

    const auto loadedMappings = myworld::loadGraphIOMappingsFromFile (
        "fixtures/graphs/g1_loudness_to_shader_uniform.graph.json");
    expect (loadedMappings.ok, loadedMappings.error);

    myworld::WorkbenchSessionRequest request;
    request.workManifestPath = workManifestPath;
    request.document = loadedPatch.document;
    request.graphIOMappings = loadedMappings.mappings;
    request.dirty = true;
    request.saveStatus = "dirty";
    request.proofStatus = "g1-report-dumped";
    request.previewStatus = "preview-ready";
    request.workDiagnostics = { "workSourceStatus=fixture-fallback-no-active-request" };

    const auto snapshot = myworld::makeWorkbenchSessionSnapshot (request);
    expect (snapshot.ok, snapshot.message);
    expectEqual (snapshot.status, "ready", "snapshot status");
    expectEqual (snapshot.workManifestPath, workManifestPath, "work manifest path");
    expectEqual (snapshot.documentId, "patch.c2-main", "document id");
    expectEqual (snapshot.documentTitle, "C2 Main", "document title");
    expectEqual (snapshot.documentVersion, 1, "document version");
    expect (snapshot.dirty, "dirty state");
    expectEqual (snapshot.saveStatus, "dirty", "save status");
    expectEqual (snapshot.proofStatus, "g1-report-dumped", "proof status");
    expectEqual (snapshot.previewStatus, "preview-ready", "preview status");
    expectEqual (snapshot.editorNodeCount, 5, "editor node count");
    expectEqual (snapshot.editorEdgeCount, 3, "editor edge count");
    expectEqual (snapshot.runtimeNodeCount, 5, "runtime node count");
    expectEqual (snapshot.runtimeEdgeCount, 3, "runtime edge count");
    expectEqual (static_cast<int> (snapshot.editorNodes.size()), 5, "editor node summary count");
    expectEqual (static_cast<int> (snapshot.editorEdges.size()), 3, "editor edge summary count");
    expectEqual (static_cast<int> (snapshot.runtimeNodes.size()), 5, "runtime node summary count");
    expectEqual (static_cast<int> (snapshot.runtimeEdges.size()), 3, "runtime edge summary count");
    expectEqual (snapshot.editorNodes.front().id, "shader1", "first editor node id");
    expectEqual (snapshot.editorNodes.front().type, "shader.fragment", "first editor node type");
    expectEqual (snapshot.editorNodes.front().systemUniformCount, 4, "first editor node uniform count");
    expect (! snapshot.editorNodes.front().collapsed, "first editor node collapsed");
    expectEqual (snapshot.editorEdges.front().from, "shader1.output", "first editor edge from");
    expectEqual (snapshot.editorEdges.front().to, "out1.input", "first editor edge to");
    expectEqual (snapshot.editorEdges.front().dataType, "texture.rgba", "first editor edge data type");
    expectEqual (snapshot.runtimeNodes.front().id, "shader1", "first runtime node id");
    expectEqual (snapshot.runtimeNodes.front().type, "shader.fragment", "first runtime node type");
    expectEqual (snapshot.runtimeEdges.front().from, "shader1.output", "first runtime edge from");
    expectEqual (snapshot.runtimeEdges.front().to, "out1.input", "first runtime edge to");
    expectEqual (snapshot.activeOutputNodeId, "out1", "active output");
    expectEqual (snapshot.timelineTransport, "stopped", "timeline transport");
    expectEqual (snapshot.graphIOMappingCount, 1, "mapping count");
    expectEqual (snapshot.validGraphIOMappingCount, 1, "valid mapping count");
    expectEqual (snapshot.graphIOMappingStatus, "valid", "mapping status");
    expect (snapshot.diagnostics.empty(), "no diagnostics");
    expectEqual (static_cast<int> (snapshot.workDiagnostics.size()), 1, "work diagnostic count");

    const auto json = myworld::makeWorkbenchSessionReportJson (snapshot);
    expectContains (json, "\"kind\": \"workbenchSessionReport\"", "json kind");
    expectContains (json, "\"ok\": true", "json ok");
    expectContains (json, "\"status\": \"ready\"", "json status");
    expectContains (json, "\"documentId\": \"patch.c2-main\"", "json document id");
    expectContains (json, "\"dirty\": true", "json dirty");
    expectContains (json, "\"editorNodeCount\": 5", "json editor node count");
    expectContains (json, "\"graphIOMappingStatus\": \"valid\"", "json mapping status");
    expectContains (json, "\"activeOutputNodeId\": \"out1\"", "json output");
    expectContains (json, "\"previewStatus\": \"preview-ready\"", "json preview");
    expectContains (json, "\"proofStatus\": \"g1-report-dumped\"", "json proof");
    expectContains (json, "\"saveStatus\": \"dirty\"", "json save");
    expectContains (json,
                    "\"workDiagnostics\": [\"workSourceStatus=fixture-fallback-no-active-request\"]",
                    "json work diagnostics");
    expectContains (json, "\"diagnostics\": []", "json diagnostics");

    auto invalidRequest = request;
    invalidRequest.graphIOMappings.front().target.uniformName.clear();
    const auto invalid = myworld::makeWorkbenchSessionSnapshot (invalidRequest);
    expect (! invalid.ok, "invalid mapping blocks ok flag");
    expectEqual (invalid.status, "blocked", "invalid mapping status");
    expectEqual (invalid.graphIOMappingStatus, "invalid", "invalid graph io status");
    expectEqual (invalid.validGraphIOMappingCount, 0, "invalid valid count");
    expectEqual (static_cast<int> (invalid.diagnostics.size()), 1, "invalid diagnostic count");
    expectContains (invalid.diagnostics.front(), "shader uniform is required", "invalid diagnostic");

    std::cout << "workbench session ok\n";
    return 0;
}
