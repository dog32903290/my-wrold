#pragma once

#include "OpenGLShaderPreview.h"

#include <juce_gui_extra/juce_gui_extra.h>

namespace myworld
{
class MainComponent final : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void setShaderStatus (juce::String message);

    OpenGLShaderPreview preview;
    juce::TextEditor shaderEditor;
    juce::Label graphLabel;
    juce::Label statusLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
}
