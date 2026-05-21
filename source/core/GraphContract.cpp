#include "GraphContract.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace myworld
{
namespace
{
GraphNode makeShaderNode()
{
    return {
        "shader1",
        "shader.fragment",
        { "u_time", "u_resolution", "u_frame" }
    };
}

GraphNode makeOutputNode()
{
    return {
        "out1",
        "output.preview",
        {}
    };
}

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
            default:
                if (static_cast<unsigned char> (character) < 0x20)
                    out << "\\u" << std::hex << std::setw (4) << std::setfill ('0')
                        << static_cast<int> (static_cast<unsigned char> (character));
                else
                    out << character;
                break;
        }
    }

    return out.str();
}

void appendStringArray (std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";

    for (size_t index = 0; index < values.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << "\"" << jsonEscaped (values[index]) << "\"";
    }

    out << "]";
}

std::string cookDomainFor (const GraphNode& node)
{
    if (node.type.rfind ("shader.", 0) == 0 || node.type.rfind ("output.", 0) == 0)
        return "render";

    return "unknown";
}

std::vector<std::string> outputPortsFor (const RuntimeGraph& runtimeGraph, const GraphNode& node)
{
    std::vector<std::string> outputs;

    for (const auto& edge : runtimeGraph.edges)
    {
        const auto prefix = node.id + ".";

        if (edge.from.rfind (prefix, 0) == 0
            && std::find (outputs.begin(), outputs.end(), edge.from) == outputs.end())
            outputs.push_back (edge.from);
    }

    return outputs;
}
}

GraphContract makeDefaultShaderOutputGraph()
{
    auto shader = makeShaderNode();
    auto output = makeOutputNode();

    return {
        1,
        { { shader, output } },
        {
            { shader, output },
            { { "shader1.output", "out1.input" } }
        }
    };
}

bool usesSystemUniform (const GraphNode& node, const std::string& uniformName)
{
    return std::find (node.systemUniforms.begin(),
                      node.systemUniforms.end(),
                      uniformName) != node.systemUniforms.end();
}

std::string defaultFragmentShader()
{
    return R"(varying vec2 v_uv;
uniform float u_time;
uniform vec2 u_resolution;
uniform float u_frame;

void main()
{
    vec2 uv = v_uv;
    vec2 centered = uv * 2.0 - 1.0;
    centered.x *= u_resolution.x / max(u_resolution.y, 1.0);

    float ring = 0.5 + 0.5 * cos(16.0 * length(centered) - u_time * 2.4);
    vec3 base = 0.5 + 0.5 * cos(u_time + vec3(0.0, 2.1, 4.2) + uv.xyx * 4.0);
    vec3 color = mix(vec3(0.02, 0.025, 0.035), base, ring);

    gl_FragColor = vec4(color, 1.0);
}
)";
}

std::string makeCookOrderJson (const GraphContract& graph)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": " << graph.version << ",\n";
    out << "  \"cookOrder\": [";

    for (size_t index = 0; index < graph.runtimeGraph.nodes.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << "\"" << jsonEscaped (graph.runtimeGraph.nodes[index].id) << "\"";
    }

    out << "],\n";
    out << "  \"edges\": [\n";

    for (size_t index = 0; index < graph.runtimeGraph.edges.size(); ++index)
    {
        const auto& edge = graph.runtimeGraph.edges[index];
        out << "    { \"from\": \"" << jsonEscaped (edge.from)
            << "\", \"to\": \"" << jsonEscaped (edge.to) << "\" }";

        if (index + 1 < graph.runtimeGraph.edges.size())
            out << ",";

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    return out.str();
}

std::string makeNodeStatsJson (const GraphContract& graph,
                               int viewportWidth,
                               int viewportHeight,
                               unsigned int frameIndex,
                               double timeSeconds,
                               const std::string& renderer,
                               const std::string& shaderStatus)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"version\": " << graph.version << ",\n";
    out << "  \"frameIndex\": " << frameIndex << ",\n";
    out << "  \"timeSeconds\": " << timeSeconds << ",\n";
    out << "  \"viewport\": { \"width\": " << viewportWidth
        << ", \"height\": " << viewportHeight << " },\n";
    out << "  \"renderer\": \"" << jsonEscaped (renderer) << "\",\n";
    out << "  \"shaderStatus\": \"" << jsonEscaped (shaderStatus) << "\",\n";
    out << "  \"nodes\": [\n";

    for (size_t index = 0; index < graph.runtimeGraph.nodes.size(); ++index)
    {
        const auto& node = graph.runtimeGraph.nodes[index];
        out << "    {\n";
        out << "      \"id\": \"" << jsonEscaped (node.id) << "\",\n";
        out << "      \"type\": \"" << jsonEscaped (node.type) << "\",\n";
        out << "      \"cookDomain\": \"" << cookDomainFor (node) << "\",\n";
        out << "      \"status\": \"ok\",\n";
        out << "      \"systemUniforms\": ";
        appendStringArray (out, node.systemUniforms);
        out << ",\n";
        out << "      \"outputs\": ";
        appendStringArray (out, outputPortsFor (graph.runtimeGraph, node));
        out << "\n";
        out << "    }";

        if (index + 1 < graph.runtimeGraph.nodes.size())
            out << ",";

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    return out.str();
}
}
