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
    return makePatchDocument (id, title, graph, {}, {});
}

PatchDocument makePatchDocument (const std::string& id,
                                 const std::string& title,
                                 const GraphContract& graph,
                                 const OutputViewState& outputView)
{
    return makePatchDocument (id, title, graph, outputView, {});
}

PatchDocument makePatchDocument (const std::string& id,
                                 const std::string& title,
                                 const GraphContract& graph,
                                 const OutputViewState& outputView,
                                 const TimelineState& timeline)
{
    return { id, title, graph.version, graph, outputView, sanitizedTimelineState (timeline) };
}

namespace
{
void appendOutputViewJson (std::ostringstream& out, const OutputViewState& outputView)
{
    out << "{ \"pinned\": " << (outputView.pinned ? "true" : "false")
        << ", \"followedNodeId\": " << jsonQuoted (outputView.followedNodeId)
        << ", \"pinnedNodeId\": " << jsonQuoted (outputView.pinnedNodeId) << " }";
}

void appendTimelineJson (std::ostringstream& out, const TimelineState& timeline)
{
    const auto sanitized = sanitizedTimelineState (timeline);
    out << "{ \"bpm\": " << sanitized.bpm
        << ", \"framesPerSecond\": " << sanitized.framesPerSecond
        << ", \"beatsPerBar\": " << sanitized.beatsPerBar
        << ", \"positionBars\": " << sanitized.positionBars
        << ", \"loopStartBars\": " << sanitized.loopStartBars
        << ", \"loopEndBars\": " << sanitized.loopEndBars
        << ", \"looping\": " << (sanitized.looping ? "true" : "false")
        << ", \"transportState\": " << jsonQuoted (transportStateToString (sanitized.transportState))
        << ", \"playbackRate\": " << sanitized.playbackRate
        << ", \"playbackDirection\": " << sanitized.playbackDirection << " }";
}

OutputViewState parseOutputView (const JsonValue& root)
{
    OutputViewState outputView;
    const auto* jsonOutputView = member (root, "outputView");
    if (jsonOutputView == nullptr || jsonOutputView->kind != JsonValue::Kind::object)
        return outputView;

    outputView.pinned = boolMember (*jsonOutputView, "pinned", false);
    outputView.followedNodeId = stringMember (*jsonOutputView, "followedNodeId");
    outputView.pinnedNodeId = stringMember (*jsonOutputView, "pinnedNodeId");

    if (outputView.pinned && outputView.pinnedNodeId.empty())
        outputView.pinned = false;

    return outputView;
}

TimelineState parseTimeline (const JsonValue& root)
{
    TimelineState timeline;
    const auto* jsonTimeline = member (root, "timeline");
    if (jsonTimeline == nullptr || jsonTimeline->kind != JsonValue::Kind::object)
        return timeline;

    timeline.bpm = numberMember (*jsonTimeline, "bpm", timeline.bpm);
    timeline.framesPerSecond = numberMember (*jsonTimeline, "framesPerSecond", timeline.framesPerSecond);
    timeline.beatsPerBar = numberMember (*jsonTimeline, "beatsPerBar", timeline.beatsPerBar);
    timeline.positionBars = numberMember (*jsonTimeline, "positionBars", timeline.positionBars);
    timeline.loopStartBars = numberMember (*jsonTimeline, "loopStartBars", timeline.loopStartBars);
    timeline.loopEndBars = numberMember (*jsonTimeline, "loopEndBars", timeline.loopEndBars);
    timeline.looping = boolMember (*jsonTimeline, "looping", timeline.looping);
    timeline.transportState = transportStateFromString (stringMember (*jsonTimeline, "transportState"));
    timeline.playbackRate = numberMember (*jsonTimeline, "playbackRate", timeline.playbackRate);
    timeline.playbackDirection = intMember (*jsonTimeline, "playbackDirection", timeline.playbackDirection);

    return sanitizedTimelineState (timeline);
}
}
}

namespace myworld
{
using namespace storage_contract_internal;

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
    out << ",\n";
    out << "  \"outputView\": ";
    appendOutputViewJson (out, document.outputView);
    out << ",\n";
    out << "  \"timeline\": ";
    appendTimelineJson (out, document.timeline);
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
    document.outputView = parseOutputView (root);
    document.timeline = parseTimeline (root);

    if (document.id.empty() || document.title.empty())
        return { false, {}, "patch document is missing required identity fields" };

    const auto* editorGraph = member (root, "editorGraph");
    if (editorGraph == nullptr || ! parseEditorGraph (document.graph.editorGraph, *editorGraph))
        return { false, {}, "patch document has invalid editorGraph" };

    const auto* runtimeGraph = member (root, "runtimeGraph");
    if (runtimeGraph == nullptr || ! parseRuntimeGraph (document.graph.runtimeGraph, *runtimeGraph))
        return { false, {}, "patch document has invalid runtimeGraph" };

    if (document.outputView.pinned && ! outputViewTargetExists (document.graph, document.outputView.pinnedNodeId))
    {
        document.outputView.pinned = false;
        document.outputView.pinnedNodeId.clear();
    }

    if (! document.outputView.followedNodeId.empty()
        && ! outputViewTargetExists (document.graph, document.outputView.followedNodeId))
    {
        document.outputView.followedNodeId.clear();
    }

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
