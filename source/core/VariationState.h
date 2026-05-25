#pragma once

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

struct VariationCaptureOptions
{
    std::vector<std::string> excludedParamIds;
};

std::string variationKindToString (VariationKind kind);
VariationKind variationKindFromString (const std::string& kind);
std::string variationSkipReasonToString (VariationSkipReason reason);
VariationSkipReason variationSkipReasonFromString (const std::string& reason);
bool isPresetSupportedParamType (const std::string& dataType);
}
