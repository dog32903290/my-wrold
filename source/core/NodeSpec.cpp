#include "NodeSpec.h"

#include <algorithm>
#include <array>

namespace myworld
{
std::vector<NodeSpec> makeSeedNodeSpecs()
{
    return {
        {
            "shader.fragment",
            "Fragment Shader",
            "shader",
            "use",
            "render",
            "tiny_preview",
            "docs/nodes/shader.fragment.md",
            1,
            {},
            { { "output", "Output", "texture.rgba", "out" } },
            { { "source", "Source", "text.glsl", "", "" } }
        },
        {
            "output.preview",
            "Preview Output",
            "output",
            "output",
            "render",
            "selected_preview",
            "docs/nodes/output.preview.md",
            1,
            { { "input", "Input", "texture.rgba", "in" } },
            {},
            {}
        },
        {
            "audio.input",
            "Audio Input",
            "audio",
            "input",
            "audio",
            "meter_scope",
            "docs/nodes/audio.input.md",
            1,
            {},
            { { "mono", "Mono", "audio.mono", "out" } },
            { { "analysisGain", "Analysis Gain", "float", "1.0", "0.0..4.0" } }
        },
        {
            "analyzer.rms",
            "RMS",
            "analyzer",
            "measurement",
            "audioAnalysis",
            "meter_scope",
            "docs/nodes/analyzer.rms.md",
            1,
            { { "input", "Input", "audio.mono", "in" } },
            { { "rms", "RMS", "signal.float", "out" } },
            {}
        },
        {
            "analyzer.loudness",
            "Loudness",
            "analyzer",
            "feature",
            "audioAnalysis",
            "meter_scope",
            "docs/nodes/analyzer.loudness.md",
            1,
            { { "input", "Input", "audio.mono", "in" } },
            { { "out", "Loudness", "signal.float", "out" } },
            {
                { "curve", "Curve", "float", "1.0", "0.25..4.0" },
                { "smooth", "Smooth", "float", "0.2", "0.0..1.0" }
            }
        },
        {
            "io.midi.cc_out",
            "MIDI CC Out",
            "io",
            "midi",
            "control",
            "none",
            "docs/nodes/io.midi.cc_out.md",
            1,
            { { "value", "Value", "signal.float", "in" } },
            {},
            {
                { "channel", "Channel", "int", "1", "1..16" },
                { "cc", "CC", "int", "1", "0..127" }
            }
        },
        {
            "image.texture",
            "Texture",
            "image",
            "use",
            "render",
            "tiny_preview",
            "docs/nodes/image.texture.md",
            1,
            {},
            { { "output", "Output", "texture.rgba", "out" } },
            {}
        },
        {
            "mesh.plane",
            "Plane",
            "mesh",
            "generate",
            "geometry",
            "selected_preview",
            "docs/nodes/mesh.plane.md",
            1,
            {},
            { { "geometry", "Geometry", "geometry.mesh", "out" } },
            {}
        },
        {
            "material.shader",
            "Shader Material",
            "material",
            "shading",
            "render",
            "tiny_preview",
            "docs/nodes/material.shader.md",
            1,
            { { "shader", "Shader", "texture.rgba", "in" } },
            { { "material", "Material", "material.shader", "out" } },
            {}
        }
    };
}

const NodeSpec* findNodeSpec (const std::vector<NodeSpec>& specs, const std::string& type)
{
    const auto found = std::find_if (specs.begin(), specs.end(), [&type] (const NodeSpec& spec)
    {
        return spec.type == type;
    });

    return found == specs.end() ? nullptr : &*found;
}

bool isKnownNodeCategory (const std::string& category)
{
    static constexpr std::array<const char*, 18> categories {
        "image", "render", "mesh", "point", "numbers", "io",
        "field", "flow", "particle", "string", "data", "assets",
        "shader", "material", "audio", "analyzer", "output", "compound"
    };

    return std::find (categories.begin(), categories.end(), category) != categories.end();
}

bool isKnownNodeSubcategory (const std::string& subcategory)
{
    static constexpr std::array<const char*, 22> subcategories {
        "generate", "modify", "draw", "color", "analyze", "transform",
        "camera", "postfx", "shading", "scene", "input", "output",
        "midi", "osc", "audio", "file", "context", "feature",
        "detector", "aggregate", "use", "measurement"
    };

    return std::find (subcategories.begin(), subcategories.end(), subcategory) != subcategories.end();
}

bool isKnownNodeCategoryAlias (const std::string& alias)
{
    static constexpr std::array<const char*, 7> aliases {
        "geometry", "signal", "midi", "texture", "top", "sop", "mat"
    };

    return std::find (aliases.begin(), aliases.end(), alias) != aliases.end();
}

bool isKnownPreviewPolicy (const std::string& policy)
{
    static constexpr std::array<const char*, 4> policies {
        "none", "tiny_preview", "meter_scope", "selected_preview"
    };

    return std::find (policies.begin(), policies.end(), policy) != policies.end();
}
}
