#include "StorageContractJson.h"

#include "JsonWriter.h"

#include <cctype>
#include <cstdlib>
#include <fstream>

namespace myworld::storage_contract_internal
{
JsonParser::JsonParser (const std::string& sourceText)
    : source (sourceText)
{
}

JsonValue JsonParser::parse()
{
    auto value = parseValue();
    skipWhitespace();

    if (! failed && position != source.size())
        fail ("trailing characters");

    return value;
}

bool JsonParser::ok() const
{
    return ! failed;
}

std::string JsonParser::error() const
{
    return errorMessage;
}

JsonValue JsonParser::parseValue()
{
    skipWhitespace();

    if (position >= source.size())
        return fail ("unexpected end of json");

    const auto current = source[position];
    if (current == '"')
        return parseString();

    if (current == '{')
        return parseObject();

    if (current == '[')
        return parseArray();

    if (current == '-' || current == '+' || std::isdigit (static_cast<unsigned char> (current)) != 0)
        return parseNumber();

    if (source.compare (position, 4, "true") == 0)
    {
        position += 4;
        JsonValue value;
        value.kind = JsonValue::Kind::boolean;
        value.boolValue = true;
        return value;
    }

    if (source.compare (position, 5, "false") == 0)
    {
        position += 5;
        JsonValue value;
        value.kind = JsonValue::Kind::boolean;
        value.boolValue = false;
        return value;
    }

    if (source.compare (position, 4, "null") == 0)
    {
        position += 4;
        return {};
    }

    return fail ("unsupported json value");
}

JsonValue JsonParser::parseString()
{
    JsonValue value;
    value.kind = JsonValue::Kind::string;

    if (! consume ('"'))
        return fail ("expected string");

    while (position < source.size())
    {
        const auto current = source[position++];

        if (current == '"')
            return value;

        if (current == '\\')
        {
            if (position >= source.size())
                return fail ("unterminated escape");

            const auto escaped = source[position++];
            switch (escaped)
            {
                case '"':  value.stringValue.push_back ('"'); break;
                case '\\': value.stringValue.push_back ('\\'); break;
                case '/':  value.stringValue.push_back ('/'); break;
                case 'b':  value.stringValue.push_back ('\b'); break;
                case 'f':  value.stringValue.push_back ('\f'); break;
                case 'n':  value.stringValue.push_back ('\n'); break;
                case 'r':  value.stringValue.push_back ('\r'); break;
                case 't':  value.stringValue.push_back ('\t'); break;
                default:   return fail ("unsupported string escape");
            }
        }
        else
        {
            value.stringValue.push_back (current);
        }
    }

    return fail ("unterminated string");
}

JsonValue JsonParser::parseNumber()
{
    const auto start = position;

    while (position < source.size())
    {
        const auto current = source[position];
        if (std::isdigit (static_cast<unsigned char> (current)) == 0
            && current != '-' && current != '+' && current != '.' && current != 'e' && current != 'E')
        {
            break;
        }

        ++position;
    }

    char* end = nullptr;
    const auto value = std::strtod (source.c_str() + start, &end);
    if (end != source.c_str() + position)
        return fail ("invalid number");

    JsonValue result;
    result.kind = JsonValue::Kind::number;
    result.numberValue = value;
    return result;
}

JsonValue JsonParser::parseObject()
{
    JsonValue value;
    value.kind = JsonValue::Kind::object;

    if (! consume ('{'))
        return fail ("expected object");

    skipWhitespace();
    if (consume ('}'))
        return value;

    while (! failed)
    {
        auto key = parseString();
        if (key.kind != JsonValue::Kind::string)
            return fail ("expected object key");

        skipWhitespace();
        if (! consume (':'))
            return fail ("expected ':' after object key");

        value.objectValue[key.stringValue] = parseValue();
        skipWhitespace();

        if (consume ('}'))
            return value;

        if (! consume (','))
            return fail ("expected ',' between object members");
    }

    return value;
}

JsonValue JsonParser::parseArray()
{
    JsonValue value;
    value.kind = JsonValue::Kind::array;

    if (! consume ('['))
        return fail ("expected array");

    skipWhitespace();
    if (consume (']'))
        return value;

    while (! failed)
    {
        value.arrayValue.push_back (parseValue());
        skipWhitespace();

        if (consume (']'))
            return value;

        if (! consume (','))
            return fail ("expected ',' between array items");
    }

    return value;
}

void JsonParser::skipWhitespace()
{
    while (position < source.size() && std::isspace (static_cast<unsigned char> (source[position])) != 0)
        ++position;
}

bool JsonParser::consume (char expected)
{
    skipWhitespace();

    if (position < source.size() && source[position] == expected)
    {
        ++position;
        return true;
    }

    return false;
}

JsonValue JsonParser::fail (const std::string& message)
{
    failed = true;
    errorMessage = message;
    return {};
}

const JsonValue* member (const JsonValue& value, const std::string& name)
{
    if (value.kind != JsonValue::Kind::object)
        return nullptr;

    const auto found = value.objectValue.find (name);
    return found == value.objectValue.end() ? nullptr : &found->second;
}

std::string stringMember (const JsonValue& value, const std::string& name)
{
    const auto* found = member (value, name);
    return found != nullptr && found->kind == JsonValue::Kind::string ? found->stringValue : std::string {};
}

bool boolMember (const JsonValue& value, const std::string& name, bool fallback)
{
    const auto* found = member (value, name);
    return found != nullptr && found->kind == JsonValue::Kind::boolean ? found->boolValue : fallback;
}

double numberMember (const JsonValue& value, const std::string& name, double fallback)
{
    const auto* found = member (value, name);
    return found != nullptr && found->kind == JsonValue::Kind::number ? found->numberValue : fallback;
}

int intMember (const JsonValue& value, const std::string& name, int fallback)
{
    return static_cast<int> (numberMember (value, name, static_cast<double> (fallback)));
}

std::vector<std::string> stringArrayMember (const JsonValue& value, const std::string& name)
{
    std::vector<std::string> result;
    const auto* array = member (value, name);

    if (array == nullptr || array->kind != JsonValue::Kind::array)
        return result;

    for (const auto& item : array->arrayValue)
    {
        if (item.kind != JsonValue::Kind::string)
            return {};

        result.push_back (item.stringValue);
    }

    return result;
}

namespace
{
void appendParamValuesJson (std::ostringstream& out, const std::vector<GraphNode::ParamValue>& params)
{
    out << "[";

    for (size_t index = 0; index < params.size(); ++index)
    {
        const auto& param = params[index];
        if (index != 0)
            out << ", ";

        out << "{ \"id\": " << jsonQuoted (param.id)
            << ", \"value\": " << jsonQuoted (param.value) << " }";
    }

    out << "]";
}

void appendPortBindingsJson (std::ostringstream& out, const std::vector<GraphNode::PortBindingValue>& bindings)
{
    out << "[";

    for (size_t index = 0; index < bindings.size(); ++index)
    {
        const auto& binding = bindings[index];
        if (index != 0)
            out << ", ";

        out << "{ \"portId\": " << jsonQuoted (binding.portId)
            << ", \"bindingMode\": " << jsonQuoted (binding.bindingMode)
            << ", \"value\": " << jsonQuoted (binding.value) << " }";
    }

    out << "]";
}

void appendGraphNodeJson (std::ostringstream& out, const GraphNode& node)
{
    out << "{ \"id\": " << jsonQuoted (node.id)
        << ", \"type\": " << jsonQuoted (node.type)
        << ", \"position\": { \"x\": " << node.position.x
        << ", \"y\": " << node.position.y << " }"
        << ", \"collapsed\": " << (node.collapsed ? "true" : "false")
        << ", \"systemUniforms\": ";
    appendJsonStringArray (out, node.systemUniforms);
    out << ", \"params\": ";
    appendParamValuesJson (out, node.params);
    out << ", \"portBindings\": ";
    appendPortBindingsJson (out, node.portBindings);
    out << " }";
}

void appendGraphEdgeJson (std::ostringstream& out, const GraphEdge& edge)
{
    out << "{ \"id\": " << jsonQuoted (edge.id)
        << ", \"from\": " << jsonQuoted (edge.from)
        << ", \"to\": " << jsonQuoted (edge.to)
        << ", \"dataType\": " << jsonQuoted (edge.dataType)
        << ", \"streamKind\": " << jsonQuoted (edge.streamKind) << " }";
}

bool appendParsedParams (GraphNode& node, const JsonValue& jsonNode)
{
    const auto* params = member (jsonNode, "params");
    if (params == nullptr)
        return true;

    if (params->kind != JsonValue::Kind::array)
        return false;

    for (const auto& param : params->arrayValue)
    {
        if (param.kind != JsonValue::Kind::object)
            return false;

        node.params.push_back ({ stringMember (param, "id"), stringMember (param, "value") });
    }

    return true;
}

bool appendParsedPortBindings (GraphNode& node, const JsonValue& jsonNode)
{
    const auto* bindings = member (jsonNode, "portBindings");
    if (bindings == nullptr)
        return true;

    if (bindings->kind != JsonValue::Kind::array)
        return false;

    for (const auto& binding : bindings->arrayValue)
    {
        if (binding.kind != JsonValue::Kind::object)
            return false;

        node.portBindings.push_back ({ stringMember (binding, "portId"),
                                       stringMember (binding, "bindingMode"),
                                       stringMember (binding, "value") });
    }

    return true;
}

bool appendParsedNodes (std::vector<GraphNode>& nodes, const JsonValue& graphSection)
{
    const auto* jsonNodes = member (graphSection, "nodes");
    if (jsonNodes == nullptr || jsonNodes->kind != JsonValue::Kind::array)
        return false;

    for (const auto& jsonNode : jsonNodes->arrayValue)
    {
        if (jsonNode.kind != JsonValue::Kind::object)
            return false;

        const auto* position = member (jsonNode, "position");
        GraphNode node;
        node.id = stringMember (jsonNode, "id");
        node.type = stringMember (jsonNode, "type");
        node.systemUniforms = stringArrayMember (jsonNode, "systemUniforms");
        node.position = { position == nullptr ? 0.0 : numberMember (*position, "x", 0.0),
                          position == nullptr ? 0.0 : numberMember (*position, "y", 0.0) };
        node.collapsed = boolMember (jsonNode, "collapsed", false);

        if (node.id.empty() || node.type.empty())
            return false;

        if (! appendParsedParams (node, jsonNode) || ! appendParsedPortBindings (node, jsonNode))
            return false;

        nodes.push_back (node);
    }

    return true;
}

bool appendParsedEdges (std::vector<GraphEdge>& edges, const JsonValue& graphSection)
{
    const auto* jsonEdges = member (graphSection, "edges");
    if (jsonEdges == nullptr)
        return true;

    if (jsonEdges->kind != JsonValue::Kind::array)
        return false;

    for (const auto& jsonEdge : jsonEdges->arrayValue)
    {
        if (jsonEdge.kind != JsonValue::Kind::object)
            return false;

        GraphEdge edge {
            stringMember (jsonEdge, "from"),
            stringMember (jsonEdge, "to"),
            stringMember (jsonEdge, "id"),
            stringMember (jsonEdge, "dataType"),
            stringMember (jsonEdge, "streamKind")
        };

        if (edge.id.empty())
            edge.id = "edge." + edge.from + "." + edge.to;

        if (edge.from.empty() || edge.to.empty())
            return false;

        edges.push_back (edge);
    }

    return true;
}
}

void appendGraphSectionJson (std::ostringstream& out,
                             const std::vector<GraphNode>& nodes,
                             const std::vector<GraphEdge>& edges)
{
    out << "{\n";
    out << "    \"nodes\": [\n";

    for (size_t index = 0; index < nodes.size(); ++index)
    {
        out << "      ";
        appendGraphNodeJson (out, nodes[index]);

        if (index + 1 < nodes.size())
            out << ",";

        out << "\n";
    }

    out << "    ],\n";
    out << "    \"edges\": [\n";

    for (size_t index = 0; index < edges.size(); ++index)
    {
        out << "      ";
        appendGraphEdgeJson (out, edges[index]);

        if (index + 1 < edges.size())
            out << ",";

        out << "\n";
    }

    out << "    ]\n";
    out << "  }";
}

bool parseEditorGraph (EditorGraph& graph, const JsonValue& graphSection)
{
    return graphSection.kind == JsonValue::Kind::object
           && appendParsedNodes (graph.nodes, graphSection)
           && appendParsedEdges (graph.edges, graphSection);
}

bool parseRuntimeGraph (RuntimeGraph& graph, const JsonValue& graphSection)
{
    return graphSection.kind == JsonValue::Kind::object
           && appendParsedNodes (graph.nodes, graphSection)
           && appendParsedEdges (graph.edges, graphSection);
}

std::string readTextFile (const std::string& path, std::string& error)
{
    std::ifstream input (path);
    if (! input)
    {
        error = "could not open file: " + path;
        return {};
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::filesystem::path resolvePathNearFile (const std::string& filePath, const std::string& relativeOrAbsolutePath)
{
    const std::filesystem::path targetPath (relativeOrAbsolutePath);
    if (targetPath.is_absolute())
        return targetPath;

    return std::filesystem::path (filePath).parent_path() / targetPath;
}
}
