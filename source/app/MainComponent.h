#pragma once

#include "OpenGLShaderPreview.h"

#include <juce_gui_extra/juce_gui_extra.h>

namespace myworld
{
class MainComponent final : public juce::Component
{
public:
    explicit MainComponent (bool dumpProofOnStart = false, bool quitAfterProofDump = false);
    ~MainComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void dumpProof();
    void setShaderStatus (juce::String message);

    OpenGLShaderPreview preview;
    juce::TextEditor shaderEditor;
    juce::TextButton dumpProofButton;
    juce::Label graphLabel;
    juce::Label statusLabel;
    GraphContract graph;
    bool shouldQuitAfterProofDump = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
}
