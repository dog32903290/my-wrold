#include "GraphIOMappingStorage.h"

#include "StorageContractJson.h"

namespace myworld
{
namespace
{
using storage_contract_internal::JsonValue;
using storage_contract_internal::member;
using storage_contract_internal::numberMember;
using storage_contract_internal::readTextFile;
using storage_contract_internal::stringMember;

bool isObject (const JsonValue* value)
{
    return value != nullptr && value->kind == JsonValue::Kind::object;
}

GraphIOMapping mappingFromJson (const JsonValue& value)
{
    GraphIOMapping mapping;
    mapping.id = stringMember (value, "id");

    const auto* source = member (value, "source");
    if (isObject (source))
    {
        mapping.source.endpoint = stringMember (*source, "endpoint");
        mapping.source.dataType = stringMember (*source, "dataType");
        mapping.source.streamKind = stringMember (*source, "streamKind");
    }

    const auto* target = member (value, "target");
    if (isObject (target))
    {
        mapping.target.id = stringMember (*target, "id");
        mapping.target.kind = stringMember (*target, "kind");
        mapping.target.uniformName = stringMember (*target, "uniformName");
        mapping.target.dataType = stringMember (*target, "dataType");
    }

    const auto* range = member (value, "range");
    if (isObject (range))
    {
        mapping.inputMin = numberMember (*range, "inputMin", mapping.inputMin);
        mapping.inputMax = numberMember (*range, "inputMax", mapping.inputMax);
    }

    return mapping;
}
}

GraphIOMappingLoadResult loadGraphIOMappingsFromJsonText (const std::string& text)
{
    GraphIOMappingLoadResult result;

    storage_contract_internal::JsonParser parser (text);
    const auto root = parser.parse();
    if (! parser.ok())
    {
        result.error = parser.error();
        return result;
    }

    const auto* graph = member (root, "graph");
    if (! isObject (graph))
        graph = &root;

    const auto* mappings = member (*graph, "ioMappings");
    if (mappings == nullptr || mappings->kind != JsonValue::Kind::array)
    {
        result.error = "ioMappings array is required";
        return result;
    }

    for (const auto& item : mappings->arrayValue)
    {
        if (item.kind != JsonValue::Kind::object)
        {
            result.error = "ioMappings entries must be objects";
            result.mappings.clear();
            return result;
        }

        result.mappings.push_back (mappingFromJson (item));
    }

    result.ok = true;
    return result;
}

GraphIOMappingLoadResult loadGraphIOMappingsFromFile (const std::string& path)
{
    std::string error;
    const auto text = readTextFile (path, error);
    if (! error.empty())
        return { false, {}, error };

    return loadGraphIOMappingsFromJsonText (text);
}
}
