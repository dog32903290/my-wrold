#pragma once

#include "GraphContract.h"

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_opengl/juce_opengl.h>

#include <functional>
#include <memory>
#include <string>

namespace myworld
{
class OpenGLShaderPreview final : public juce::Component,
                                  private juce::OpenGLRenderer,
                                  private juce::Timer
{
public:
    using StatusCallback = std::function<void(juce::String)>;

    OpenGLShaderPreview();
    ~OpenGLShaderPreview() override;

    void setFragmentShader (std::string source);

    StatusCallback onStatusMessage;

private:
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;
    void timerCallback() override;

    void compilePendingShader();
    void releaseGLObjects();
    void reportStatus (juce::String message);

    static juce::String vertexShaderSource();

    juce::OpenGLContext openGLContext;
    juce::CriticalSection shaderLock;

    std::string pendingFragmentShader;
    bool compileRequested = true;

    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
    std::unique_ptr<juce::OpenGLShaderProgram::Attribute> positionAttribute;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> timeUniform;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> resolutionUniform;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> frameUniform;

    juce::uint32 vertexArray = 0;
    juce::uint32 vertexBuffer = 0;
    juce::uint32 frameIndex = 0;
    double startTimeSeconds = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLShaderPreview)
};
}
