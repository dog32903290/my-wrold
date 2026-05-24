#pragma once

#include "StorageContract.h"

#include <filesystem>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace myworld
{
namespace storage_contract_internal
{
struct JsonValue
{
    enum class Kind
    {
        nullValue,
        string,
        boolean,
        number,
        array,
        object
    };

    Kind kind = Kind::nullValue;
    std::string stringValue;
    bool boolValue = false;
    double numberValue = 0.0;
    std::vector<JsonValue> arrayValue;
    std::map<std::string, JsonValue> objectValue;
};

class JsonParser
{
public:
    explicit JsonParser (const std::string& sourceText);

    JsonValue parse();
    bool ok() const;
    std::string error() const;

private:
    JsonValue parseValue();
    JsonValue parseString();
    JsonValue parseNumber();
    JsonValue parseObject();
    JsonValue parseArray();
    void skipWhitespace();
    bool consume (char expected);
    JsonValue fail (const std::string& message);

    const std::string& source;
    size_t position = 0;
    bool failed = false;
    std::string errorMessage;
};

const JsonValue* member (const JsonValue& value, const std::string& name);
std::string stringMember (const JsonValue& value, const std::string& name);
bool boolMember (const JsonValue& value, const std::string& name, bool fallback);
double numberMember (const JsonValue& value, const std::string& name, double fallback);
int intMember (const JsonValue& value, const std::string& name, int fallback);
std::vector<std::string> stringArrayMember (const JsonValue& value, const std::string& name);

void appendGraphSectionJson (std::ostringstream& out,
                             const std::vector<GraphNode>& nodes,
                             const std::vector<GraphEdge>& edges);
bool parseEditorGraph (EditorGraph& graph, const JsonValue& graphSection);
bool parseRuntimeGraph (RuntimeGraph& graph, const JsonValue& graphSection);
std::string readTextFile (const std::string& path, std::string& error);
std::filesystem::path resolvePathNearFile (const std::string& filePath, const std::string& relativeOrAbsolutePath);
}
}
