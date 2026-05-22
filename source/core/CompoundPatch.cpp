#include "CompoundPatch.h"

#include <algorithm>
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

bool hasChild (const CompoundPatchSpec& spec, const std::string& childId)
{
    return findCompoundChild (spec, childId) != nullptr;
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

    for (const auto& output : spec.publicOutputs)
        if (! hasChild (spec, portOwner (output.mapsTo)))
            return false;

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

    out << "  \"publicOutputs\": [\n";
    for (size_t index = 0; index < spec.publicOutputs.size(); ++index)
    {
        const auto& port = spec.publicOutputs[index];
        out << "    { \"id\": \"" << jsonEscaped (port.id)
            << "\", \"dataType\": \"" << jsonEscaped (port.dataType)
            << "\", \"mapsTo\": \"" << jsonEscaped (port.mapsTo) << "\" }";

        if (index + 1 < spec.publicOutputs.size())
            out << ",";

        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}
}
