#include "GraphContract.h"

#include "JsonWriter.h"

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
        { "u_time", "u_resolution", "u_frame", "u_loudness" },
        { 80.0, 80.0 },
        false
    };
}

GraphNode makeOutputNode()
{
    return {
        "out1",
        "output.preview",
        {},
        { 320.0, 80.0 },
        false
    };
}

GraphEdge makeDefaultEdge()
{
    return {
        "shader1.output",
        "out1.input",
        "edge.shader1.output.out1.input",
        "texture.rgba",
        "continuous"
    };
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
    auto edge = makeDefaultEdge();

    return {
        1,
        { { shader, output }, { edge } },
        {
            { shader, output },
            { edge }
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
uniform float u_loudness;

void main()
{
    vec2 uv = v_uv;
    vec2 centered = uv * 2.0 - 1.0;
    centered.x *= u_resolution.x / max(u_resolution.y, 1.0);

    float ring = 0.5 + 0.5 * cos(16.0 * length(centered) - u_time * 2.4);
    vec3 base = 0.5 + 0.5 * cos(u_time + vec3(0.0, 2.1, 4.2) + uv.xyx * 4.0);
    float pulse = 1.0 + clamp(u_loudness, 0.0, 1.0) * 0.35;
    vec3 color = mix(vec3(0.02, 0.025, 0.035), base * pulse, ring);

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

        out << jsonQuoted (graph.runtimeGraph.nodes[index].id);
    }

    out << "],\n";
    out << "  \"edges\": [\n";

    for (size_t index = 0; index < graph.runtimeGraph.edges.size(); ++index)
    {
        const auto& edge = graph.runtimeGraph.edges[index];
        out << "    { \"from\": " << jsonQuoted (edge.from)
            << ", \"to\": " << jsonQuoted (edge.to) << " }";

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
    out << "  \"renderer\": " << jsonQuoted (renderer) << ",\n";
    out << "  \"shaderStatus\": " << jsonQuoted (shaderStatus) << ",\n";
    out << "  \"nodes\": [\n";

    for (size_t index = 0; index < graph.runtimeGraph.nodes.size(); ++index)
    {
        const auto& node = graph.runtimeGraph.nodes[index];
        out << "    {\n";
        out << "      \"id\": " << jsonQuoted (node.id) << ",\n";
        out << "      \"type\": " << jsonQuoted (node.type) << ",\n";
        out << "      \"cookDomain\": \"" << cookDomainFor (node) << "\",\n";
        out << "      \"status\": \"ok\",\n";
        out << "      \"systemUniforms\": ";
        appendJsonStringArray (out, node.systemUniforms);
        out << ",\n";
        out << "      \"outputs\": ";
        appendJsonStringArray (out, outputPortsFor (graph.runtimeGraph, node));
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
