#include "RenderBackend.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}

void expectEqual (const std::string& actual, const std::string& expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + expected + " got " + actual);
}

void expectNear (float actual, float expected, float tolerance, const std::string& message)
{
    expect (std::fabs (actual - expected) <= tolerance,
            message + " expected near " + std::to_string (expected) + " got " + std::to_string (actual));
}

class FakeRenderBackend final : public myworld::RenderBackend
{
public:
    const char* backendName() const override
    {
        return "fake";
    }

    std::string lastStatus() const override
    {
        return status;
    }

    myworld::ShaderCompileResult compileShader (const std::string& fragmentSource) override
    {
        status = fragmentSource.empty() ? "compile failed" : "compiled";
        return { ! fragmentSource.empty(), status };
    }

    void resize (int width, int height, float scale) override
    {
        resizedWidth = width;
        resizedHeight = height;
        resizedScale = scale;
    }

    void renderFrame (const myworld::RenderFrameInput& input) override
    {
        lastFrameIndex = input.frameIndex;
        lastLoudness = input.loudness;
        status = "rendered";
    }

    myworld::CapturedFrame captureFrame() const override
    {
        return { 2, 1, { 0, 1, 2, 3, 4, 5, 6, 7 } };
    }

    void release() override
    {
        status = "released";
    }

    int resizedWidth = 0;
    int resizedHeight = 0;
    float resizedScale = 0.0f;
    unsigned int lastFrameIndex = 0;
    float lastLoudness = 0.0f;

private:
    std::string status = "waiting";
};
}

int main()
{
    FakeRenderBackend backend;

    expectEqual (backend.backendName(), "fake", "backend name");
    expectEqual (backend.lastStatus(), "waiting", "initial status");

    const auto compile = backend.compileShader ("void main(){}");
    expect (compile.ok, "compile succeeds for non-empty source");
    expectEqual (compile.message, "compiled", "compile message");

    backend.resize (640, 360, 2.0f);
    expect (backend.resizedWidth == 640, "resize width");
    expect (backend.resizedHeight == 360, "resize height");
    expectNear (backend.resizedScale, 2.0f, 0.0001f, "resize scale");

    backend.renderFrame ({ 0.5, 7u, 0.25f });
    expect (backend.lastFrameIndex == 7u, "render frame index");
    expectNear (backend.lastLoudness, 0.25f, 0.0001f, "render loudness");

    const auto frame = backend.captureFrame();
    expect (frame.width == 2, "captured width");
    expect (frame.height == 1, "captured height");
    expect (frame.rgba.size() == 8, "captured rgba bytes");

    backend.release();
    expectEqual (backend.lastStatus(), "released", "release status");

    std::cout << "render backend contract ok\n";
    return 0;
}
