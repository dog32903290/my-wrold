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
    void requestProofDump (juce::File outputDirectory, GraphContract graph);

    StatusCallback onStatusMessage;

private:
    struct PendingProofDump
    {
        juce::File outputDirectory;
        GraphContract graph;
    };

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;
    void timerCallback() override;

    void compilePendingShader();
    void handlePendingProofDump (int width, int height, double timeSeconds, juce::uint32 currentFrameIndex);
    juce::Image readCurrentFrameBuffer (int width, int height) const;
    void releaseGLObjects();
    void reportStatus (juce::String message);

    static juce::String vertexShaderSource();

    juce::OpenGLContext openGLContext;
    juce::CriticalSection shaderLock;
    juce::CriticalSection proofDumpLock;

    std::string pendingFragmentShader;
    bool compileRequested = true;
    std::unique_ptr<PendingProofDump> pendingProofDump;

    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
    std::unique_ptr<juce::OpenGLShaderProgram::Attribute> positionAttribute;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> timeUniform;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> resolutionUniform;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> frameUniform;

    juce::uint32 vertexArray = 0;
    juce::uint32 vertexBuffer = 0;
    juce::uint32 frameIndex = 0;
    double startTimeSeconds = 0.0;
    juce::String lastStatus = "waiting for GL context";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLShaderPreview)
};
}
