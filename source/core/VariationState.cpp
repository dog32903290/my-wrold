#include "VariationState.h"

namespace myworld
{
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
}
