#include "StorageContract.h"

#include "JsonWriter.h"
#include "StorageContractJson.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace myworld
{
using namespace storage_contract_internal;

PatchDocument makePatchDocument (const std::string& id, const std::string& title, const GraphContract& graph)
{
    return { id, title, graph.version, graph };
}

std::string toJson (const PatchDocumentManifest& manifest)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"patchDocument\",\n";
    out << "  \"id\": " << jsonQuoted (manifest.id) << ",\n";
    out << "  \"title\": " << jsonQuoted (manifest.title) << ",\n";
    out << "  \"editorGraph\": { \"kind\": " << jsonQuoted (manifest.editorGraphKind) << " },\n";
    out << "  \"runtimeGraph\": { \"kind\": " << jsonQuoted (manifest.runtimeGraphKind) << " },\n";
    out << "  \"portBindings\": { \"kind\": " << jsonQuoted (manifest.portBindingsKind) << " }\n";
    out << "}\n";
    return out.str();
}

std::string toJson (const PatchDocument& document)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"patchDocument\",\n";
    out << "  \"id\": " << jsonQuoted (document.id) << ",\n";
    out << "  \"title\": " << jsonQuoted (document.title) << ",\n";
    out << "  \"version\": " << document.version << ",\n";
    out << "  \"editorGraph\": ";
    appendGraphSectionJson (out, document.graph.editorGraph.nodes, document.graph.editorGraph.edges);
    out << ",\n";
    out << "  \"runtimeGraph\": ";
    appendGraphSectionJson (out, document.graph.runtimeGraph.nodes, document.graph.runtimeGraph.edges);
    out << "\n";
    out << "}\n";
    return out.str();
}

PatchDocumentLoadResult parsePatchDocument (const std::string& text)
{
    JsonParser parser (text);
    const auto root = parser.parse();

    if (! parser.ok())
        return { false, {}, parser.error() };

    if (root.kind != JsonValue::Kind::object)
        return { false, {}, "patch document root must be an object" };

    if (stringMember (root, "kind") != "patchDocument")
        return { false, {}, "patch document kind must be patchDocument" };

    PatchDocument document;
    document.id = stringMember (root, "id");
    document.title = stringMember (root, "title");
    document.version = intMember (root, "version", 1);
    document.graph.version = document.version;

    if (document.id.empty() || document.title.empty())
        return { false, {}, "patch document is missing required identity fields" };

    const auto* editorGraph = member (root, "editorGraph");
    if (editorGraph == nullptr || ! parseEditorGraph (document.graph.editorGraph, *editorGraph))
        return { false, {}, "patch document has invalid editorGraph" };

    const auto* runtimeGraph = member (root, "runtimeGraph");
    if (runtimeGraph == nullptr || ! parseRuntimeGraph (document.graph.runtimeGraph, *runtimeGraph))
        return { false, {}, "patch document has invalid runtimeGraph" };

    return { true, document, {} };
}

PatchDocumentLoadResult loadPatchDocument (const std::string& path)
{
    std::string error;
    const auto text = readTextFile (path, error);
    if (! error.empty())
        return { false, {}, "could not open patch document: " + path };

    return parsePatchDocument (text);
}

PatchDocumentLoadResult loadMainPatchDocumentForWork (const std::string& workManifestPath)
{
    const auto work = loadWorkProjectManifest (workManifestPath);
    if (! work.ok)
        return { false, {}, work.error };

    const auto patchPath = resolvePathNearFile (workManifestPath, work.manifest.mainPatchPath);
    return loadPatchDocument (patchPath.string());
}

PatchDocumentSaveResult savePatchDocument (const std::string& path, const PatchDocument& document)
{
    if (document.id.empty() || document.title.empty())
        return { false, "validation-failed", path, "patch document is missing required identity fields" };

    std::error_code error;
    const std::filesystem::path targetPath (path);
    const auto parent = targetPath.parent_path();

    if (! parent.empty())
        std::filesystem::create_directories (parent, error);

    if (error)
        return { false, "write-failed", path, "could not create patch document directory: " + error.message() };

    const auto tempPath = targetPath.string() + ".tmp";
    {
        std::ofstream output (tempPath, std::ios::trunc);
        if (! output)
            return { false, "write-failed", path, "could not open patch document for writing: " + tempPath };

        output << toJson (document);

        if (! output)
            return { false, "write-failed", path, "could not write patch document: " + tempPath };
    }

    std::filesystem::rename (tempPath, targetPath, error);
    if (error)
    {
        std::filesystem::remove (tempPath);
        return { false, "write-failed", path, "could not replace patch document: " + error.message() };
    }

    return { true, "save-ok commit-pending", path, {} };
}
}
