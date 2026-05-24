#pragma once

#include "CompoundPatch.h"
#include "GraphContract.h"
#include "ImGuiSmokeOverlay.h"
#include "NodeSpec.h"
#include "OpenGLRenderBackend.h"

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_opengl/juce_opengl.h>

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace myworld
{
class OpenGLShaderPreview final : public juce::Component,
                                  private juce::OpenGLRenderer,
                                  private juce::Timer
{
public:
    using StatusCallback = std::function<void(juce::String)>;
    using SaveWorkCallback = ImGuiSmokeOverlay::SaveWorkCallback;
    using PublishModuleCallback = ImGuiSmokeOverlay::PublishModuleCallback;

    OpenGLShaderPreview();
    ~OpenGLShaderPreview() override;

    void setFragmentShader (std::string source);
    void setLoudness (float newLoudness);
    void requestProofDump (juce::File outputDirectory, GraphContract graph);

    StatusCallback onStatusMessage;
    SaveWorkCallback onSaveWorkRequested;
    PublishModuleCallback onPublishModuleRequested;

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
    void mouseMove (const juce::MouseEvent& event) override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;
    void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    bool keyPressed (const juce::KeyPress& key) override;

    void compilePendingShader();
    void handlePendingProofDump (int width, int height, double timeSeconds, juce::uint32 currentFrameIndex);
    juce::Image capturedFrameToImage (const CapturedFrame& frame) const;
    void releaseGLObjects();
    void reportStatus (juce::String message);
    void updateImGuiMousePosition (const juce::MouseEvent& event);

    juce::OpenGLContext openGLContext;
    OpenGLRenderBackend renderBackend;
    ImGuiSmokeOverlay imguiOverlay;
    juce::CriticalSection shaderLock;
    juce::CriticalSection proofDumpLock;
    std::vector<NodeSpec> seedNodeSpecs;
    std::vector<RuntimeOpModuleDiagnostic> runtimeOpDiagnostics;
    CompoundPatchSpec loudnessCompound;

    std::string pendingFragmentShader;
    bool compileRequested = true;
    std::unique_ptr<PendingProofDump> pendingProofDump;

    std::atomic<float> loudness { 0.0f };
    juce::uint32 frameIndex = 0;
    double startTimeSeconds = 0.0;
    double lastFrameSeconds = 0.0;
    juce::String lastStatus = "waiting for GL context";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLShaderPreview)
};
}
