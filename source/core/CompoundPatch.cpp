#include "CompoundPatch.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <set>
#include <sstream>

namespace myworld
{
namespace
{
std::string jsonEscaped (const std::string& text)
{
    std::ostringstream out;

    for (const auto character : text)
    {
        switch (character)
        {
            case '"':  out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:   out << character; break;
        }
    }

    return out.str();
}

std::string portOwner (const std::string& portPath)
{
    const auto dot = portPath.find ('.');
    return dot == std::string::npos ? portPath : portPath.substr (0, dot);
}

std::string portName (const std::string& portPath)
{
    const auto dot = portPath.find ('.');
    return dot == std::string::npos ? std::string {} : portPath.substr (dot + 1);
}

bool hasChild (const CompoundPatchSpec& spec, const std::string& childId)
{
    return findCompoundChild (spec, childId) != nullptr;
}

std::string qualifiedEndpoint (const std::string& parentNodeId, const std::string& endpoint)
{
    return compoundPatchChildNodeId (parentNodeId, portOwner (endpoint)) + "." + portName (endpoint);
}

std::string edgeIdFor (const std::string& from, const std::string& to)
{
    return "edge." + from + "." + to;
}

struct JsonValue
{
    enum class Kind
    {
        nullValue,
        string,
        boolean,
        array,
        object
    };

    Kind kind = Kind::nullValue;
    std::string stringValue;
    bool boolValue = false;
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

bool appendChildren (CompoundPatchSpec& spec, const JsonValue& root)
{
    const auto* children = member (root, "children");
    if (children == nullptr || children->kind != JsonValue::Kind::array)
        return false;

    for (const auto& child : children->arrayValue)
    {
        if (child.kind != JsonValue::Kind::object)
            return false;

        spec.children.push_back ({ stringMember (child, "id"),
                                   stringMember (child, "type"),
                                   stringMember (child, "role") });
    }

    return true;
}

bool appendInternalEdges (CompoundPatchSpec& spec, const JsonValue& root)
{
    const auto* edges = member (root, "internalEdges");
    if (edges == nullptr || edges->kind != JsonValue::Kind::array)
        return false;

    for (const auto& edge : edges->arrayValue)
    {
        if (edge.kind != JsonValue::Kind::object)
            return false;

        spec.internalEdges.push_back ({ stringMember (edge, "from"),
                                        stringMember (edge, "to"),
                                        stringMember (edge, "dataType") });
    }

    return true;
}

bool appendPublicPorts (std::vector<CompoundPublicPort>& ports, const JsonValue& root, const std::string& arrayName)
{
    const auto* array = member (root, arrayName);
    if (array == nullptr || array->kind != JsonValue::Kind::array)
        return false;

    for (const auto& port : array->arrayValue)
    {
        if (port.kind != JsonValue::Kind::object)
            return false;

        ports.push_back ({ stringMember (port, "id"),
                           stringMember (port, "label"),
                           stringMember (port, "dataType"),
                           stringMember (port, "direction"),
                           stringMember (port, "mapsTo") });
    }

    return true;
}
}

CompoundPatchSpec makeLoudnessCompoundPatchSpec()
{
    return {
        "compound.loudness",
        "Loudness",
        true,
        {
            { "audio_in", "audio.input", "Raw audio input adapter" },
            { "mono_mix", "audio.mono_mix", "Fold channels into one mono analysis lane" },
            { "rms", "analyzer.rms", "Measure raw RMS and peak facts" },
            { "analysis_gain", "analyzer.analysis_gain", "Calibrate measured energy" },
            { "pre_gate", "analyzer.pre_gate", "Reject untrusted near-silence" },
            { "output_smoother", "signal.smoother", "Shape output motion for live use" },
            { "loudness_out", "analyzer.loudness_out", "Publish public loudness ports" }
        },
        {
            { "audio.in", "audio_in.input", "audio.channels" },
            { "audio_in.channels", "mono_mix.input", "audio.channels" },
            { "mono_mix.mono", "rms.input", "audio.mono" },
            { "rms.rms", "analysis_gain.input", "signal.float" },
            { "analysis_gain.out", "pre_gate.input", "signal.float" },
            { "pre_gate.out", "output_smoother.input", "signal.float" },
            { "output_smoother.out", "loudness_out.input", "signal.float" },
            { "rms.peak", "loudness_out.peak", "signal.float" },
            { "pre_gate.confidence", "loudness_out.confidence", "signal.float" }
        },
        {
            { "audio.in", "Audio In", "audio.channels", "in", "audio_in.input" }
        },
        {
            { "out", "Loudness", "signal.float", "out", "loudness_out.out" },
            { "rms", "RMS", "signal.float", "out", "rms.rms" },
            { "peak", "Peak", "signal.float", "out", "rms.peak" },
            { "gate", "Gate", "signal.float", "out", "pre_gate.gate" },
            { "confidence", "Confidence", "signal.float", "out", "pre_gate.confidence" }
        }
    };
}

const CompoundChildNode* findCompoundChild (const CompoundPatchSpec& spec, const std::string& childId)
{
    const auto found = std::find_if (spec.children.begin(), spec.children.end(), [&childId] (const auto& child)
    {
        return child.id == childId;
    });

    return found == spec.children.end() ? nullptr : &*found;
}

const CompoundPublicPort* findCompoundPublicOutput (const CompoundPatchSpec& spec, const std::string& outputId)
{
    const auto found = std::find_if (spec.publicOutputs.begin(), spec.publicOutputs.end(), [&outputId] (const auto& port)
    {
        return port.id == outputId;
    });

    return found == spec.publicOutputs.end() ? nullptr : &*found;
}

std::vector<std::string> makeCompoundCookOrder (const CompoundPatchSpec& spec)
{
    std::vector<std::string> cookOrder;
    cookOrder.reserve (spec.children.size());

    for (const auto& child : spec.children)
        cookOrder.push_back (child.id);

    return cookOrder;
}

bool isValidCompoundPatchSpec (const CompoundPatchSpec& spec)
{
    if (spec.type.empty() || spec.children.empty() || spec.publicOutputs.empty())
        return false;

    std::set<std::string> childIds;

    for (const auto& child : spec.children)
    {
        if (child.id.empty() || child.nodeType.empty())
            return false;

        if (! childIds.insert (child.id).second)
            return false;
    }

    for (const auto& edge : spec.internalEdges)
    {
        if (edge.from.empty() || edge.to.empty() || edge.dataType.empty())
            return false;

        const auto fromOwner = portOwner (edge.from);
        const auto toOwner = portOwner (edge.to);
        const auto fromIsPublicInput = std::find_if (spec.publicInputs.begin(), spec.publicInputs.end(), [&edge] (const auto& port)
        {
            return port.id == edge.from;
        }) != spec.publicInputs.end();

        if (! fromIsPublicInput && ! hasChild (spec, fromOwner))
            return false;

        if (! hasChild (spec, toOwner))
            return false;
    }

    for (const auto& input : spec.publicInputs)
    {
        if (input.id.empty() || input.label.empty() || input.dataType.empty() || input.direction != "in" || input.mapsTo.empty())
            return false;

        if (! hasChild (spec, portOwner (input.mapsTo)))
            return false;
    }

    for (const auto& output : spec.publicOutputs)
    {
        if (output.id.empty() || output.label.empty() || output.dataType.empty() || output.direction != "out" || output.mapsTo.empty())
            return false;

        if (! hasChild (spec, portOwner (output.mapsTo)))
            return false;
    }

    return true;
}

std::string makeCompoundPatchJson (const CompoundPatchSpec& spec)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"type\": \"" << jsonEscaped (spec.type) << "\",\n";
    out << "  \"displayName\": \"" << jsonEscaped (spec.displayName) << "\",\n";
    out << "  \"collapsedByDefault\": " << (spec.collapsedByDefault ? "true" : "false") << ",\n";

    out << "  \"children\": [\n";
    for (size_t index = 0; index < spec.children.size(); ++index)
    {
        const auto& child = spec.children[index];
        out << "    { \"id\": \"" << jsonEscaped (child.id)
            << "\", \"type\": \"" << jsonEscaped (child.nodeType)
            << "\", \"role\": \"" << jsonEscaped (child.role) << "\" }";

        if (index + 1 < spec.children.size())
            out << ",";

        out << "\n";
    }
    out << "  ],\n";

    out << "  \"internalEdges\": [\n";
    for (size_t index = 0; index < spec.internalEdges.size(); ++index)
    {
        const auto& edge = spec.internalEdges[index];
        out << "    { \"from\": \"" << jsonEscaped (edge.from)
            << "\", \"to\": \"" << jsonEscaped (edge.to)
            << "\", \"dataType\": \"" << jsonEscaped (edge.dataType) << "\" }";

        if (index + 1 < spec.internalEdges.size())
            out << ",";

        out << "\n";
    }
    out << "  ],\n";

    out << "  \"publicInputs\": [\n";
    for (size_t index = 0; index < spec.publicInputs.size(); ++index)
    {
        const auto& port = spec.publicInputs[index];
        out << "    { \"id\": \"" << jsonEscaped (port.id)
            << "\", \"label\": \"" << jsonEscaped (port.label)
            << "\", \"dataType\": \"" << jsonEscaped (port.dataType)
            << "\", \"direction\": \"" << jsonEscaped (port.direction)
            << "\", \"mapsTo\": \"" << jsonEscaped (port.mapsTo) << "\" }";

        if (index + 1 < spec.publicInputs.size())
            out << ",";

        out << "\n";
    }
    out << "  ],\n";

    out << "  \"publicOutputs\": [\n";
    for (size_t index = 0; index < spec.publicOutputs.size(); ++index)
    {
        const auto& port = spec.publicOutputs[index];
        out << "    { \"id\": \"" << jsonEscaped (port.id)
            << "\", \"label\": \"" << jsonEscaped (port.label)
            << "\", \"dataType\": \"" << jsonEscaped (port.dataType)
            << "\", \"direction\": \"" << jsonEscaped (port.direction)
            << "\", \"mapsTo\": \"" << jsonEscaped (port.mapsTo) << "\" }";

        if (index + 1 < spec.publicOutputs.size())
            out << ",";

        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

CompoundPatchLoadResult parseCompoundPatchJson (const std::string& text)
{
    JsonParser parser (text);
    const auto root = parser.parse();

    if (! parser.ok())
        return { false, {}, parser.error() };

    if (root.kind != JsonValue::Kind::object)
        return { false, {}, "compound json root must be an object" };

    CompoundPatchSpec spec;
    spec.type = stringMember (root, "type");
    spec.displayName = stringMember (root, "displayName");
    spec.collapsedByDefault = boolMember (root, "collapsedByDefault", true);

    if (! appendChildren (spec, root))
        return { false, {}, "invalid children array" };

    if (! appendInternalEdges (spec, root))
        return { false, {}, "invalid internalEdges array" };

    if (! appendPublicPorts (spec.publicInputs, root, "publicInputs"))
        return { false, {}, "invalid publicInputs array" };

    if (! appendPublicPorts (spec.publicOutputs, root, "publicOutputs"))
        return { false, {}, "invalid publicOutputs array" };

    if (! isValidCompoundPatchSpec (spec))
        return { false, {}, "compound patch failed validation" };

    return { true, spec, {} };
}

CompoundPatchLoadResult loadCompoundPatchSpec (const std::string& path)
{
    std::ifstream input (path);
    if (! input)
        return { false, {}, "could not open compound patch: " + path };

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return parseCompoundPatchJson (buffer.str());
}

std::string compoundPatchChildNodeId (const std::string& parentNodeId, const std::string& childId)
{
    return parentNodeId + "/" + childId;
}

GraphContract makeCompoundPatchInteractionGraph (const CompoundPatchSpec& spec, const std::string& parentNodeId)
{
    GraphContract graph;
    graph.version = 1;

    constexpr double startX = 100.0;
    constexpr double startY = 110.0;
    constexpr double columnWidth = 210.0;
    constexpr double rowHeight = 120.0;
    constexpr int columns = 3;

    for (size_t index = 0; index < spec.children.size(); ++index)
    {
        const auto& child = spec.children[index];
        graph.editorGraph.nodes.push_back ({ compoundPatchChildNodeId (parentNodeId, child.id),
                                             child.nodeType,
                                             {},
                                             { startX + static_cast<double> (index % columns) * columnWidth,
                                               startY + static_cast<double> (index / columns) * rowHeight },
                                             false });
    }

    for (const auto& edge : spec.internalEdges)
    {
        if (! hasChild (spec, portOwner (edge.from)) || ! hasChild (spec, portOwner (edge.to)))
            continue;

        const auto from = qualifiedEndpoint (parentNodeId, edge.from);
        const auto to = qualifiedEndpoint (parentNodeId, edge.to);
        graph.editorGraph.edges.push_back ({ from, to, edgeIdFor (from, to), edge.dataType, "continuous" });
    }

    graph.runtimeGraph.nodes = graph.editorGraph.nodes;
    graph.runtimeGraph.edges = graph.editorGraph.edges;
    return graph;
}
}
