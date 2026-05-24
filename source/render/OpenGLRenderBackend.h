#pragma once

#include "RenderBackend.h"

#include <juce_opengl/juce_opengl.h>

#include <memory>
#include <string>

namespace myworld
{
class OpenGLRenderBackend final : public RenderBackend
{
public:
    explicit OpenGLRenderBackend (juce::OpenGLContext& context);
    ~OpenGLRenderBackend() override;

    const char* backendName() const override;
    std::string lastStatus() const override;
    ShaderCompileResult compileShader (const std::string& fragmentSource) override;
    void resize (int width, int height, float scale) override;
    void renderFrame (const RenderFrameInput& input) override;
    CapturedFrame captureFrame() const override;
    void release() override;

    void initialise();

private:
    void createFullScreenQuad();
    static juce::String vertexShaderSource();

    juce::OpenGLContext& openGLContext;
    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
    std::unique_ptr<juce::OpenGLShaderProgram::Attribute> positionAttribute;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> timeUniform;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> resolutionUniform;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> frameUniform;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> loudnessUniform;
    juce::uint32 vertexArray = 0;
    juce::uint32 vertexBuffer = 0;
    int viewportWidth = 1;
    int viewportHeight = 1;
    float viewportScale = 1.0f;
    std::string status = "waiting for GL context";
};
}
