#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace myworld
{
struct ShaderPreviewUniformInput
{
    std::string bindingId;
    std::string uniformName;
    double value = 0.0;
};

struct ShaderPreviewInputSnapshot
{
    bool ok = false;
    std::string status;
    std::string message;
    std::uint64_t sampleCounter = 0;
    std::vector<ShaderPreviewUniformInput> uniforms;
    std::vector<std::string> errors;
};

ShaderPreviewInputSnapshot makeShaderPreviewInputFromUniformEvidence (
    const std::string& bindingId,
    const std::string& uniformName,
    double value,
    std::uint64_t sampleCounter);

std::string makeShaderPreviewInputSnapshotJson (const ShaderPreviewInputSnapshot& snapshot);
}
