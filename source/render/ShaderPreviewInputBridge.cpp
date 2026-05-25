#include "ShaderPreviewInputBridge.h"

#include <iomanip>
#include <sstream>

namespace myworld
{
namespace
{
std::string jsonQuoted (const std::string& text)
{
    std::ostringstream out;
    out << '"';

    for (const auto character : text)
    {
        switch (character)
        {
            case '\\': out << "\\\\"; break;
            case '"':  out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:   out << character; break;
        }
    }

    out << '"';
    return out.str();
}

void appendErrorsJson (std::ostringstream& out, const std::vector<std::string>& errors)
{
    out << "[";

    for (size_t index = 0; index < errors.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << jsonQuoted (errors[index]);
    }

    out << "]";
}
}

ShaderPreviewInputSnapshot makeShaderPreviewInputFromUniformEvidence (
    const std::string& bindingId,
    const std::string& uniformName,
    double value,
    std::uint64_t sampleCounter)
{
    ShaderPreviewInputSnapshot snapshot;
    snapshot.sampleCounter = sampleCounter;

    if (bindingId.empty())
    {
        snapshot.status = "failed";
        snapshot.message = "shader preview uniform binding id is required";
        snapshot.errors.push_back (snapshot.message);
        return snapshot;
    }

    if (uniformName.empty())
    {
        snapshot.status = "failed";
        snapshot.message = "shader preview uniform name is required";
        snapshot.errors.push_back (snapshot.message);
        return snapshot;
    }

    snapshot.ok = true;
    snapshot.status = "ready";
    snapshot.message = "shader_preview_input_ready";
    snapshot.uniforms.push_back ({ bindingId, uniformName, value });
    return snapshot;
}

std::string makeShaderPreviewInputSnapshotJson (const ShaderPreviewInputSnapshot& snapshot)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"shaderPreviewInputSnapshot\",\n";
    out << "  \"ok\": " << (snapshot.ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (snapshot.status) << ",\n";
    out << "  \"message\": " << jsonQuoted (snapshot.message) << ",\n";
    out << "  \"sampleCounter\": " << snapshot.sampleCounter << ",\n";
    out << "  \"uniforms\": [\n";

    for (size_t index = 0; index < snapshot.uniforms.size(); ++index)
    {
        const auto& uniform = snapshot.uniforms[index];
        out << "    {\n";
        out << "      \"bindingId\": " << jsonQuoted (uniform.bindingId) << ",\n";
        out << "      \"uniformName\": " << jsonQuoted (uniform.uniformName) << ",\n";
        out << "      \"value\": " << uniform.value << "\n";
        out << "    }";

        if (index + 1 < snapshot.uniforms.size())
            out << ",";

        out << "\n";
    }

    out << "  ],\n";
    out << "  \"errors\": ";
    appendErrorsJson (out, snapshot.errors);
    out << "\n";
    out << "}\n";
    return out.str();
}

RenderFrameInput makeRenderFrameInputFromShaderPreviewInput (
    const ShaderPreviewInputSnapshot& snapshot,
    double timeSeconds,
    unsigned int frameIndex,
    float fallbackLoudness)
{
    RenderFrameInput input;
    input.timeSeconds = timeSeconds;
    input.frameIndex = frameIndex;
    input.loudness = fallbackLoudness;

    if (! snapshot.ok)
        return input;

    for (const auto& uniform : snapshot.uniforms)
    {
        if (uniform.uniformName == "u_loudness")
        {
            input.loudness = static_cast<float> (uniform.value);
            break;
        }
    }

    return input;
}
}
