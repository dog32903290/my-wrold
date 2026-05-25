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
            { { "source", "Source", "text.glsl", "", "", "Shader", "Editable fragment shader source.", true } }
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
            { { "channels", "Channels", "audio.channels", "out" } },
            { { "device", "Device", "string", "system", "", "Device", "Audio input device name.", true } }
        },
        {
            "audio.mono_mix",
            "Mono Mix",
            "audio",
            "modify",
            "audioAnalysis",
            "meter_scope",
            "docs/nodes/audio.mono_mix.md",
            1,
            { { "input", "Input", "audio.channels", "in" } },
            { { "mono", "Mono", "audio.mono", "out" } },
            {}
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
            {
                { "rms", "RMS", "signal.float", "out" },
                { "peak", "Peak", "signal.float", "out" }
            },
            {}
        },
        {
            "analyzer.analysis_gain",
            "Analysis Gain",
            "analyzer",
            "calibration",
            "audioAnalysis",
            "meter_scope",
            "docs/nodes/analyzer.analysis_gain.md",
            1,
            { { "input", "Input", "signal.float", "in" } },
            { { "out", "Out", "signal.float", "out" } },
            { { "gain", "Gain", "float", "1.0", "0.0..8.0", "Calibration", "Scales measured signal before later analysis." } }
        },
        {
            "analyzer.pre_gate",
            "Pre Gate",
            "analyzer",
            "gate",
            "audioAnalysis",
            "meter_scope",
            "docs/nodes/analyzer.pre_gate.md",
            1,
            { { "input", "Input", "signal.float", "in" } },
            {
                { "out", "Out", "signal.float", "out" },
                { "gate", "Gate", "signal.float", "out" },
                { "confidence", "Confidence", "signal.float", "out" }
            },
            {
                { "threshold", "Threshold", "float", "0.0001", "0.0..1.0", "Gate", "Minimum signal before gate opens." },
                { "hysteresis", "Hysteresis", "float", "0.01", "0.0..0.2", "Gate", "Offset that keeps the gate from flickering." }
            }
        },
        {
            "signal.smoother",
            "Smoother",
            "signal",
            "shaping",
            "audioAnalysis",
            "meter_scope",
            "docs/nodes/signal.smoother.md",
            1,
            { { "input", "Input", "signal.float", "in" } },
            { { "out", "Out", "signal.float", "out" } },
            { { "smooth", "Smooth", "float", "0.2", "0.0..1.0", "Smoothing", "Amount of temporal smoothing applied to the signal." } }
        },
        {
            "analyzer.loudness_out",
            "Loudness Out",
            "analyzer",
            "output",
            "audioAnalysis",
            "meter_scope",
            "docs/nodes/analyzer.loudness_out.md",
            1,
            {
                { "input", "Input", "signal.float", "in" },
                { "rms", "RMS", "signal.float", "in" },
                { "peak", "Peak", "signal.float", "in" },
                { "gate", "Gate", "signal.float", "in" },
                { "confidence", "Confidence", "signal.float", "in" }
            },
            {
                { "out", "Loudness", "signal.float", "out" },
                { "rms", "RMS", "signal.float", "out" },
                { "peak", "Peak", "signal.float", "out" },
                { "gate", "Gate", "signal.float", "out" },
                { "confidence", "Confidence", "signal.float", "out" }
            },
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
                { "curve", "Curve", "float", "1.0", "0.25..4.0", "Shaping", "Response curve applied to measured loudness." },
                { "smooth", "Smooth", "float", "0.2", "0.0..1.0", "Shaping", "Amount of temporal smoothing applied to loudness." }
            }
        },
        {
            "compound.loudness",
            "Loudness Compound",
            "compound",
            "feature",
            "audioAnalysis",
            "meter_scope",
            "docs/nodes/compound.loudness.md",
            1,
            { { "audio.in", "Audio In", "audio.channels", "in" } },
            {
                { "out", "Loudness", "signal.float", "out" },
                { "rms", "RMS", "signal.float", "out" },
                { "peak", "Peak", "signal.float", "out" },
                { "gate", "Gate", "signal.float", "out" },
                { "confidence", "Confidence", "signal.float", "out" }
            },
            {}
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
                { "channel", "Channel", "int", "1", "1..16", "MIDI", "MIDI channel used for outgoing control change messages." },
                { "cc", "CC", "int", "1", "0..127", "MIDI", "Control change number written by this node." }
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
    static constexpr std::array<const char*, 19> categories {
        "image", "render", "mesh", "point", "numbers", "io",
        "field", "flow", "particle", "string", "data", "assets",
        "shader", "material", "audio", "analyzer", "output", "compound", "signal"
    };

    return std::find (categories.begin(), categories.end(), category) != categories.end();
}

bool isKnownNodeSubcategory (const std::string& subcategory)
{
    static constexpr std::array<const char*, 25> subcategories {
        "generate", "modify", "draw", "color", "analyze", "transform",
        "camera", "postfx", "shading", "scene", "input", "output",
        "midi", "osc", "audio", "file", "context", "feature",
        "detector", "aggregate", "use", "measurement", "calibration",
        "gate", "shaping"
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
