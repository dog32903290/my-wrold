#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace myworld
{
enum class VariationKind
{
    preset,
    snapshot
};

enum class VariationSkipReason
{
    defaultValue,
    excludedFromPresets,
    unsupportedType,
    missingInput
};

struct VariationValue
{
    std::string nodeId;
    std::string paramId;
    std::string value;
};

struct VariationSkippedValue
{
    std::string nodeId;
    std::string paramId;
    VariationSkipReason reason = VariationSkipReason::defaultValue;
};

enum class VariationPreviewValueStatus
{
    blended,
    stepped,
    missingNode,
    missingParam
};

struct VariationPreviewValue
{
    std::string nodeId;
    std::string paramId;
    std::string currentValue;
    std::string targetValue;
    std::string previewValue;
    VariationPreviewValueStatus status = VariationPreviewValueStatus::stepped;
};

struct VariationPreviewReport
{
    bool ok = false;
    std::string message;
    VariationKind kind = VariationKind::preset;
    std::string variationId;
    double weight = 0.0;
    std::vector<VariationPreviewValue> values;
    std::vector<std::string> errors;
};

struct VariationRecord
{
    std::string id;
    std::string title;
    VariationKind kind = VariationKind::preset;
    std::vector<VariationValue> values;
    std::vector<std::string> enabledNodeIds;
    std::vector<VariationSkippedValue> skippedValues;
};

struct VariationLibrary
{
    std::vector<VariationRecord> presets;
    std::vector<VariationRecord> snapshots;
};

struct VariationSelection
{
    VariationKind kind = VariationKind::preset;
    std::string variationId;
};

struct VariationThumbnailBounds
{
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
};

struct VariationThumbnailLayoutOptions
{
    double originX = 0.0;
    double originY = 0.0;
    double availableWidth = 148.0;
    double thumbnailWidth = 64.0;
    double thumbnailHeight = 44.0;
    double gapX = 8.0;
    double gapY = 12.0;
    double labelHeight = 14.0;
};

struct VariationThumbnailItem
{
    VariationKind kind = VariationKind::preset;
    std::string variationId;
    std::string title;
    std::size_t index = 0;
    VariationThumbnailBounds bounds;
    VariationThumbnailBounds previewBounds;
    VariationThumbnailBounds labelBounds;
    bool selected = false;
    int valueCount = 0;
    int enabledNodeCount = 0;
};

struct VariationThumbnailLayout
{
    std::vector<VariationThumbnailItem> items;
    double contentHeight = 0.0;
};

struct VariationThumbnailHitTest
{
    bool hit = false;
    VariationKind kind = VariationKind::preset;
    std::string variationId;
    std::size_t index = 0;
};

struct VariationCaptureOptions
{
    std::vector<std::string> excludedParamIds;
};

std::string variationKindToString (VariationKind kind);
VariationKind variationKindFromString (const std::string& kind);
std::string variationSkipReasonToString (VariationSkipReason reason);
VariationSkipReason variationSkipReasonFromString (const std::string& reason);
std::string variationPreviewValueStatusToString (VariationPreviewValueStatus status);
bool isPresetSupportedParamType (const std::string& dataType);
bool hasVariationSelection (const VariationSelection& selection);
bool variationSelectionMatches (const VariationSelection& selection,
                                VariationKind kind,
                                const std::string& variationId);
VariationThumbnailLayout makeVariationThumbnailLayout (const VariationLibrary& variations,
                                                       VariationKind kind,
                                                       const VariationSelection& selection = {},
                                                       const VariationThumbnailLayoutOptions& options = {});
VariationThumbnailHitTest hitTestVariationThumbnails (const VariationThumbnailLayout& layout,
                                                      double x,
                                                      double y);
}
