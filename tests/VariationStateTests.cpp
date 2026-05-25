#include "InteractionContract.h"
#include "GraphEndpoint.h"
#include "StorageCommand.h"
#include "StorageContract.h"
#include "VariationState.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}

const myworld::GraphNode& requireNode (const myworld::GraphContract& graph, const std::string& nodeId)
{
    const auto* node = myworld::findEditorNode (graph, nodeId);
    expect (node != nullptr, "missing node " + nodeId);
    return *node;
}

std::string paramValue (const myworld::GraphNode& node, const std::string& paramId)
{
    for (const auto& param : node.params)
        if (param.id == paramId)
            return param.value;

    return {};
}

bool hasSkipReason (const myworld::VariationRecord& record,
                    const std::string& paramId,
                    myworld::VariationSkipReason reason)
{
    for (const auto& skipped : record.skippedValues)
        if (skipped.paramId == paramId && skipped.reason == reason)
            return true;

    return false;
}

const myworld::VariationRecord& requireVariation (const std::vector<myworld::VariationRecord>& records,
                                                  const std::string& variationId)
{
    for (const auto& record : records)
        if (record.id == variationId)
            return record;

    expect (false, "missing variation " + variationId);
    return records.front();
}

const myworld::VariationPreviewValue& requirePreviewValue (const myworld::VariationPreviewReport& report,
                                                           const std::string& nodeId,
                                                           const std::string& paramId)
{
    for (const auto& value : report.values)
        if (value.nodeId == nodeId && value.paramId == paramId)
            return value;

    expect (false, "missing preview value " + nodeId + "." + paramId);
    return report.values.front();
}

bool hasVariation (const std::vector<myworld::VariationRecord>& records, const std::string& variationId)
{
    for (const auto& record : records)
        if (record.id == variationId)
            return true;

    return false;
}

void expectThumbnailHit (const myworld::VariationThumbnailHitTest& hit,
                         myworld::VariationKind kind,
                         const std::string& variationId,
                         const std::string& message)
{
    expect (hit.hit, message + " hit");
    expect (hit.kind == kind, message + " kind");
    expect (hit.variationId == variationId, message + " id");
}

void writeText (const std::filesystem::path& path, const std::string& text)
{
    std::filesystem::create_directories (path.parent_path());
    std::ofstream output (path, std::ios::trunc);
    expect (static_cast<bool> (output), "open " + path.string());
    output << text;
    expect (static_cast<bool> (output), "write " + path.string());
}
}

int main()
{
    myworld::NodeSpec spec;
    spec.type = "test.variation";
    spec.displayName = "Variation Test";
    spec.params = {
        { "gain", "Gain", "float", "1.0", "0.0..8.0" },
        { "mode", "Mode", "enum", "soft", "soft|hard" },
        { "secret", "Secret", "string", "", "" },
        { "curve", "Curve", "curve", "", "" },
        { "empty", "Empty", "float", "", "" },
        { "threshold", "Threshold", "float", "0.5", "0.0..1.0" }
    };

    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    const std::vector<myworld::NodeSpec> specs { spec };
    expect (myworld::createNode (session, specs, "test.variation", "var1", { 120.0, 160.0 }).ok,
            "create variation node");
    expect (myworld::setParam (session, "var1", "gain", "2.5").ok, "set gain");
    expect (myworld::setParam (session, "var1", "mode", "hard").ok, "set mode");
    expect (myworld::setParam (session, "var1", "secret", "keep-out").ok, "set secret");
    expect (myworld::setParam (session, "var1", "curve", "0,1,0").ok, "set curve");

    myworld::VariationCaptureOptions presetOptions;
    presetOptions.excludedParamIds = { "secret" };
    expect (myworld::createPreset (session, "var1", spec, "preset.hot", "Hot", presetOptions).ok,
            "create preset");
    expect (session.commandLog.back() == "create_preset", "create preset command logged");
    expect (session.variations.presets.size() == 1, "preset stored separately");
    expect (session.variations.snapshots.empty(), "snapshot list remains separate");

    const auto& preset = session.variations.presets.front();
    expect (preset.kind == myworld::VariationKind::preset, "preset kind");
    expect (preset.values.size() == 2, "preset captures two non-default supported values");
    expect (hasSkipReason (preset, "secret", myworld::VariationSkipReason::excludedFromPresets),
            "preset records excluded skip");
    expect (hasSkipReason (preset, "curve", myworld::VariationSkipReason::unsupportedType),
            "preset records unsupported skip");
    expect (hasSkipReason (preset, "empty", myworld::VariationSkipReason::missingInput),
            "preset records missing input skip");
    expect (hasSkipReason (preset, "threshold", myworld::VariationSkipReason::defaultValue),
            "preset records default skip");

    expect (myworld::setParam (session, "var1", "gain", "7.0").ok, "change gain before apply");
    expect (myworld::setParam (session, "var1", "mode", "soft").ok, "change mode before apply");
    expect (myworld::applyPreset (session, "preset.hot").ok, "apply preset");
    expect (session.commandLog.back() == "apply_preset", "apply preset command logged");
    expect (paramValue (requireNode (session.graph, "var1"), "gain") == "2.5", "preset restores gain");
    expect (paramValue (requireNode (session.graph, "var1"), "mode") == "hard", "preset restores mode");
    expect (paramValue (requireNode (session.graph, "var1"), "secret") == "keep-out", "preset does not touch excluded");
    expect (myworld::undo (session), "undo apply preset");
    expect (paramValue (requireNode (session.graph, "var1"), "gain") == "7.0", "undo restores gain");

    expect (myworld::setParam (session, "var1", "gain", "3.0").ok, "set snapshot gain");
    expect (myworld::createSnapshot (session, "snapshot.one", "One", { "var1" }).ok, "create snapshot");
    expect (session.commandLog.back() == "create_snapshot", "create snapshot command logged");
    expect (session.variations.snapshots.size() == 1, "snapshot stored separately");
    expect (session.variations.snapshots.front().kind == myworld::VariationKind::snapshot, "snapshot kind");
    expect (session.variations.snapshots.front().enabledNodeIds.size() == 1, "snapshot records enabled node");

    expect (myworld::setParam (session, "var1", "gain", "6.0").ok, "change gain before snapshot apply");
    expect (myworld::applySnapshot (session, "snapshot.one").ok, "apply snapshot");
    expect (session.commandLog.back() == "apply_snapshot", "apply snapshot command logged");
    expect (paramValue (requireNode (session.graph, "var1"), "gain") == "3.0", "snapshot restores gain");

    expect (myworld::setParam (session, "var1", "gain", "4.0").ok, "set second preset gain");
    expect (myworld::setParam (session, "var1", "mode", "hard").ok, "set second preset mode");
    expect (myworld::createPreset (session, "var1", spec, "preset.alt", "Alt", {}).ok,
            "create second preset");

    expect (myworld::setParam (session, "var1", "gain", "8.0").ok, "set current gain before preview");
    expect (myworld::setParam (session, "var1", "mode", "soft").ok, "set current mode before preview");
    const auto commandCountBeforePreview = session.commandLog.size();
    const auto halfPreview = myworld::previewVariationBlend (session,
                                                             myworld::VariationKind::preset,
                                                             "preset.alt",
                                                             0.5);
    expect (halfPreview.ok, halfPreview.message);
    expect (std::abs (halfPreview.weight - 0.5) < 0.000001, "preview keeps requested weight");
    expect (session.commandLog.size() == commandCountBeforePreview, "preview does not log command");
    expect (paramValue (requireNode (session.graph, "var1"), "gain") == "8.0",
            "preview does not mutate numeric param");
    expect (paramValue (requireNode (session.graph, "var1"), "mode") == "soft",
            "preview does not mutate stepped param");

    const auto& gainPreview = requirePreviewValue (halfPreview, "var1", "gain");
    expect (gainPreview.status == myworld::VariationPreviewValueStatus::blended,
            "numeric preview is blended got " + myworld::variationPreviewValueStatusToString (gainPreview.status)
                + " current=" + gainPreview.currentValue + " target=" + gainPreview.targetValue);
    expect (std::abs (std::stod (gainPreview.previewValue) - 6.0) < 0.000001,
            "numeric preview blends current and target");

    const auto& modePreview = requirePreviewValue (halfPreview, "var1", "mode");
    expect (modePreview.status == myworld::VariationPreviewValueStatus::stepped,
            "enum preview is stepped");
    expect (modePreview.previewValue == "soft", "stepped preview stays current before full weight");

    const auto fullPreview = myworld::previewVariationBlend (session,
                                                             myworld::VariationKind::preset,
                                                             "preset.alt",
                                                             1.0);
    expect (fullPreview.ok, fullPreview.message);
    expect (requirePreviewValue (fullPreview, "var1", "mode").previewValue == "hard",
            "full stepped preview reaches target");

    expect (! myworld::previewVariationBlend (session,
                                              myworld::VariationKind::preset,
                                              "missing",
                                              0.5)
                .ok,
            "preview rejects missing variation");

    expect (myworld::commitVariationBlend (session, myworld::VariationKind::preset, "preset.alt", 0.5).ok,
            "commit half blend");
    expect (session.commandLog.back() == "apply_variation_blend", "variation blend command logged");
    expect (std::abs (std::stod (paramValue (requireNode (session.graph, "var1"), "gain")) - 6.0) < 0.000001,
            "committed half blend sets numeric param");
    expect (paramValue (requireNode (session.graph, "var1"), "mode") == "soft",
            "committed half blend leaves stepped param current");
    expect (myworld::undo (session), "undo half blend");
    expect (paramValue (requireNode (session.graph, "var1"), "gain") == "8.0",
            "undo half blend restores gain");
    expect (paramValue (requireNode (session.graph, "var1"), "mode") == "soft",
            "undo half blend restores mode");

    expect (myworld::commitVariationBlend (session, myworld::VariationKind::preset, "preset.alt", 1.0).ok,
            "commit full blend");
    expect (paramValue (requireNode (session.graph, "var1"), "mode") == "hard",
            "committed full blend steps enum target");
    expect (myworld::undo (session), "undo full blend");
    expect (! myworld::commitVariationBlend (session, myworld::VariationKind::snapshot, "missing", 0.5).ok,
            "commit blend rejects missing variation");

    myworld::VariationSelection selectedPreset;
    selectedPreset.kind = myworld::VariationKind::preset;
    selectedPreset.variationId = "preset.alt";

    myworld::VariationThumbnailLayoutOptions thumbnailOptions;
    thumbnailOptions.originX = 10.0;
    thumbnailOptions.originY = 20.0;
    thumbnailOptions.availableWidth = 148.0;
    thumbnailOptions.thumbnailWidth = 64.0;
    thumbnailOptions.thumbnailHeight = 44.0;
    thumbnailOptions.gapX = 8.0;
    thumbnailOptions.gapY = 12.0;
    thumbnailOptions.labelHeight = 14.0;

    const auto presetThumbnails = myworld::makeVariationThumbnailLayout (session.variations,
                                                                         myworld::VariationKind::preset,
                                                                         selectedPreset,
                                                                         thumbnailOptions);
    expect (presetThumbnails.items.size() == 2, "preset thumbnails include both presets");
    expect (presetThumbnails.items.front().variationId == "preset.hot", "preset thumbnail keeps record order");
    expect (presetThumbnails.items[1].selected, "selected preset thumbnail marked");
    expect (presetThumbnails.items[1].valueCount == 3, "preset thumbnail records captured value count");
    expect (presetThumbnails.items[1].previewBounds.width == thumbnailOptions.thumbnailWidth,
            "preset thumbnail preview width");
    expect (presetThumbnails.contentHeight > thumbnailOptions.thumbnailHeight, "preset thumbnail content height");

    expectThumbnailHit (myworld::hitTestVariationThumbnails (presetThumbnails,
                                                             presetThumbnails.items[1].bounds.x + 4.0,
                                                             presetThumbnails.items[1].bounds.y + 4.0),
                        myworld::VariationKind::preset,
                        "preset.alt",
                        "preset thumbnail");
    expect (! myworld::hitTestVariationThumbnails (presetThumbnails,
                                                   presetThumbnails.items[0].bounds.x + thumbnailOptions.thumbnailWidth + 2.0,
                                                   presetThumbnails.items[0].bounds.y + 4.0)
                .hit,
            "gap between thumbnails is not a hit");

    const auto snapshotThumbnails = myworld::makeVariationThumbnailLayout (session.variations,
                                                                           myworld::VariationKind::snapshot,
                                                                           {},
                                                                           thumbnailOptions);
    expect (snapshotThumbnails.items.size() == 1, "snapshot thumbnails stay separate");
    expect (snapshotThumbnails.items.front().kind == myworld::VariationKind::snapshot, "snapshot thumbnail kind");
    expect (snapshotThumbnails.items.front().enabledNodeCount == 1, "snapshot thumbnail records enabled nodes");

    const auto commandCountBeforeSelect = session.commandLog.size();
    session.selectedNodeIds = { "shader1" };
    session.selectedEdgeIds = { "edge.shader1.output.out1.input" };
    expect (myworld::selectVariation (session, myworld::VariationKind::preset, "preset.alt").ok,
            "select preset thumbnail");
    expect (myworld::hasVariationSelection (session.selectedVariation), "variation selection present");
    expect (myworld::variationSelectionMatches (session.selectedVariation,
                                                myworld::VariationKind::preset,
                                                "preset.alt"),
            "selected preset thumbnail id");
    expect (session.selectedNodeIds.empty(), "variation selection clears node selection");
    expect (session.selectedEdgeIds.empty(), "variation selection clears edge selection");
    expect (session.commandLog.size() == commandCountBeforeSelect, "variation selection does not log command");
    expect (! myworld::selectVariation (session, myworld::VariationKind::preset, "missing").ok,
            "select variation rejects missing thumbnail");

    expect (myworld::renameVariation (session, myworld::VariationKind::preset, "preset.alt", "Alt Renamed").ok,
            "rename preset");
    expect (session.commandLog.back() == "rename_variation", "rename variation command logged");
    expect (requireVariation (session.variations.presets, "preset.alt").title == "Alt Renamed",
            "preset title renamed");
    expect (myworld::undo (session), "undo rename variation");
    expect (requireVariation (session.variations.presets, "preset.alt").title == "Alt",
            "undo restores preset title");
    expect (myworld::redo (session), "redo rename variation");
    expect (requireVariation (session.variations.presets, "preset.alt").title == "Alt Renamed",
            "redo reapplies preset title");

    expect (myworld::moveVariation (session, myworld::VariationKind::preset, "preset.alt", 0).ok,
            "move preset to front");
    expect (session.commandLog.back() == "move_variation", "move variation command logged");
    expect (session.variations.presets.front().id == "preset.alt", "preset moved to front");
    expect (myworld::undo (session), "undo move variation");
    expect (session.variations.presets.front().id == "preset.hot", "undo restores preset order");
    expect (myworld::redo (session), "redo move variation");
    expect (session.variations.presets.front().id == "preset.alt", "redo reapplies preset order");

    expect (! myworld::renameVariation (session, myworld::VariationKind::preset, "preset.alt", "").ok,
            "rename variation rejects empty title");
    expect (! myworld::moveVariation (session, myworld::VariationKind::preset, "preset.alt", 9).ok,
            "move variation rejects out of range");

    expect (myworld::deleteVariation (session, myworld::VariationKind::preset, "preset.hot").ok,
            "delete preset");
    expect (session.commandLog.back() == "delete_variation", "delete variation command logged");
    expect (! hasVariation (session.variations.presets, "preset.hot"), "preset deleted");
    expect (myworld::undo (session), "undo delete preset");
    expect (hasVariation (session.variations.presets, "preset.hot"), "undo restores deleted preset");

    expect (myworld::selectVariation (session, myworld::VariationKind::preset, "preset.hot").ok,
            "select preset before delete");
    expect (myworld::deleteVariation (session, myworld::VariationKind::preset, "preset.hot").ok,
            "delete selected preset");
    expect (! myworld::hasVariationSelection (session.selectedVariation), "delete clears selected variation");
    expect (myworld::undo (session), "undo selected preset delete");
    expect (myworld::variationSelectionMatches (session.selectedVariation,
                                                myworld::VariationKind::preset,
                                                "preset.hot"),
            "undo selected preset delete restores selection");

    expect (myworld::renameVariation (session, myworld::VariationKind::snapshot, "snapshot.one", "Snapshot Renamed").ok,
            "rename snapshot");
    expect (requireVariation (session.variations.snapshots, "snapshot.one").title == "Snapshot Renamed",
            "snapshot title renamed");
    expect (myworld::deleteVariation (session, myworld::VariationKind::snapshot, "snapshot.one").ok,
            "delete snapshot");
    expect (session.variations.snapshots.empty(), "snapshot deleted");
    expect (myworld::undo (session), "undo delete snapshot");
    expect (requireVariation (session.variations.snapshots, "snapshot.one").title == "Snapshot Renamed",
            "undo restores snapshot");

    const auto document = myworld::makePatchDocument ("patch.variations",
                                                      "Variations",
                                                      session.graph,
                                                      session.outputView,
                                                      session.timeline,
                                                      session.variations);
    const auto json = myworld::toJson (document);
    expect (json.find ("\"presets\"") != std::string::npos, "json writes presets");
    expect (json.find ("\"snapshots\"") != std::string::npos, "json writes snapshots");
    const auto parsed = myworld::parsePatchDocument (json);
    expect (parsed.ok, parsed.error);
    expect (parsed.document.variations.presets.size() == 2, "patch reloads presets");
    expect (parsed.document.variations.snapshots.size() == 1, "patch reloads snapshots");
    expect (requireVariation (parsed.document.variations.presets, "preset.hot").values.size() == 2,
            "patch reloads preset values");
    expect (requireVariation (parsed.document.variations.presets, "preset.hot").skippedValues.size() == 4,
            "patch reloads skip reasons");

    const auto preserveRoot = std::filesystem::temp_directory_path() / "my-world-variation-preserve-tests";
    std::filesystem::remove_all (preserveRoot);
    const auto preserveManifestPath = preserveRoot / "myworld.work.json";
    const auto preservePatchPath = preserveRoot / "patches" / "main.patch.json";

    writeText (preserveManifestPath,
               myworld::toJson (myworld::makeMinimalWorkProject ("work.variation-preserve",
                                                                  "Variation Preserve Work")));
    const auto preserveInitialSave = myworld::savePatchDocument (preservePatchPath.string(), document);
    expect (preserveInitialSave.ok, preserveInitialSave.error);

    const auto preserveLoaded = myworld::loadMainPatchDocumentForWork (preserveManifestPath.string());
    expect (preserveLoaded.ok, preserveLoaded.error);
    auto shallowSession = myworld::makeGraphSession (preserveLoaded.document.graph);
    shallowSession.dirty = true;
    const auto preserveSave = myworld::saveWork (shallowSession, preserveManifestPath.string());
    expect (preserveSave.ok, preserveSave.error);

    const auto preserveReloaded = myworld::loadMainPatchDocumentForWork (preserveManifestPath.string());
    expect (preserveReloaded.ok, preserveReloaded.error);
    expect (preserveReloaded.document.variations.presets.size() == 2,
            "save_work preserves existing presets when session was not variation-hydrated");
    expect (preserveReloaded.document.variations.snapshots.size() == 1,
            "save_work preserves existing snapshots when session was not variation-hydrated");

    std::filesystem::remove_all (preserveRoot);

    const auto root = std::filesystem::temp_directory_path() / "my-world-variation-state-tests";
    std::filesystem::remove_all (root);
    const auto manifestPath = root / "myworld.work.json";
    const auto patchPath = root / "patches" / "main.patch.json";

    writeText (manifestPath, myworld::toJson (myworld::makeMinimalWorkProject ("work.variation", "Variation Work")));
    const auto initialSave = myworld::savePatchDocument (
        patchPath.string(),
        myworld::makePatchDocument ("patch.variation-main",
                                    "Variation Main",
                                    myworld::makeDefaultShaderOutputGraph()));
    expect (initialSave.ok, initialSave.error);

    const auto saveResult = myworld::saveWork (session, manifestPath.string());
    expect (saveResult.ok, saveResult.error);

    const auto reloadedMain = myworld::loadMainPatchDocumentForWork (manifestPath.string());
    expect (reloadedMain.ok, reloadedMain.error);
    expect (reloadedMain.document.variations.presets.size() == 2, "save_work preserves presets");
    expect (reloadedMain.document.variations.snapshots.size() == 1, "save_work preserves snapshots");

    std::filesystem::remove_all (root);

    std::cout << "variation state ok\n";
    return 0;
}
