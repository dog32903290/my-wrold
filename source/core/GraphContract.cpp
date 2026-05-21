#include "GraphContract.h"

#include <algorithm>

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
}
