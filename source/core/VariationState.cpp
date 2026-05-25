#include "VariationState.h"

#include <algorithm>

namespace myworld
{
namespace
{
const std::vector<VariationRecord>& recordsForKind (const VariationLibrary& variations, VariationKind kind)
{
    return kind == VariationKind::preset ? variations.presets : variations.snapshots;
}

bool pointInBounds (double x, double y, VariationThumbnailBounds bounds)
{
    return x >= bounds.x
           && x <= bounds.x + bounds.width
           && y >= bounds.y
           && y <= bounds.y + bounds.height;
}
}

std::string variationKindToString (VariationKind kind)
{
    switch (kind)
    {
        case VariationKind::preset:   return "preset";
        case VariationKind::snapshot: return "snapshot";
    }

    return "preset";
}

VariationKind variationKindFromString (const std::string& kind)
{
    return kind == "snapshot" ? VariationKind::snapshot : VariationKind::preset;
}

std::string variationSkipReasonToString (VariationSkipReason reason)
{
    switch (reason)
    {
        case VariationSkipReason::defaultValue:          return "default";
        case VariationSkipReason::excludedFromPresets:   return "excludedFromPresets";
        case VariationSkipReason::unsupportedType:       return "unsupportedType";
        case VariationSkipReason::missingInput:          return "missingInput";
    }

    return "default";
}

VariationSkipReason variationSkipReasonFromString (const std::string& reason)
{
    if (reason == "excludedFromPresets")
        return VariationSkipReason::excludedFromPresets;

    if (reason == "unsupportedType")
        return VariationSkipReason::unsupportedType;

    if (reason == "missingInput")
        return VariationSkipReason::missingInput;

    return VariationSkipReason::defaultValue;
}

std::string variationPreviewValueStatusToString (VariationPreviewValueStatus status)
{
    switch (status)
    {
        case VariationPreviewValueStatus::blended:      return "blended";
        case VariationPreviewValueStatus::stepped:      return "stepped";
        case VariationPreviewValueStatus::missingNode:  return "missingNode";
        case VariationPreviewValueStatus::missingParam: return "missingParam";
    }

    return "stepped";
}

bool isPresetSupportedParamType (const std::string& dataType)
{
    return dataType == "float"
           || dataType == "double"
           || dataType == "int"
           || dataType == "bool"
           || dataType == "vec2"
           || dataType == "vec3"
           || dataType == "vec4"
           || dataType == "float2"
           || dataType == "float3"
           || dataType == "float4"
           || dataType == "quaternion"
           || dataType == "enum"
           || dataType == "string"
           || dataType == "resource.file"
           || dataType == "path"
           || dataType == "path.file"
           || dataType.rfind ("path.", 0) == 0
           || dataType.rfind ("text.", 0) == 0;
}

bool hasVariationSelection (const VariationSelection& selection)
{
    return ! selection.variationId.empty();
}

bool variationSelectionMatches (const VariationSelection& selection,
                                VariationKind kind,
                                const std::string& variationId)
{
    return selection.kind == kind && selection.variationId == variationId && ! variationId.empty();
}

VariationThumbnailLayout makeVariationThumbnailLayout (const VariationLibrary& variations,
                                                       VariationKind kind,
                                                       const VariationSelection& selection,
                                                       const VariationThumbnailLayoutOptions& options)
{
    VariationThumbnailLayout layout;
    const auto& records = recordsForKind (variations, kind);
    if (records.empty())
        return layout;

    const auto tileWidth = std::max (1.0, options.thumbnailWidth);
    const auto previewHeight = std::max (1.0, options.thumbnailHeight);
    const auto labelHeight = std::max (0.0, options.labelHeight);
    const auto tileHeight = previewHeight + labelHeight;
    const auto gapX = std::max (0.0, options.gapX);
    const auto gapY = std::max (0.0, options.gapY);
    const auto availableWidth = std::max (tileWidth, options.availableWidth);
    const auto columns = std::max<std::size_t> (
        1,
        static_cast<std::size_t> ((availableWidth + gapX) / (tileWidth + gapX)));

    layout.items.reserve (records.size());

    for (std::size_t index = 0; index < records.size(); ++index)
    {
        const auto column = index % columns;
        const auto row = index / columns;
        const auto x = options.originX + static_cast<double> (column) * (tileWidth + gapX);
        const auto y = options.originY + static_cast<double> (row) * (tileHeight + gapY);

        VariationThumbnailItem item;
        item.kind = kind;
        item.variationId = records[index].id;
        item.title = records[index].title;
        item.index = index;
        item.bounds = { x, y, tileWidth, tileHeight };
        item.previewBounds = { x, y, tileWidth, previewHeight };
        item.labelBounds = { x, y + previewHeight, tileWidth, labelHeight };
        item.selected = variationSelectionMatches (selection, kind, records[index].id);
        item.valueCount = static_cast<int> (records[index].values.size());
        item.enabledNodeCount = static_cast<int> (records[index].enabledNodeIds.size());
        layout.items.push_back (item);
    }

    const auto rowCount = (records.size() + columns - 1) / columns;
    layout.contentHeight = static_cast<double> (rowCount) * tileHeight
                           + static_cast<double> (rowCount - 1) * gapY;
    return layout;
}

VariationThumbnailHitTest hitTestVariationThumbnails (const VariationThumbnailLayout& layout,
                                                      double x,
                                                      double y)
{
    for (auto iter = layout.items.rbegin(); iter != layout.items.rend(); ++iter)
    {
        if (! pointInBounds (x, y, iter->bounds))
            continue;

        return { true, iter->kind, iter->variationId, iter->index };
    }

    return {};
}
}
