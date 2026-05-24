#include "StorageContract.h"

#include "JsonWriter.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>

namespace myworld
{
namespace
{
size_t findMemberValue (const std::string& text, const std::string& key)
{
    const auto keyToken = "\"" + key + "\"";
    const auto keyPosition = text.find (keyToken);
    if (keyPosition == std::string::npos)
        return std::string::npos;

    const auto colon = text.find (':', keyPosition + keyToken.size());
    if (colon == std::string::npos)
        return std::string::npos;

    auto valuePosition = colon + 1;
    while (valuePosition < text.size() && std::isspace (static_cast<unsigned char> (text[valuePosition])) != 0)
        ++valuePosition;

    return valuePosition;
}

std::string parseJsonStringAt (const std::string& text, size_t position)
{
    if (position >= text.size() || text[position] != '"')
        return {};

    std::string result;
    ++position;

    while (position < text.size())
    {
        const auto current = text[position++];

        if (current == '"')
            return result;

        if (current == '\\')
        {
            if (position >= text.size())
                return {};

            const auto escaped = text[position++];
            switch (escaped)
            {
                case '"':  result.push_back ('"'); break;
                case '\\': result.push_back ('\\'); break;
                case 'n':  result.push_back ('\n'); break;
                case 'r':  result.push_back ('\r'); break;
                case 't':  result.push_back ('\t'); break;
                default:   result.push_back (escaped); break;
            }
        }
        else
        {
            result.push_back (current);
        }
    }

    return {};
}

std::string stringMember (const std::string& text, const std::string& key)
{
    const auto position = findMemberValue (text, key);
    return position == std::string::npos ? std::string {} : parseJsonStringAt (text, position);
}

std::vector<std::string> stringArrayMember (const std::string& text, const std::string& key)
{
    std::vector<std::string> result;
    auto position = findMemberValue (text, key);

    if (position == std::string::npos || position >= text.size() || text[position] != '[')
        return result;

    ++position;

    while (position < text.size())
    {
        while (position < text.size() && std::isspace (static_cast<unsigned char> (text[position])) != 0)
            ++position;

        if (position < text.size() && text[position] == ']')
            return result;

        const auto value = parseJsonStringAt (text, position);
        if (value.empty())
            return {};

        result.push_back (value);
        position = text.find_first_of (",]", position + 1);

        if (position == std::string::npos)
            return {};

        if (text[position] == ']')
            return result;

        ++position;
    }

    return {};
}

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
    explicit JsonParser (const std::string& sourceText)
        : source (sourceText)
    {
    }

    JsonValue parse()
    {
        auto value = parseValue();
        skipWhitespace();

        if (! failed && position != source.size())
            fail ("trailing characters");

        return value;
    }

    bool ok() const
    {
        return ! failed;
    }

    std::string error() const
    {
        return errorMessage;
    }

private:
    JsonValue parseValue()
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

    JsonValue parseString()
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

    JsonValue parseNumber()
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

    JsonValue parseObject()
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

    JsonValue parseArray()
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

    void skipWhitespace()
    {
        while (position < source.size() && std::isspace (static_cast<unsigned char> (source[position])) != 0)
            ++position;
    }

    bool consume (char expected)
    {
        skipWhitespace();

        if (position < source.size() && source[position] == expected)
        {
            ++position;
            return true;
        }

        return false;
    }

    JsonValue fail (const std::string& message)
    {
        failed = true;
        errorMessage = message;
        return {};
    }

    const std::string& source;
    size_t position = 0;
    bool failed = false;
    std::string errorMessage;
};

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

WorkProjectManifest makeMinimalWorkProject (const std::string& id, const std::string& title)
{
    return {
        id,
        title,
        "patches/main.patch.json",
        "commandS.localCommit",
        { "modules" },
        { "works" }
    };
}

PatchDocumentManifest makeMinimalPatchDocument (const std::string& id, const std::string& title)
{
    return { id, title, "editorGraph", "runtimeGraph", "portBindings" };
}

PatchDocument makePatchDocument (const std::string& id, const std::string& title, const GraphContract& graph)
{
    return { id, title, graph.version, graph };
}

ModulePackageManifest makeModulePackage (const std::string& id, const std::string& title, const std::string& patchPath)
{
    return {
        id,
        title,
        id,
        patchPath,
        "docs/manual.md",
        "compound",
        "feature",
        "graph",
        "none",
        { "in", "out" }
    };
}

ModuleLibraryManifest makeModuleLibrary (const std::string& id,
                                         const std::string& title,
                                         const std::vector<std::string>& modulePackages)
{
    return { id, title, modulePackages };
}

std::string toJson (const WorkProjectManifest& manifest)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"workProject\",\n";
    out << "  \"id\": " << jsonQuoted (manifest.id) << ",\n";
    out << "  \"title\": " << jsonQuoted (manifest.title) << ",\n";
    out << "  \"mainPatchPath\": " << jsonQuoted (manifest.mainPatchPath) << ",\n";
    out << "  \"savePolicy\": " << jsonQuoted (manifest.savePolicy) << ",\n";
    out << "  \"moduleLibraries\": ";
    appendJsonStringArray (out, manifest.moduleLibraries);
    out << ",\n";
    out << "  \"workLibraries\": ";
    appendJsonStringArray (out, manifest.workLibraries);
    out << "\n";
    out << "}\n";
    return out.str();
}

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
    out << "\n";
    out << "}\n";
    return out.str();
}

std::string toJson (const ModulePackageManifest& manifest)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"modulePackage\",\n";
    out << "  \"id\": " << jsonQuoted (manifest.id) << ",\n";
    out << "  \"title\": " << jsonQuoted (manifest.title) << ",\n";
    out << "  \"nodeType\": " << jsonQuoted (manifest.nodeType) << ",\n";
    out << "  \"patchPath\": " << jsonQuoted (manifest.patchPath) << ",\n";
    out << "  \"humanDocPath\": " << jsonQuoted (manifest.humanDocPath) << ",\n";
    out << "  \"category\": " << jsonQuoted (manifest.category) << ",\n";
    out << "  \"subcategory\": " << jsonQuoted (manifest.subcategory) << ",\n";
    out << "  \"runtimeDomain\": " << jsonQuoted (manifest.runtimeDomain) << ",\n";
    out << "  \"previewPolicy\": " << jsonQuoted (manifest.previewPolicy) << ",\n";
    out << "  \"publicPorts\": ";
    appendJsonStringArray (out, manifest.publicPorts);
    out << "\n";
    out << "}\n";
    return out.str();
}

std::string toJson (const ModuleLibraryManifest& manifest)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"moduleLibrary\",\n";
    out << "  \"id\": " << jsonQuoted (manifest.id) << ",\n";
    out << "  \"title\": " << jsonQuoted (manifest.title) << ",\n";
    out << "  \"modulePackages\": ";
    appendJsonStringArray (out, manifest.modulePackages);
    out << "\n";
    out << "}\n";
    return out.str();
}

WorkProjectLoadResult parseWorkProjectManifest (const std::string& text)
{
    JsonParser parser (text);
    const auto root = parser.parse();

    if (! parser.ok())
        return { false, {}, parser.error() };

    if (root.kind != JsonValue::Kind::object)
        return { false, {}, "work project root must be an object" };

    if (stringMember (root, "kind") != "workProject")
        return { false, {}, "work project kind must be workProject" };

    WorkProjectManifest manifest;
    manifest.id = stringMember (root, "id");
    manifest.title = stringMember (root, "title");
    manifest.mainPatchPath = stringMember (root, "mainPatchPath");
    manifest.savePolicy = stringMember (root, "savePolicy");
    manifest.moduleLibraries = stringArrayMember (root, "moduleLibraries");
    manifest.workLibraries = stringArrayMember (root, "workLibraries");

    if (manifest.id.empty() || manifest.title.empty() || manifest.mainPatchPath.empty())
        return { false, {}, "work project is missing required identity or main patch fields" };

    return { true, manifest, {} };
}

WorkProjectLoadResult loadWorkProjectManifest (const std::string& path)
{
    std::string error;
    const auto text = readTextFile (path, error);
    if (! error.empty())
        return { false, {}, error };

    return parseWorkProjectManifest (text);
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

    if (document.id.empty() || document.title.empty())
        return { false, {}, "patch document is missing required identity fields" };

    const auto* editorGraph = member (root, "editorGraph");
    if (editorGraph == nullptr || ! parseEditorGraph (document.graph.editorGraph, *editorGraph))
        return { false, {}, "patch document has invalid editorGraph" };

    const auto* runtimeGraph = member (root, "runtimeGraph");
    if (runtimeGraph == nullptr || ! parseRuntimeGraph (document.graph.runtimeGraph, *runtimeGraph))
        return { false, {}, "patch document has invalid runtimeGraph" };

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

ModulePackageLoadResult parseModulePackageManifest (const std::string& text)
{
    ModulePackageManifest manifest {
        stringMember (text, "id"),
        stringMember (text, "title"),
        stringMember (text, "nodeType"),
        stringMember (text, "patchPath"),
        stringMember (text, "humanDocPath"),
        stringMember (text, "category"),
        stringMember (text, "subcategory"),
        stringMember (text, "runtimeDomain"),
        stringMember (text, "previewPolicy"),
        stringArrayMember (text, "publicPorts")
    };

    if (stringMember (text, "kind") != "modulePackage")
        return { false, {}, "module manifest kind must be modulePackage" };

    if (manifest.id.empty() || manifest.title.empty() || manifest.nodeType.empty() || manifest.patchPath.empty())
        return { false, {}, "module manifest is missing required identity fields" };

    if (manifest.humanDocPath.empty() || manifest.category.empty() || manifest.subcategory.empty()
        || manifest.runtimeDomain.empty() || manifest.previewPolicy.empty())
    {
        return { false, {}, "module manifest is missing node spec fields" };
    }

    if (manifest.publicPorts.empty())
        return { false, {}, "module manifest must expose public ports" };

    return { true, manifest, {} };
}

ModuleLibraryLoadResult parseModuleLibraryManifest (const std::string& text)
{
    ModuleLibraryManifest manifest {
        stringMember (text, "id"),
        stringMember (text, "title"),
        stringArrayMember (text, "modulePackages")
    };

    if (stringMember (text, "kind") != "moduleLibrary")
        return { false, {}, "module library kind must be moduleLibrary" };

    if (manifest.id.empty() || manifest.title.empty())
        return { false, {}, "module library is missing required identity fields" };

    if (manifest.modulePackages.empty())
        return { false, {}, "module library must list module packages" };

    return { true, manifest, {} };
}

ModulePackageLoadResult loadModulePackageManifest (const std::string& path)
{
    std::ifstream input (path);
    if (! input)
        return { false, {}, "could not open module manifest: " + path };

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return parseModulePackageManifest (buffer.str());
}

ModuleLibraryLoadResult loadModuleLibraryManifest (const std::string& path)
{
    std::ifstream input (path);
    if (! input)
        return { false, {}, "could not open module library: " + path };

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return parseModuleLibraryManifest (buffer.str());
}

bool isKnownSaveStatus (const std::string& status)
{
    static constexpr std::array<const char*, 6> statuses {
        "clean",
        "saved-and-committed",
        "save-ok commit-pending",
        "save-ok commit-failed",
        "validation-failed",
        "write-failed"
    };

    return std::find (statuses.begin(), statuses.end(), status) != statuses.end();
}
}
