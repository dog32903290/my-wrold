#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct ShaderCompileResult
{
    bool ok = false;
    std::string message;
};

struct RenderFrameInput
{
    double timeSeconds = 0.0;
    unsigned int frameIndex = 0;
    float loudness = 0.0f;
};

struct CapturedFrame
{
    int width = 0;
    int height = 0;
    std::vector<unsigned char> rgba;
};

class RenderBackend
{
public:
    virtual ~RenderBackend() = default;

    virtual const char* backendName() const = 0;
    virtual std::string lastStatus() const = 0;
    virtual ShaderCompileResult compileShader (const std::string& fragmentSource) = 0;
    virtual void resize (int width, int height, float scale) = 0;
    virtual void renderFrame (const RenderFrameInput& input) = 0;
    virtual CapturedFrame captureFrame() const = 0;
    virtual void release() = 0;
};
}
