#include "RenderBackend.h"
#include "ShaderPreviewInputBridge.h"

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

void expectNear (float actual, float expected, float tolerance, const std::string& message)
{
    expect (std::fabs (actual - expected) <= tolerance,
            message + " expected near " + std::to_string (expected) + " got " + std::to_string (actual));
}

class SmokeRenderBackend final : public myworld::RenderBackend
{
public:
    const char* backendName() const override { return "smoke"; }
    std::string lastStatus() const override { return status; }
    myworld::ShaderCompileResult compileShader (const std::string&) override { return { true, "compiled" }; }
    void resize (int, int, float) override {}
    void renderFrame (const myworld::RenderFrameInput& input) override
    {
        lastLoudness = input.loudness;
        lastFrameIndex = input.frameIndex;
        status = "rendered";
    }
    myworld::CapturedFrame captureFrame() const override { return {}; }
    void release() override { status = "released"; }

    float lastLoudness = 0.0f;
    unsigned int lastFrameIndex = 0;

private:
    std::string status = "waiting";
};
}

int main()
{
    const auto snapshot = myworld::makeShaderPreviewInputFromUniformEvidence (
        "uniform.loudness",
        "u_loudness",
        0.625,
        128);

    const auto input = myworld::makeRenderFrameInputFromShaderPreviewInput (
        snapshot,
        1.25,
        9);

    SmokeRenderBackend backend;
    backend.renderFrame (input);

    expect (backend.lastFrameIndex == 9, "smoke frame index");
    expectNear (backend.lastLoudness, 0.625f, 0.0001f, "smoke loudness");
    expect (backend.lastStatus() == "rendered", "smoke status");

    std::cout << "shader preview smoke read ok\n";
    return 0;
}
