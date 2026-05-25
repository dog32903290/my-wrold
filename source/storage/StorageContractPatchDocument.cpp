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
    return makePatchDocument (id, title, graph, {}, {}, {});
}

PatchDocument makePatchDocument (const std::string& id,
                                 const std::string& title,
                                 const GraphContract& graph,
                                 const OutputViewState& outputView)
{
    return makePatchDocument (id, title, graph, outputView, {}, {});
}

PatchDocument makePatchDocument (const std::string& id,
                                 const std::string& title,
                                 const GraphContract& graph,
                                 const OutputViewState& outputView,
                                 const TimelineState& timeline)
{
    return makePatchDocument (id, title, graph, outputView, timeline, {});
}

PatchDocument makePatchDocument (const std::string& id,
                                 const std::string& title,
                                 const GraphContract& graph,
                                 const OutputViewState& outputView,
                                 const TimelineState& timeline,
                                 const VariationLibrary& variations)
{
    return { id, title, graph.version, graph, outputView, sanitizedTimelineState (timeline), variations };
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

void appendVariationValuesJson (std::ostringstream& out, const std::vector<VariationValue>& values)
{
    out << "[";
    for (size_t index = 0; index < values.size(); ++index)
    {
        const auto& value = values[index];
        if (index != 0)
            out << ", ";

        out << "{ \"nodeId\": " << jsonQuoted (value.nodeId)
            << ", \"paramId\": " << jsonQuoted (value.paramId)
            << ", \"value\": " << jsonQuoted (value.value) << " }";
    }
    out << "]";
}

void appendVariationSkippedJson (std::ostringstream& out, const std::vector<VariationSkippedValue>& values)
{
    out << "[";
    for (size_t index = 0; index < values.size(); ++index)
    {
        const auto& value = values[index];
        if (index != 0)
            out << ", ";

        out << "{ \"nodeId\": " << jsonQuoted (value.nodeId)
            << ", \"paramId\": " << jsonQuoted (value.paramId)
            << ", \"reason\": " << jsonQuoted (variationSkipReasonToString (value.reason)) << " }";
    }
    out << "]";
}

void appendVariationRecordsJson (std::ostringstream& out, const std::vector<VariationRecord>& records)
{
    out << "[";
    for (size_t index = 0; index < records.size(); ++index)
    {
        const auto& record = records[index];
        if (index != 0)
            out << ", ";

        out << "{ \"id\": " << jsonQuoted (record.id)
            << ", \"title\": " << jsonQuoted (record.title)
            << ", \"kind\": " << jsonQuoted (variationKindToString (record.kind))
            << ", \"enabledNodeIds\": ";
        appendJsonStringArray (out, record.enabledNodeIds);
        out << ", \"values\": ";
        appendVariationValuesJson (out, record.values);
        out << ", \"skippedValues\": ";
        appendVariationSkippedJson (out, record.skippedValues);
        out << " }";
    }
    out << "]";
}

void appendVariationLibraryJson (std::ostringstream& out, const VariationLibrary& variations)
{
    out << "{ \"presets\": ";
    appendVariationRecordsJson (out, variations.presets);
    out << ", \"snapshots\": ";
    appendVariationRecordsJson (out, variations.snapshots);
    out << " }";
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

std::vector<VariationValue> parseVariationValues (const JsonValue& record)
{
    std::vector<VariationValue> values;
    const auto* jsonValues = member (record, "values");
    if (jsonValues == nullptr || jsonValues->kind != JsonValue::Kind::array)
        return values;

    for (const auto& jsonValue : jsonValues->arrayValue)
    {
        if (jsonValue.kind != JsonValue::Kind::object)
            continue;

        values.push_back ({ stringMember (jsonValue, "nodeId"),
                            stringMember (jsonValue, "paramId"),
                            stringMember (jsonValue, "value") });
    }

    return values;
}

std::vector<VariationSkippedValue> parseVariationSkippedValues (const JsonValue& record)
{
    std::vector<VariationSkippedValue> values;
    const auto* jsonValues = member (record, "skippedValues");
    if (jsonValues == nullptr || jsonValues->kind != JsonValue::Kind::array)
        return values;

    for (const auto& jsonValue : jsonValues->arrayValue)
    {
        if (jsonValue.kind != JsonValue::Kind::object)
            continue;

        values.push_back ({ stringMember (jsonValue, "nodeId"),
                            stringMember (jsonValue, "paramId"),
                            variationSkipReasonFromString (stringMember (jsonValue, "reason")) });
    }

    return values;
}

std::vector<VariationRecord> parseVariationRecords (const JsonValue& variations,
                                                    const std::string& memberName,
                                                    VariationKind fallbackKind)
{
    std::vector<VariationRecord> records;
    const auto* jsonRecords = member (variations, memberName);
    if (jsonRecords == nullptr || jsonRecords->kind != JsonValue::Kind::array)
        return records;

    for (const auto& jsonRecord : jsonRecords->arrayValue)
    {
        if (jsonRecord.kind != JsonValue::Kind::object)
            continue;

        VariationRecord record;
        record.id = stringMember (jsonRecord, "id");
        record.title = stringMember (jsonRecord, "title");
        record.kind = variationKindFromString (stringMember (jsonRecord, "kind"));
        if (record.kind != fallbackKind)
            record.kind = fallbackKind;
        record.enabledNodeIds = stringArrayMember (jsonRecord, "enabledNodeIds");
        record.values = parseVariationValues (jsonRecord);
        record.skippedValues = parseVariationSkippedValues (jsonRecord);

        if (! record.id.empty())
            records.push_back (record);
    }

    return records;
}

VariationLibrary parseVariationLibrary (const JsonValue& root)
{
    VariationLibrary variations;
    const auto* jsonVariations = member (root, "variations");
    if (jsonVariations == nullptr || jsonVariations->kind != JsonValue::Kind::object)
        return variations;

    variations.presets = parseVariationRecords (*jsonVariations, "presets", VariationKind::preset);
    variations.snapshots = parseVariationRecords (*jsonVariations, "snapshots", VariationKind::snapshot);
    return variations;
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
    out << ",\n";
    out << "  \"variations\": ";
    appendVariationLibraryJson (out, document.variations);
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
    document.variations = parseVariationLibrary (root);

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
