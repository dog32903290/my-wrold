#pragma once

#include <string>

namespace myworld
{
struct ShaderCompileResult
{
    bool ok = false;
    std::string message;
};

class RenderBackend
{
public:
    virtual ~RenderBackend() = default;

    virtual ShaderCompileResult compileShader (const std::string& fragmentSource) = 0;
    virtual void resize (int width, int height, float scale) = 0;
    virtual void renderFrame (double timeSeconds, unsigned int frameIndex) = 0;
};
}
